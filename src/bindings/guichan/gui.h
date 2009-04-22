/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
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

#ifndef GUI
#define GUI

#include <guichan/gui.hpp>

#include <string>

#include "guichanfwd.h"

class Game;
class Graphics;
class GuiConfigListener;
class ImageSet;
class SDLInput;
class Viewport;

extern volatile int fps;
extern volatile int tick_time;
/**
 * \defgroup GUI Core GUI related classes (widgets)
 */

/**
 * \defgroup Interface User interface related classes (windows, dialogs)
 */

/**
 * Main GUI class.
 *
 * \ingroup GUI
 */
class Gui : public gcn::Gui
{
    public:
        /**
         * Constructor.
         */
        Gui(Graphics *screen);

        /**
         * Destructor.
         */
        ~Gui();

        /**
         * Performs logic of the GUI. Overridden to track mouse pointer
         * activity.
         */
        void logic();

        /**
         * Draws the whole Gui by calling draw functions down in the
         * Gui hierarchy. It also draws the mouse pointer.
         */
        void draw();

        /**
         * Whether there's focused input or not.
         */
        bool isInputFocused();

        /**
         * Gets the currently focused widget.
         */
        gcn::Widget* getFocused();

        /**
         * Return game font.
         */
        gcn::Font* getFont() const { return mGuiFont; }

        /**
         * Return game font height.
         */
        const int getFontHeight() const;

        /**
         * Return the Font used for "Info Particles", i.e. ones showing, what
         * you picked up, etc.
         */
        gcn::Font* getInfoParticleFont() const { return mInfoParticleFont; }

        /**
         * Sets whether a custom cursor should be rendered.
         */
        void setUseCustomCursor(bool customCursor);

        /**
         * Sets which cursor should be used.
         */
        void setCursorType(int index) { mCursorType = index; }

        /**
         * Frame rate has been changed. Adjust it accordingly.
         */
        void framerateChanged();

        /**
         * Cursors are in graphic order from left to right.
         * CURSOR_POINTER should be left untouched.
         * CURSOR_TOTAL should always be last.
         */
        enum {
            CURSOR_POINTER = 0,
            CURSOR_RESIZE_ACROSS,
            CURSOR_RESIZE_DOWN,
            CURSOR_RESIZE_DOWN_LEFT,
            CURSOR_RESIZE_DOWN_RIGHT,
            CURSOR_TOTAL
        };

    protected:
        void handleMouseMoved(const gcn::MouseInput &mouseInput);

    private:
        GuiConfigListener *mConfigListener;
        gcn::Font *mGuiFont;                  /**< The global GUI font */
        gcn::Font *mInfoParticleFont;         /**< Font for Info Particles*/
        bool mCustomCursor;                   /**< Show custom cursor */
        ImageSet *mMouseCursors;              /**< Mouse cursor images */
        float mMaxMouseCursorAlpha;           /**< Cursor opacity/transparency. */
        float mMouseCursorAlpha;              /**< Current cursor opacity/transparency.  Changes as the cursor fades out due to inactivity. */
        int mMouseInactivityTimer;
        int mCursorType;

        /** Used to determine whether to draw the next frame. */
        int mDrawTime;

        /** The minimum frame time (used for frame limiting). */
        int mMinFrameTime;
};

/**
 * Returns elapsed time. (Warning: supposes the delay is always < 100 seconds)
 */
int get_elapsed_time(int start_time);

extern Gui *gui;                              /**< The GUI system */
extern SDLInput *guiInput;                    /**< GUI input */

/**
 * Bolded text font
 */
extern gcn::Font *boldFont;

#endif
