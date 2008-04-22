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

#include "map.h"

#include <algorithm>
#include <queue>

#include "beingmanager.h"
#include "game.h"
#include "graphics.h"
#include "particle.h"
#include "sprite.h"
#include "tileset.h"

#include "resources/resourcemanager.h"
#include "resources/ambientoverlay.h"
#include "resources/image.h"

#include "utils/dtor.h"
#include "utils/tostring.h"

/**
 * A location on a tile map. Used for pathfinding, open list.
 */
struct Location
{
    /**
     * Constructor.
     */
    Location(int px, int py, MetaTile *ptile):x(px),y(py),tile(ptile) {};

    /**
     * Comparison operator.
     */
    bool operator< (const Location &loc) const
    {
        return tile->Fcost > loc.tile->Fcost;
    }

    int x, y;
    MetaTile *tile;
};

Map::Map(int width, int height, int tileWidth, int tileHeight):
    mWidth(width), mHeight(height),
    mTileWidth(tileWidth), mTileHeight(tileHeight),
    mMaxTileHeight(height),
    mOnClosedList(1), mOnOpenList(2),
    mLastScrollX(0.0f), mLastScrollY(0.0f)
{
    int size = mWidth * mHeight;

    mMetaTiles = new MetaTile[size];
    mTiles = new Image*[size * 3];
    std::fill_n(mTiles, size * 3, (Image*)0);
}

Map::~Map()
{
    // clean up map data
    delete[] mMetaTiles;
    delete[] mTiles;
    // clean up tilesets
    for_each(mTilesets.begin(), mTilesets.end(), make_dtor(mTilesets));
    mTilesets.clear();
    // clean up overlays
    for_each(mOverlays.begin(), mOverlays.end(), make_dtor(mOverlays));
}

void Map::initializeOverlays()
{
    ResourceManager *resman = ResourceManager::getInstance();

    for (int i = 0;
         hasProperty("overlay" + toString(i) + "image");
         i++)
    {
        const std::string name = "overlay" + toString(i);

        Image *img = resman->getImage(getProperty(name + "image"));
        float speedX = getFloatProperty(name + "scrollX");
        float speedY = getFloatProperty(name + "scrollY");
        float parallax = getFloatProperty(name + "parallax");

        if (img)
        {
            mOverlays.push_back(
                    new AmbientOverlay(img, parallax, speedX, speedY));

            // The AmbientOverlay takes control over the image.
            img->decRef();
        }
    }
}

void Map::addTileset(Tileset *tileset)
{
    mTilesets.push_back(tileset);

    if (tileset->getHeight() > mMaxTileHeight)
        mMaxTileHeight = tileset->getHeight();
}

bool spriteCompare(const Sprite *a, const Sprite *b)
{
    return a->getPixelY() < b->getPixelY();
}

void Map::draw(Graphics *graphics, int scrollX, int scrollY, int layer)
{
    int endPixelY = graphics->getHeight() + scrollY + mTileHeight - 1;

    // If drawing the fringe layer, make sure sprites are sorted
    SpriteIterator si;
    if (layer == 1)
    {
        mSprites.sort(spriteCompare);
        si = mSprites.begin();
        endPixelY += mMaxTileHeight - mTileHeight;
    }

    int startX = scrollX / mTileWidth;
    int startY = scrollY / mTileHeight;
    int endX = (graphics->getWidth() + scrollX + mTileWidth - 1) / mTileWidth;
    int endY = endPixelY / mTileHeight;

    if (startX < 0) startX = 0;
    if (startY < 0) startY = 0;
    if (endX > mWidth) endX = mWidth;
    if (endY > mHeight) endY = mHeight;

    for (int y = startY; y < endY; y++)
    {
        // If drawing the fringe layer, make sure all sprites above this row of
        // tiles have been drawn
        if (layer == 1)
        {
            while (si != mSprites.end() && (*si)->getPixelY() <= y * 32 - 32)
            {
                (*si)->draw(graphics, -scrollX, -scrollY);
                si++;
            }
        }

        for (int x = startX; x < endX; x++)
        {
            Image *img = getTile(x, y, layer);
            if (img) {
                graphics->drawImage(img,
                        x * mTileWidth - scrollX,
                        y * mTileHeight - scrollY +
                            mTileHeight - img->getHeight());
            }
        }
    }

    // Draw any remaining sprites
    if (layer == 1)
    {
        while (si != mSprites.end())
        {
            (*si)->draw(graphics, -scrollX, -scrollY);
            si++;
        }
    }
}

