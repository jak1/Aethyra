/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#include "being.h"

#include <algorithm>
#include <sstream>

#include "game.h"
#include "graphics.h"
#include "log.h"
#include "map.h"

#include "graphic/spriteset.h"

#include "gui/gui.h"

#include "net/messageout.h"
#include "net/protocol.h"

#include "resources/resourcemanager.h"
#include "resources/image.h"

extern Being* autoTarget;
extern std::map<int, Spriteset*> monsterset;

// From main.cpp
extern Spriteset *hairset;
extern Spriteset *playerset;

// From engine.cpp
extern Spriteset *emotionset;
extern Spriteset *npcset;
extern Spriteset *weaponset;

Being *player_node = NULL;

Beings beings;

signed char hairtable[16][4][2] = {
    // S(x,y)    W(x,y)   N(x,y)   E(x,y)
    { { 0,  0}, {-1, 2}, {-1, 2}, { 0, 2} }, // STAND
    { { 0,  2}, {-2, 3}, {-1, 2}, { 1, 3} }, // WALK 1st frame
    { { 0,  3}, {-2, 4}, {-1, 3}, { 1, 4} }, // WALK 2nd frame
    { { 0,  1}, {-2, 2}, {-1, 2}, { 1, 2} }, // WALK 3rd frame
    { { 0,  2}, {-2, 3}, {-1, 2}, { 1, 3} }, // WALK 4th frame
    { { 0,  1}, { 1, 2}, {-1, 3}, {-2, 2} }, // ATTACK 1st frame
    { { 0,  1}, {-1, 2}, {-1, 3}, { 0, 2} }, // ATTACK 2nd frame
    { { 0,  2}, {-4, 3}, { 0, 4}, { 3, 3} }, // ATTACK 3rd frame
    { { 0,  2}, {-4, 3}, { 0, 4}, { 3, 3} }, // ATTACK 4th frame
    { { 0,  0}, {-1, 2}, {-1, 2}, {-1, 2} }, // BOW_ATTACK 1st frame
    { { 0,  0}, {-1, 2}, {-1, 2}, {-1, 2} }, // BOW_ATTACK 2nd frame
    { { 0,  0}, {-1, 2}, {-1, 2}, {-1, 2} }, // BOW_ATTACK 3rd frame
    { { 0,  0}, {-1, 2}, {-1, 2}, {-1, 2} }, // BOW_ATTACK 4th frame
    { { 0,  4}, {-1, 6}, {-1, 6}, { 0, 6} }, // SIT
    { { 0,  0}, { 0, 0}, { 0, 0}, { 0, 0} }, // ?? HIT
    { { 0, 16}, {-1, 6}, {-1, 6}, { 0, 6} }  // DEAD
};

PATH_NODE::PATH_NODE(Uint16 iX, Uint16 iY):
    x(iX), y(iY)
{
}

Being* createBeing(Uint32 id, Uint16 job, Map *map)
{
    Being *being = new Being;

    being->setId(id);
    being->job = job;
    being->setMap(map);

    beings.push_back(being);

    // If the being is a player, request the name
    if (being->getType() == Being::PLAYER)
    {
        MessageOut outMsg;
        outMsg.writeShort(0x0094);
        outMsg.writeLong(being->getId());//readLong(2));
    }
    // If the being is a monster then load the monsterset
    else if (being->job >= 1002 &&
            monsterset.find(being->job - 1002) == monsterset.end())
    {
        std::stringstream filename;

        filename << "graphics/sprites/monster" << (being->job - 1002) << ".png";
        logger->log("%s",filename.str().c_str());

        Image *monsterbitmap =
            ResourceManager::getInstance()->getImage(filename.str());

        if (!monsterbitmap) {
            logger->error("Unable to load monster.png");
        } else {
            monsterset[being->job - 1002] = new Spriteset(monsterbitmap, 60, 60);
            monsterbitmap->decRef();
        }
    }

    return being;
}

void remove_node(Being *being)
{
    delete being;
    beings.remove(being);
}

Being *findNode(Uint32 id)
{
    for (Beings::iterator i = beings.begin(); i != beings.end(); i++)
    {
        Being *being = (*i);
        if (being->getId() == id) {
            return being;
        }
    }
    return NULL;
}

class FindNodeFunctor
{
    public:
        bool operator() (Being *being)
        {
            Uint16 other_y = y + ((being->getType() == Being::NPC) ? 1 : 0);
            return (being->x == x && (being->y == y || being->y == other_y) &&
                    being->action != Being::MONSTER_DEAD &&
                    (type == Being::UNKNOWN || being->getType() == type));
        }

        Uint16 x, y;
        Being::Type type;
} nodeFinder;

Being *findNode(Uint16 x, Uint16 y)
{
    return findNode(x, y, Being::UNKNOWN);
}

