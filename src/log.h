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
 */

#ifndef _LOG_H
#define _LOG_H

#include <iosfwd>
#include <fstream>

/**
 * The Log Class : Useful to write debug or info messages
 */
class Logger
{
    public:
        /**
         * Destructor, closes log file.
         */
        ~Logger();

        /**
         * Sets the file to log to and opens it
         */
        void setLogFile(const std::string &logFilename);

        /**
         * Enters a message in the log. The message will be timestamped.
         */
        void log(const char *log_text, ...);

        /**
         * Log an error and quit. The error will pop-up in Windows and will be
         * printed to standard error everywhere else. 
         */
        void error(const std::string &error_text);

    private:
        std::ofstream logFile;
};

extern Logger *logger;

#endif
