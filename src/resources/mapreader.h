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

#ifndef _INCLUDED_MAPREADER_H
#define _INCLUDED_MAPREADER_H

#include "../map.h"
#include "../graphic/spriteset.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <vector>

/**
 * A tileset, which is basically just a spriteset but it stores a firstgid.
 */
class Tileset : public Spriteset {
    public:
        /**
         * Constructor.
         */
        Tileset(Image *img, int w, int h, int firstGid);

        /**
         * Returns the first gid.
         */
        int getFirstGid();

    private:
        int firstGid;
};

/**
 * Reader for XML map files (*.tmx)
 */
class MapReader
{
    public:
        /**
         * Read an XML map from a file.
         */
        static Map *readMap(const std::string &filename);

        /**
         * Read an XML map from a parsed XML tree. The path is used to find the
         * location of referenced tileset images.
         */
        static Map *readMap(xmlNodePtr node, const std::string &path);

    private:
        /**
         * Reads a map layer.
         */
        static void readLayer(xmlNodePtr node, Map *map, int layer);

        /**
         * Reads a tile set.
         */
        static Tileset *readTileset(xmlNodePtr node, const std::string &path,
                Map *map);

        /**
         * Gets an integer property from an xmlNodePtr.
         */
        static int getProperty(xmlNodePtr node, const char* name, int def);

        /**
         * Converts a global tile id to the Image* pointing to the associated
         * tile image.
         */
        static Image *getTileWithGid(int gid);

        /**
         * Finds the tile set that a tile with the given global id is part of.
         */
        static Tileset *MapReader::getTilesetWithGid(int gid);

        /**
         * Sets a tile using a global tile id. Used by the layer loading
         * routine.
         */
        static void MapReader::setTileWithGid(
                Map *map, int x, int y, int layer, int gid);

        static std::vector<Tileset*> tilesets;
};

#endif