Being* findNode(Uint16 x, Uint16 y, Being::Type type)
{
    nodeFinder.x = x;
    nodeFinder.y = y;
    nodeFinder.type = type;

    Beings::iterator i = find_if(beings.begin(), beings.end(), nodeFinder);

    return (i == beings.end()) ? NULL : *i;
}

Being::Being():
    job(0),
    x(0), y(0), direction(SOUTH),
    action(0), mFrame(0),
    speech_color(0),
    walk_time(0),
    emotion(0), emotion_time(0),
    aspd(350),
    mId(0),
    mWeapon(0),
    mWalkSpeed(150),
    mMap(NULL),
    hairStyle(1), hairColor(1),
    speech_time(0),
    damage_time(0),
    showSpeech(false), showDamage(false)
{
}

Being::~Being()
{
    clearPath();
    setMap(NULL);
}

void Being::setDestination(Uint16 destX, Uint16 destY)
{
    if (mMap)
    {
        setPath(mMap->findPath(x, y, destX, destY));
    }
}

void Being::clearPath()
{
    mPath.clear();
}

void Being::setPath(std::list<PATH_NODE> path)
{
    mPath = path;

    if (action != WALK && action != DEAD)
    {
        nextStep();
        walk_time = tick_time;
    }
}

void Being::setHairColor(Uint16 color)
{
    hairColor = color;
    if (hairColor < 1 || hairColor > NR_HAIR_COLORS + 1)
    {
        hairColor = 1;
    }
}

void Being::setHairStyle(Uint16 style)
{
    hairStyle = style;
    if (hairStyle < 1 || hairStyle > NR_HAIR_STYLES)
    {
        hairStyle = 1;
    }
}

void
Being::setSpeech(const std::string &text, Uint32 time)
{
    speech = text;
    speech_time = tick_time;
    showSpeech = true;
}

void
Being::setDamage(Sint16 amount, Uint32 time)
{
    if (!amount) {
        damage = "miss";
    } else {
        std::stringstream damageString;
        damageString << amount;
        damage = damageString.str();
    }
    damage_time = tick_time;
    showDamage = true;
}

void
Being::setMap(Map *map)
{
    // Remove sprite from potential previous map
    if (mMap != NULL)
    {
        mMap->removeSprite(mSpriteIterator);
    }

    mMap = map;

    // Add sprite to potential new map
    if (mMap != NULL)
    {
        mSpriteIterator = mMap->addSprite(this);
    }
}

void
Being::nextStep()
{
    mFrame = 0;

    if (mPath.empty())
    {
        action = STAND;
        return;
    }

    PATH_NODE node = mPath.front();
    mPath.pop_front();

    if (node.x > x) {
        if (node.y > y)       direction = SE;
        else if (node.y < y)  direction = NE;
        else                  direction = EAST;
    }
    else if (node.x < x) {
        if (node.y > y)       direction = SW;
        else if (node.y < y)  direction = NW;
        else                  direction = WEST;
    }
    else {
        if (node.y > y)       direction = SOUTH;
        else if (node.y < y)  direction = NORTH;
    }

    x = node.x;
    y = node.y;
    action = WALK;
    walk_time += mWalkSpeed / 10;
}

void
Being::logic()
{
    // Determine whether speech should still be displayed
    if (get_elapsed_time(speech_time) > 5000)
    {
        showSpeech = false;
    }

    // Determine whether damange should still be displayed
    if (get_elapsed_time(damage_time) > 3000)
    {
        showDamage = false;
    }

    // Execute next walk or attack command for players
    if (getType() == PLAYER)
    {
        switch (action) {
            case WALK:
                mFrame = (get_elapsed_time(walk_time) * 4) / mWalkSpeed;
                if (mFrame >= 4) {
                    nextStep();
                }
                break;
            case ATTACK:
                mFrame = (get_elapsed_time(walk_time) * 4) / aspd;
                if (mFrame >= 4) {
                    nextStep();
                    if (autoTarget && this == player_node) {
                        attack(autoTarget);
                    }
                }
                break;
        }

        if (emotion != 0)
        {
            emotion_time--;
            if (emotion_time == 0) {
                emotion = 0;
            }
        }
    }

    if (getType() == MONSTER && action != STAND)
    {
        mFrame = (get_elapsed_time(walk_time) * 4) / mWalkSpeed;

        if (mFrame >= 4 && action != MONSTER_DEAD)
        {
            nextStep();
        }
    }

    // Update pixel coordinates
    mPx = x * 32;
    mPy = y * 32;

    if (getType() == PLAYER || getType() == MONSTER)
    {
        mPy += getYOffset();
        mPx += getXOffset();
    }
}

