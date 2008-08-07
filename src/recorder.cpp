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
 *  $Id: record.cpp
 */

#include "recorder.h"

#include "gui/chat.h"
#include "gui/buttonbox.h"

#include "utils/trim.h"

Recorder::Recorder(ChatWindow *chat) : mChat(chat)
{
    mButtonBox = new ButtonBox("Recording...", "Stop recording", this);
    mButtonBox->setY(20);
}

void Recorder::record(const std::string &msg)
{
    if (mStream.is_open())
    {
	mStream << msg << std::endl;
    }
}

void Recorder::respond(const std::string &msg)
{
    std::string msgCopy = msg;
    trim(msgCopy);
    if (msgCopy == "")
    {
	if (mStream.is_open())
	{
	    mStream.close();
	    mButtonBox->setVisible(false);
	    /*
	     * Message should go after mStream is closed so that it isn't
	     * recorded.
	     */
	    mChat->chatLog("Finishing recording.", BY_SERVER);
	}
	else
	{
	    mChat->chatLog("Not currently recording.", BY_SERVER);
	}
	return;
    }
    if (mStream.is_open())
    {
	mChat->chatLog("Already recording.", BY_SERVER);
    }
    else
    {
	/*
	 * Message should go before mStream is opened so that it isn't
	 * recorded.
	 */
	mChat->chatLog("Starting to record...", BY_SERVER);
	mStream.open(msg.c_str(), std::ios_base::trunc);
	if (mStream.is_open())
	{
	    mButtonBox->setVisible(true);
	}
	else
	{
	    mChat->chatLog("Failed to start recording.", BY_SERVER);
	}
    }
}

void Recorder::help() const
{
    mChat->chatLog("/record <filename>: Start recording the chat.", BY_SERVER);
}

void Recorder::help(const std::string &args) const
{
    mChat->chatLog("Command: /record <filename>", BY_SERVER);
    mChat->chatLog("This command starts recording the chat log to the file "
		  "<filename>.", BY_SERVER);
    mChat->chatLog("Command: /record", BY_SERVER);
    mChat->chatLog("This command finishes a recording session.", BY_SERVER);
}

void Recorder::buttonBoxRespond()
{
    respond("");
}

Recorder::~Recorder()
{
    delete mButtonBox;
}