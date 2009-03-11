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

#ifndef WINDOW_H
#define WINDOW_H

#include <guichan/widgetlistener.hpp>

#include <guichan/widgets/window.hpp>

#include "../graphics.h"
#include "../guichanfwd.h"

class ConfigListener;
class GCContainer;
class ContainerPlacer;
class Layout;
class LayoutCell;
class ResizeGrip;
class Skin;
class SkinLoader;
class WindowContainer;

/**
 * A window. This window can be dragged around and has a title bar. Windows are
 * invisible by default.
 *
 * \ingroup GUI
 */
class Window : public gcn::Window, gcn::WidgetListener
{
    public:
        friend class WindowConfigListener;

        /**
         * Constructor. Initializes the title to the given text and hooks
         * itself into the window container.
         *
         * @param caption The initial window title, "Window" by default.
         * @param modal   Block input to other windows.
         * @param parent  The parent window. This is the window standing above
         *                this one in the window hiearchy. When reordering,
         *                a window will never go below its parent window.
         * @param skin    The location where the window's skin XML can be found.
         */
        Window(const std::string &caption = "Window", bool modal = false,
               Window *parent = NULL, const std::string &skin = "graphics/gui/gui.xml");

        /**
         * Destructor. Deletes all the added widgets.
         */
        ~Window();

        /**
         * Sets the window container to be used by new windows.
         */
        static void setWindowContainer(WindowContainer *windowContainer);

        /**
         * Draws the window.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Sets the size of this window.
         */
        void setContentSize(int width, int height);

        /**
         * Sets the location relative to the given widget.
         */
        void setLocationRelativeTo(gcn::Widget *widget);

        /**
         * Sets the location relative to the given enumerated position.
         */
        void setLocationRelativeTo(ImageRect::ImagePosition position,
                                   int offsetX = 0, int offsetY = 0);

        /**
         * Sets whether or not the window can be resized.
         */
        void setResizable(bool resize);

        /**
         * Called whenever the widget changes size.
         */
        void widgetResized(const gcn::Event &event);

        /**
         * Sets whether or not the window has a close button.
         */
        void setCloseButton(bool flag);

        /**
         * Returns whether the window can be resized.
         */
        bool isResizable();

        /**
         * Sets the minimum width of the window.
         */
        void setMinWidth(unsigned int width);

        /**
         * Sets the minimum height of the window.
         */
        void setMinHeight(unsigned int height);

        /**
         * Sets the maximum width of the window.
         */
        void setMaxWidth(unsigned int width);

        /**
         * Sets the minimum height of the window.
         */
        void setMaxHeight(unsigned int height);

        /**
         * Gets the minimum width of the window.
         */
        int getMinWidth() { return mMinWinWidth; }

        /**
         * Gets the minimum height of the window.
         */
        int getMinHeight() { return mMinWinHeight; }

        /**
         * Gets the maximum width of the window.
         */
        int getMaxWidth() { return mMaxWinWidth; }

        /**
         * Gets the minimum height of the window.
         */
        int getMaxHeight() { return mMaxWinHeight; }

        /**
         * Sets flag to show a title or not.
         */
        void setShowTitle(bool flag) { mShowTitle = flag; }

        /**
          * Sets whether the window is sticky. A sticky window will not have
          * its visibility set to false on a general setVisible(false) call.
          */
        void setSticky(bool sticky);

        /**
         * Returns whether the window is sticky.
         */
        bool isSticky();

        /**
         * Overloads window setVisible by Guichan to allow sticky window
         * handling.
         */
        void setVisible(bool visible);

        /**
         * Returns the parent window.
         *
         * @return The parent window or <code>NULL</code> if there is none.
         */
        Window* getParentWindow() { return mParent; }

        /**
         * Schedule this window for deletion. It will be deleted at the start
         * of the next logic update.
         */
        void scheduleDelete();

        /**
         * Starts window resizing when appropriate.
         */
        void mousePressed(gcn::MouseEvent &event);

        /**
         * Implements window resizing and makes sure the window is not
         * dragged/resized outside of the screen.
         */
        void mouseDragged(gcn::MouseEvent &event);

        /**
         * Implements custom cursor image changing context, based on mouse
         * relative position.
         */
        void mouseMoved(gcn::MouseEvent &event);

        /**
         * When the mouse button has been let go, this ensures that the mouse
         * custom cursor is restored back to it's standard image.
         */
        void mouseReleased(gcn::MouseEvent &event);

