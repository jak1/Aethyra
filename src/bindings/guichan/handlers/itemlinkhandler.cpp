/*
 *  Aethyra
 *  Copyright 2009 The Mana World Development Team
 *
 *  This file is part of Aethyra based on original code
 *  from The Mana World.
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
 */

#include <sstream>
#include <string>

#include "itemlinkhandler.h"

#include "../../../gui/itempopup.h"
#include "../../../gui/viewport.h"

#include "../../../resources/db/iteminfo.h"
#include "../../../resources/db/itemdb.h"

ItemLinkHandler::ItemLinkHandler()
{
    mItemPopup = new ItemPopup;
    mItemPopup->setOpaque(false);
}

ItemLinkHandler::~ItemLinkHandler()
{
    delete mItemPopup;
}

void ItemLinkHandler::handleLink(const std::string &link)
{
    int id = 0;
    std::stringstream stream;
    stream << link;
    stream >> id;
    if (id > 0)
    {
        const ItemInfo &iteminfo = ItemDB::get(id);

        if (iteminfo.getName() != mItemPopup->getItemName())
            mItemPopup->setItem(iteminfo);

        if (mItemPopup->isVisible())
        {
            mItemPopup->setVisible(false);
        }
        else
        {
            mItemPopup->updateColors();
            mItemPopup->view(viewport->getMouseX(), viewport->getMouseY());
        }
    }
}