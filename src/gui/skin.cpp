/*
 *  Aethyra
 *  Copyright (C) 2009  Aethyra Development Team
 *
 *  This file is part of Aethyra.
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

#include "skin.h"

#include "../log.h"

#include "../resources/image.h"
#include "../resources/resourcemanager.h"

#include "../utils/dtor.h"
#include "../utils/xml.h"

SkinLoader* skinLoader = NULL;

Skin::Skin():
    closeImage(NULL),
    instances(0)
{
}

Skin::~Skin()
{
    // Clean up static resources
    for (int i = 0; i < 9; i++)
    {
        delete border.grid[i];
        border.grid[i] = NULL;
    }

    closeImage->decRef();
}

Skin* SkinLoader::load(const std::string &filename)
{
    SkinIterator skinIterator = mSkins.find(filename);

    if (mSkins.end() != skinIterator)
    {
        skinIterator->second->instances++;
        return skinIterator->second;
    }

    Skin* skin = new Skin();

    ResourceManager *resman = ResourceManager::getInstance();

    logger->log("Loading Skin '%s'.", filename.c_str());

    if (filename.empty())
        logger->error("SkinLoader::load(): Invalid File Name.");

    // TODO:
    // If there is an error loading the specified file, we should try to revert
    // to a 'default' skin file. Only if the 'default' skin file can't be loaded
    // should we have a terminating error.
    XML::Document doc(filename);
    xmlNodePtr rootNode = doc.rootNode();

    if (!rootNode || !xmlStrEqual(rootNode->name, BAD_CAST "skinset"))
        logger->error("Widget Skinning error");

    std::string skinSetImage;
    skinSetImage = XML::getProperty(rootNode, "image", "");
    Image *dBorders = NULL;
    if (!skinSetImage.empty())
    {
        logger->log("SkinLoader::load(): <skinset> defines "
                    "'%s' as a skin image.", skinSetImage.c_str());
        dBorders = resman->getImage("graphics/gui/" + skinSetImage);
    }
    else
    {
        logger->error("SkinLoader::load(): Skinset does not define an image!");
    }

    //iterate <widget>'s
    for_each_xml_child_node(widgetNode, rootNode)
    {
        if (!xmlStrEqual(widgetNode->name, BAD_CAST "widget"))
            continue;

        std::string widgetType;
        widgetType = XML::getProperty(widgetNode, "type", "unknown");
        if (widgetType == "Window")
        {
            // Iterate through <part>'s
            // LEEOR / TODO:
            // We need to make provisions to load in a CloseButton image. For 
            // now it can just be hard-coded.
            for_each_xml_child_node(partNode, widgetNode)
            {
                if (!xmlStrEqual(partNode->name, BAD_CAST "part"))
                    continue;

                std::string partType;
                partType = XML::getProperty(partNode, "type", "unknown");
                // TOP ROW
                const int xPos = XML::getProperty(partNode, "xpos", 0);
                const int yPos = XML::getProperty(partNode, "ypos", 0);
                const int width = XML::getProperty(partNode, "width", 1);
                const int height = XML::getProperty(partNode, "height", 1);

                if (partType == "top-left-corner")
                    skin->border.grid[0] = dBorders->getSubImage(xPos, yPos, width, height);
                else if (partType == "top-edge")
                    skin->border.grid[1] = dBorders->getSubImage(xPos, yPos, width, height);
                else if (partType == "top-right-corner")
                    skin->border.grid[2] = dBorders->getSubImage(xPos, yPos, width, height);

                // MIDDLE ROW
                else if (partType == "left-edge")
                    skin->border.grid[3] = dBorders->getSubImage(xPos, yPos, width, height);
                else if (partType == "bg-quad")
                    skin->border.grid[4] = dBorders->getSubImage(xPos, yPos, width, height);
                else if (partType == "right-edge")
                    skin->border.grid[5] = dBorders->getSubImage(xPos, yPos, width, height);

                // BOTTOM ROW
                else if (partType == "bottom-left-corner")
                    skin->border.grid[6] = dBorders->getSubImage(xPos, yPos, width, height);
                else if (partType == "bottom-edge")
                    skin->border.grid[7] = dBorders->getSubImage(xPos, yPos, width, height);
                else if (partType == "bottom-right-corner")
                    skin->border.grid[8] = dBorders->getSubImage(xPos, yPos, width, height);

                // Part is of an uknown type.
                else
                    logger->log("SkinLoader::load(): Unknown Part Type '%s'", partType.c_str());
            }
        }
        // Widget is of an uknown type.
        else
        {
            logger->log("SkinLoader::load(): Unknown Widget Type '%s'", widgetType.c_str());
        }
    }
    dBorders->decRef();

    logger->log("Finished loading Skin.");

    // Hard-coded for now until we update the above code to look for window buttons.
    skin->closeImage = resman->getImage("graphics/gui/close_button.png");
    mSkins[filename] = skin;
    return skin;
}

SkinLoader::SkinLoader()
{
}

SkinLoader::~SkinLoader()
{
    delete_all(mSkins);
}