void Map::drawOverlay(Graphics *graphics,
                      float scrollX, float scrollY, int detail)
{
    static int lastTick = tick_time;

    // Detail 0: no overlays
    if (detail <= 0) return;

    if (mLastScrollX == 0.0f && mLastScrollY == 0.0f)
    {
        // First call - initialisation
        mLastScrollX = scrollX;
        mLastScrollY = scrollY;
    }

    // Update Overlays
    int timePassed = get_elapsed_time(lastTick);
    float dx = scrollX - mLastScrollX;
    float dy = scrollY - mLastScrollY;

    std::list<AmbientOverlay*>::iterator i;
    for (i = mOverlays.begin(); i != mOverlays.end(); i++)
    {
        (*i)->update(timePassed, dx, dy);
    }
    mLastScrollX = scrollX;
    mLastScrollY = scrollY;
    lastTick = tick_time;

    // Draw overlays
    for (i = mOverlays.begin(); i != mOverlays.end(); i++)
    {
        (*i)->draw(graphics, graphics->getWidth(), graphics->getHeight());

        // Detail 1: only one overlay, higher: all overlays
        if (detail == 1)
            break;
    };
}

void Map::setTileWithGid(int x, int y, int layer, int gid)
{
    if (layer == 3)
    {
        Tileset *set = getTilesetWithGid(gid);
        setWalk(x, y, (!set || (gid - set->getFirstGid() == 0)));
    }
    else if (layer < 3)
    {
        setTile(x, y, layer, getTileWithGid(gid));
    }
}

class ContainsGidFunctor
{
    public:
        bool operator() (Tileset* set)
        {
            return (set->getFirstGid() <= gid &&
                    gid - set->getFirstGid() < (int)set->size());
        }
        int gid;
} containsGid;

Tileset* Map::getTilesetWithGid(int gid) const
{
    containsGid.gid = gid;

    Tilesets::const_iterator i = find_if(mTilesets.begin(), mTilesets.end(),
            containsGid);

    return (i == mTilesets.end()) ? NULL : *i;
}

Image* Map::getTileWithGid(int gid) const
{
    Tileset *set = getTilesetWithGid(gid);

    if (set) {
        return set->get(gid - set->getFirstGid());
    }

    return NULL;
}

void Map::setWalk(int x, int y, bool walkable)
{
    mMetaTiles[x + y * mWidth].walkable = walkable;
}

bool Map::occupied(int x, int y) const
{
    Beings &beings = beingManager->getAll();
    for (BeingIterator i = beings.begin(); i != beings.end(); i++)
    {
        // job 45 is a portal, they don't collide
        if ((*i)->mX == x && (*i)->mY == y && (*i)->mJob != 45)
        {
            return true;
        }
    }

    return false;
}

bool Map::tileCollides(int x, int y) const
{
    return !(contains(x, y) && mMetaTiles[x + y * mWidth].walkable);
}

bool Map::contains(int x, int y) const
{
    return x >= 0 && y >= 0 && x < mWidth && y < mHeight;
}

void Map::setTile(int x, int y, int layer, Image *img)
{
    mTiles[x + y * mWidth + layer * (mWidth * mHeight)] = img;
}

Image* Map::getTile(int x, int y, int layer)
{
    return mTiles[x + y * mWidth + layer * (mWidth * mHeight)];
}

MetaTile* Map::getMetaTile(int x, int y)
{
    return &mMetaTiles[x + y * mWidth];
}

SpriteIterator Map::addSprite(Sprite *sprite)
{
    mSprites.push_front(sprite);
    return mSprites.begin();
}

void Map::removeSprite(SpriteIterator iterator)
{
    mSprites.erase(iterator);
}