void
Being::drawSpeech(Graphics *graphics, Sint32 offsetX, Sint32 offsetY)
{
    int px = mPx + offsetX;
    int py = mPy + offsetY;

    // Draw speech above this being
    if (showSpeech)
    {
        graphics->setFont(speechFont);
        graphics->drawText(speech, px + 18, py - 60, gcn::Graphics::CENTER);
    }

    // Draw damage above this being
    if (showDamage)
    {
        // Selecting the right color
        if (damage == "miss")
        {
            graphics->setFont(hitYellowFont);
        }
        else if (getType() == MONSTER)
        {
            graphics->setFont(hitBlueFont);
        }
        else
        {
            graphics->setFont(hitRedFont);
        }

        int textY = (getType() == PLAYER) ? 70 : 32;
        int ft = get_elapsed_time(damage_time) - 1500;
        float a = (ft > 0) ? 1.0 - ft / 1500.0 : 1.0;

        graphics->setColor(gcn::Color(255, 255, 255, (int)(255 * a)));
        graphics->drawText(damage,
                           px + 16,
                           py - textY - get_elapsed_time(damage_time) / 100,
                           gcn::Graphics::CENTER);
    }

    // Potentially draw [TARGET] above this being
    if (this == autoTarget)
    {
        graphics->setFont(speechFont);
        int dy = (getType() == PLAYER) ? 90 : 52;

        graphics->drawText("[TARGET]", px + 15, py - dy,
                           gcn::Graphics::CENTER);
    }
}

Being::Type Being::getType() const
{
    if (job < 10) {
        return PLAYER;
    } else if (job >= 100 & job < 200) {
        return NPC;
    } else if (job >= 1000 && job < 1200) {
        return MONSTER;
    } else {
        return UNKNOWN;
    }
}

void Being::setWeaponById(Uint16 weapon)
{
    switch (weapon)
    {
    case 529: // iron arrows
    case 1199: // arrows
        break;

    case 1200: // bow
    case 530: // short bow
    case 545: // forest bow
        setWeapon(2);
        break;

    case 521: // sharp knife
    case 522: // dagger
    case 536: // short sword
    case 1201: // knife
        setWeapon(1);
        break;

    case 0: // unequip
        setWeapon(0);
        break;

    default:
        logger->log("unknown item equiped : %d", weapon);
    }
}

int
Being::getXOffset() const
{
    // Only beings walking to the left or the right have an x offset
    if (action != WALK || direction == NORTH || direction == SOUTH) {
        return 0;
    }

    int offset = (get_elapsed_time(walk_time) * 32) / mWalkSpeed;

    // We calculate the offset _from_ the _target_ location
    offset -= 32;
    if (offset > 0) {
        offset = 0;
    }

    // Going to the right? Invert the offset.
    if (direction == WEST || direction == NW || direction == SW) {
        offset = -offset;
    }

    return offset;
}

int
Being::getYOffset() const
{
    // Only beings walking up or down have an y offset
    if (action != WALK || direction == EAST || direction == WEST) {
        return 0;
    }

    int offset = (get_elapsed_time(walk_time) * 32) / mWalkSpeed;

    // We calculate the offset _from_ the _target_ location
    offset -= 32;
    if (offset > 0) {
        offset = 0;
    }

    if (direction == NORTH || direction == NW || direction == NE) {
        offset = -offset;
    }

    return offset;
}

void
Being::draw(Graphics *graphics, int offsetX, int offsetY)
{
    unsigned char dir = direction / 2;
    int px = mPx + offsetX;
    int py = mPy + offsetY;
    int frame;

    switch (getType())
    {
        case PLAYER:
            frame = action;

            if (action != SIT && action != DEAD)
            {
                frame += mFrame;
            }

            if (action == ATTACK && getWeapon() > 0)
            {
                frame += 4 * (getWeapon() - 1);
            }

            graphics->drawImage(playerset->spriteset[frame + 16 * dir],
                                px - 16, py - 32);

            if (getWeapon() != 0 && action == ATTACK)
            {
                Image *image = weaponset->spriteset[
                    16 * (getWeapon() - 1) + 4 * mFrame + dir];

                graphics->drawImage(image, px - 64, py - 80);
            }

            if (getHairColor() <= NR_HAIR_COLORS)
            {
                int hf = getHairColor() - 1 + 10 * (dir + 4 *
                         (getHairStyle() - 1));

                graphics->drawImage(hairset->spriteset[hf],
                                    px - 2 + 2 * hairtable[frame][dir][0],
                                    py - 50 + 2 * hairtable[frame][dir][1]);
            }

            if (emotion != 0)
            {
                graphics->drawImage(emotionset->spriteset[emotion - 1],
                                    px + 3, py - 90);
            }

            // Draw player name
            if (this != player_node) {
                graphics->setFont(speechFont);
                graphics->drawText(mName, px + 15, py + 30, gcn::Graphics::CENTER);
            }
            break;

        case NPC:
            graphics->drawImage(npcset->spriteset[job - 100], px - 8, py - 52);
            break;

        case MONSTER:
            if (mFrame >= 4)
            {
                mFrame = 3;
            }

            frame = action;
            if (action != MONSTER_DEAD) {
                frame += mFrame;
            }
            graphics->drawImage(
                    monsterset[job-1002]->spriteset[dir + 4 * frame],
                    px - 12, py - 25);
            break;

        default:
            break;
    }
}
