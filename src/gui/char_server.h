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

#ifndef _CHAR_SEL_SERVER_H
#define _CHAR_SEL_SERVER_H

#include <guichan/actionlistener.hpp>
#include <guichan/listmodel.hpp>
#include <SDL_events.h>

#include "window.h"

#include "../guichanfwd.h"

/**
 * The list model for the server list.
 */
class ServerListModel : public gcn::ListModel {
    public:
        virtual ~ServerListModel() {};

        int getNumberOfElements();
        std::string getElementAt(int i);
};

/**
 * The server select dialog.
 *
 * \ingroup Interface
 */
class ServerSelectDialog : public Window, public gcn::ActionListener {
    public:
        /**
         * Constructor
         *
         * @see Window::Window
         */
        ServerSelectDialog();

        /**
         * Destructor.
         */
        ~ServerSelectDialog();

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const std::string& eventId);

        /**
         * Updates dialog logic
         */
        void logic();

    private:
        ServerListModel *serverListModel;
        gcn::ListBox *serverList;
        gcn::Button *okButton;
        gcn::Button *cancelButton;
        gcn::ScrollArea *scrollArea;
        int mStatus;

        void selectServer(int index);
};

void charServerInputHandler(SDL_KeyboardEvent *keyEvent);

#endif