Path Map::findPath(int startX, int startY, int destX, int destY)
{
    // Path to be built up (empty by default)
    Path path;

    // Declare open list, a list with open tiles sorted on F cost
    std::priority_queue<Location> openList;

    // Reset starting tile's G cost to 0
    MetaTile *startTile = getMetaTile(startX, startY);
    startTile->Gcost = 0;

    // Add the start point to the open list
    openList.push(Location(startX, startY, startTile));

    bool foundPath = false;

    // Keep trying new open tiles until no more tiles to try or target found
    while (!openList.empty() && !foundPath)
    {
        // Take the location with the lowest F cost from the open list.
        Location curr = openList.top();
        openList.pop();

        // If the tile is already on the closed list, this means it has already
        // been processed with a shorter path to the start point (lower G cost)
        if (curr.tile->whichList == mOnClosedList)
        {
            continue;
        }

        // Put the current tile on the closed list
        curr.tile->whichList = mOnClosedList;

        // Check the adjacent tiles
        for (int dy = -1; dy <= 1; dy++)
        {
            for (int dx = -1; dx <= 1; dx++)
            {
                // Calculate location of tile to check
                int x = curr.x + dx;
                int y = curr.y + dy;

                // Skip if if we're checking the same tile we're leaving from,
                // or if the new location falls outside of the map boundaries
                if ((dx == 0 && dy == 0) || !contains(x, y))
                {
                    continue;
                }

                MetaTile *newTile = getMetaTile(x, y);

                // Skip if the tile is on the closed list or collides unless its the destination tile
                if (newTile->whichList == mOnClosedList ||
                        (tileCollides(x, y) && !(x == destX && y == destY)))
                {
                    continue;
                }

                // When taking a diagonal step, verify that we can skip the
                // corner. We allow skipping past beings but not past non-
                // walkable tiles.
                if (dx != 0 && dy != 0)
                {
                    MetaTile *t1 = getMetaTile(curr.x, curr.y + dy);
                    MetaTile *t2 = getMetaTile(curr.x + dx, curr.y);

                    if (!(t1->walkable && t2->walkable))
                    {
                        continue;
                    }
                }

                // Calculate G cost for this route, 10 for moving straight and
                // 14 for moving diagonal (sqrt(200) = 14.1421...)
                int Gcost = curr.tile->Gcost + ((dx == 0 || dy == 0) ? 10 : 14);

                // It costs extra to walk through a being (needs to be enough
                // to make it more attractive to walk around).
                if (occupied(x, y))
                {
                    Gcost += 30;
                }

                // Skip if Gcost becomes too much
                // Warning: probably not entirely accurate
                if (Gcost > 200)
                {
                    continue;
                }

                if (newTile->whichList != mOnOpenList)
                {
                    // Found a new tile (not on open nor on closed list)
                    // Update Hcost of the new tile using Manhatten distance
                    newTile->Hcost = 10 * (abs(x - destX) + abs(y - destY));

                    // Set the current tile as the parent of the new tile
                    newTile->parentX = curr.x;
                    newTile->parentY = curr.y;

                    // Update Gcost and Fcost of new tile
                    newTile->Gcost = Gcost;
                    newTile->Fcost = newTile->Gcost + newTile->Hcost;

                    if (x != destX || y != destY) {
                        // Add this tile to the open list
                        newTile->whichList = mOnOpenList;
                        openList.push(Location(x, y, newTile));
                    }
                    else {
                        // Target location was found
                        foundPath = true;
                    }
                }
                else if (Gcost < newTile->Gcost)
                {
                    // Found a shorter route.
                    // Update Gcost and Fcost of the new tile
                    newTile->Gcost = Gcost;
                    newTile->Fcost = newTile->Gcost + newTile->Hcost;

                    // Set the current tile as the parent of the new tile
                    newTile->parentX = curr.x;
                    newTile->parentY = curr.y;

                    // Add this tile to the open list (it's already
                    // there, but this instance has a lower F score)
                    openList.push(Location(x, y, newTile));
                }
            }
        }
    }

    // Two new values to indicate whether a tile is on the open or closed list,
    // this way we don't have to clear all the values between each pathfinding.
    mOnClosedList += 2;
    mOnOpenList += 2;

    // If a path has been found, iterate backwards using the parent locations
    // to extract it.
    if (foundPath)
    {
        int pathX = destX;
        int pathY = destY;

        while (pathX != startX || pathY != startY)
        {
            // Add the new path node to the start of the path list
            path.push_front(PATH_NODE(pathX, pathY));

            // Find out the next parent
            MetaTile *tile = getMetaTile(pathX, pathY);
            pathX = tile->parentX;
            pathY = tile->parentY;
        }
    }

    return path;
}

void Map::addParticleEffect (std::string effectFile, int x, int y)
{
    ParticleEffectData newEffect;
    newEffect.file = effectFile;
    newEffect.x = x;
    newEffect.y = y;
    particleEffects.push_back(newEffect);
}

void Map::initializeParticleEffects(Particle* particleEngine)
{
    for (std::list<ParticleEffectData>::iterator i = particleEffects.begin();
         i != particleEffects.end();
         i++
        )
    {
        particleEngine->addEffect(i->file, i->x, i->y);
    }
}