        /**
         * When the mouse leaves the window this ensures that the custom cursor
         * is restored back to it's standard image.
         */
        void mouseExited(gcn::MouseEvent &event);

        /**
         * Sets the name of the window. This is not the window title.
         */
        void setWindowName(const std::string &name) { mWindowName = name; }

        /**
         * Returns the name of the window. This is not the window title.
         */
        const std::string& getWindowName() { return mWindowName; }

        /**
         * Reads the position (and the size for resizable windows) in the
         * configuration based on the given string.
         * Uses the default values when config values are missing.
         * Don't forget to set these default values and resizable before
         * calling this function.
         */
        void loadWindowState();

        /**
         * Saves the window state so that when the window is reloaded, it'll
         * maintain its previous state and location.
         */
        void saveWindowState();

        /**
         * Set the default win pos and size.
         * (which can be different of the actual ones.)
         */
        void setDefaultSize(int defaultX, int defaultY,
                            int defaultWidth, int defaultHeight);

        /**
         * Set the default win pos and size.
         * (which can be different of the actual ones.)
         * This version of setDefaultSize sets the window's position based
         * on a relative enumerated position, rather than a coordinate position.
         */
        void setDefaultSize(int defaultWidth, int defaultHeight,
                            ImageRect::ImagePosition position,
                            int offsetx = 0, int offsetY = 0);

        /**
         * Reset the win pos and size to default. Don't forget to set defaults
         * first.
         */
        void resetToDefaultSize();

        /**
         * Gets the layout handler for this window.
         */
        Layout &getLayout();

        /**
         * Computes the position of the widgets according to the current
         * layout. Resizes the window so that the layout fits. Deletes the
         * layout.
         * @param w if non-zero, force the window to this width.
         * @param h if non-zero, force the window to this height.
         * @note This function is meant to be called with fixed-size windows.
         */
        void reflowLayout(int w = 0, int h = 0);

        /**
         * Adds a widget to the window and sets it at given cell.
         */
        LayoutCell &place(int x, int y, gcn::Widget *, int w = 1, int h = 1);

        /**
         * Returns a proxy for adding widgets in an inner table of the layout.
         */
        ContainerPlacer getPlacer(int x, int y);

        /**
         * Overrideable functionality for when the window is to close. This
         * allows for class implementations to clean up or do certain actions
         * on window close they couldn't do otherwise.
         */
        virtual void close();

    protected:
        /** The window container windows add themselves to. */
        static WindowContainer *windowContainer;

    private:
        enum ResizeHandles
        {
            TOP    = 0x01,
            RIGHT  = 0x02,
            BOTTOM = 0x04,
            LEFT   = 0x08
        };

        /**
         * Determines if the mouse is in a resize area and returns appropriate
         * resize handles. Also initializes drag offset in case the resize
         * grip is used.
         *
         * @see ResizeHandles
         */
        int getResizeHandles(gcn::MouseEvent &event);

        void setGuiAlpha();

        GCContainer *mChrome;      /**< Contained container */
        ResizeGrip *mGrip;         /**< Resize grip */
        Window *mParent;           /**< The parent window */
        Layout *mLayout;           /**< Layout handler */
        std::string mWindowName;   /**< Name of the window */
        bool mShowTitle;           /**< Window has a title bar */
        bool mModal;               /**< Window is modal */
        bool mCloseButton;         /**< Window has a close button */
        bool mSticky;              /**< Window resists minimization */
        static bool mAlphaChanged; /**< Whether the alpha percent was changed */
        int mMinWinWidth;          /**< Minimum window width */
        int mMinWinHeight;         /**< Minimum window height */
        int mMaxWinWidth;          /**< Maximum window width */
        int mMaxWinHeight;         /**< Maximum window height */
        int mDefaultX;             /**< Default window X position */
        int mDefaultY;             /**< Default window Y position */
        int mDefaultWidth;         /**< Default window width */
        int mDefaultHeight;        /**< Default window height */

        /**
         * The config listener that listens to changes relevant to all windows.
         */
        static ConfigListener *windowConfigListener;

        static int mouseResize;    /**< Active resize handles */
        static int instances;      /**< Number of Window instances */

        Skin* mSkin;               /**< Skin in use by this window */

        /**
         * The width of the resize border. Is independent of the actual window
         * border width, and determines mostly the size of the corner area
         * where two borders are moved at the same time.
         */
        static const int resizeBorderWidth = 10;
};

#endif
