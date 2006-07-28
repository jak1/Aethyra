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

#include "monster.h"

#include "animatedsprite.h"
#include "game.h"

#include "utils/tostring.h"


Monster::Monster(Uint32 id, Uint16 job, Map *map):
    Being(id, job, map)
{
    mSprites[BASE_SPRITE] = new AnimatedSprite("graphics/sprites/monster" + toString(job - 1002) + ".xml", 0);
}

void
Monster::logic()
{
    if (mAction != STAND)
    {
        mFrame = (get_elapsed_time(mWalkTime) * 4) / mWalkSpeed;

        if (mFrame >= 4 && mAction != MONSTER_DEAD)
        {
            nextStep();
        }
    }

    Being::logic();
}

Being::Type
Monster::getType() const
{
    return MONSTER;
}

