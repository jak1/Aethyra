/*
 *  Aethyra
 *  Copyright (C) 2007  The Mana World Development Team
 *
 *  This file is part of Aethyra based on original code
 *  from The Mana World.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef KEY_LIST_MODEL_H
#define KEY_LIST_MODEL_H

#include <string>

#include <guichan/listmodel.hpp>

#include "../../sdl/keyboardconfig.h"

/**
 * The list model for key function list.
 *
 * \ingroup Interface
 */
class KeyListModel : public gcn::ListModel
{
    public:
        /**
         * Returns the number of elements in container.
         */
        int getNumberOfElements() { return keyboard.KEY_TOTAL; }

        /**
         * Returns element from container.
         */
        std::string getElementAt(int i) { return mKeyFunctions[i]; }

        /**
         * Sets element from container.
         */
        void setElementAt(int i, std::string caption)
        { mKeyFunctions[i] = caption; }

    private:
        std::string mKeyFunctions[KeyboardConfig::KEY_TOTAL];
};
#endif
