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

#include "scrollarea.h"

#include "../graphics.h"
#include "../gui.h"

#include "../../../core/configlistener.h"
#include "../../../core/configuration.h"
#include "../../../core/resourcemanager.h"

#include "../../../core/image/image.h"

#include "../../../core/utils/dtor.h"

int ScrollArea::instances = 0;
float ScrollArea::mAlpha = 1.0;
ScrollAreaConfigListener *ScrollArea::mConfigListener = NULL;

ImageRect ScrollArea::background;
ImageRect ScrollArea::vMarker;
Image *ScrollArea::buttons[4][2];

class ScrollAreaConfigListener : public ConfigListener
{
    public:
        ScrollAreaConfigListener(ScrollArea *sa):
            mScrollArea(sa)
        {}

        void optionChanged(const std::string &name)
        {
            if (name == "guialpha")
            {
                mScrollArea->mAlpha = config.getValue("guialpha", 0.8);

                for (int a = 0; a < 9; a++)
                {
                    mScrollArea->background.grid[a]->setAlpha(mScrollArea->mAlpha);
                    mScrollArea->vMarker.grid[a]->setAlpha(mScrollArea->mAlpha);
                }
            }
        }
    private:
        ScrollArea *mScrollArea;
};

ScrollArea::ScrollArea(bool gc, bool opaque):
    gcn::ScrollArea(),
    mOpaque(opaque),
    mGC(gc)
{
    init();
}

ScrollArea::ScrollArea(gcn::Widget *widget, bool gc, bool opaque):
    gcn::ScrollArea(widget),
    mOpaque(opaque),
    mGC(gc)
{
    init();
}

ScrollArea::~ScrollArea()
{
    // Garbage collection
    if (mGC)
        delete getContent();

    instances--;

    if (instances == 0)
    {
        config.removeListener("guialpha", mConfigListener);
        destroy(mConfigListener);

        for_each(background.grid, background.grid + 9, dtor<Image*>());
        for_each(vMarker.grid, vMarker.grid + 9, dtor<Image*>());

        buttons[UP][0]->decRef();
        buttons[UP][1]->decRef();
        buttons[DOWN][0]->decRef();
        buttons[DOWN][1]->decRef();
        buttons[LEFT][0]->decRef();
        buttons[LEFT][1]->decRef();
        buttons[RIGHT][0]->decRef();
        buttons[RIGHT][1]->decRef();
    }
}

void ScrollArea::init()
{
    // Draw background by default
    setOpaque(true);

    if (instances == 0)
    {
        mAlpha = config.getValue("guialpha", 0.8);

        // Load the background skin
        ResourceManager *resman = ResourceManager::getInstance();
        Image *textbox = resman->getImage("graphics/gui/deepbox.png");
        const int bggridx[4] = {0, 3, 28, 31};
        const int bggridy[4] = {0, 3, 28, 31};
        int a = 0, x, y;

        for (y = 0; y < 3; y++)
        {
            for (x = 0; x < 3; x++)
            {
                background.grid[a] = textbox->getSubImage(
                        bggridx[x], bggridy[y],
                        bggridx[x + 1] - bggridx[x] + 1,
                        bggridy[y + 1] - bggridy[y] + 1);
                background.grid[a]->setAlpha(config.getValue("guialpha", 0.8));
                a++;
            }
        }

        textbox->decRef();

        // Load vertical scrollbar skin
        Image *vscroll = resman->getImage("graphics/gui/vscroll_grey.png");
        int vsgridx[4] = {0, 4, 7, 11};
        int vsgridy[4] = {0, 4, 15, 19};
        a = 0;

        for (y = 0; y < 3; y++)
        {
            for (x = 0; x < 3; x++)
            {
                vMarker.grid[a] = vscroll->getSubImage(
                        vsgridx[x], vsgridy[y],
                        vsgridx[x + 1] - vsgridx[x],
                        vsgridy[y + 1] - vsgridy[y]);
                vMarker.grid[a]->setAlpha(config.getValue("guialpha", 0.8));
                a++;
            }
        }

        vscroll->decRef();

        buttons[UP][0] =
            resman->getImage("graphics/gui/vscroll_up_default.png");
        buttons[DOWN][0] =
            resman->getImage("graphics/gui/vscroll_down_default.png");
        buttons[LEFT][0] =
            resman->getImage("graphics/gui/hscroll_left_default.png");
        buttons[RIGHT][0] =
            resman->getImage("graphics/gui/hscroll_right_default.png");
        buttons[UP][1] =
            resman->getImage("graphics/gui/vscroll_up_pressed.png");
        buttons[DOWN][1] =
            resman->getImage("graphics/gui/vscroll_down_pressed.png");
        buttons[LEFT][1] =
            resman->getImage("graphics/gui/hscroll_left_pressed.png");
        buttons[RIGHT][1] =
            resman->getImage("graphics/gui/hscroll_right_pressed.png");

        mConfigListener = new ScrollAreaConfigListener(this);
        config.addListener("guialpha", mConfigListener);
    }

    mLastUpdate = tick_time;

    instances++;
}

void ScrollArea::logic()
{
    if (!isVisible())
    {
        mLastUpdate = tick_time;
        return;
    }

    gcn::ScrollArea::logic();
    gcn::Widget *content = getContent();

    // When no scrollbar in a certain direction, adapt content size to match
    // the content dimension exactly.
    if (content)
    {
        if (getHorizontalScrollPolicy() == gcn::ScrollArea::SHOW_NEVER)
        {
            content->setWidth(getChildrenArea().width -
                    2 * content->getFrameSize());
        }
        if (getVerticalScrollPolicy() == gcn::ScrollArea::SHOW_NEVER)
        {
            content->setHeight(getChildrenArea().height -
                    2 * content->getFrameSize());
        }
    }

    int updateTicks = get_elapsed_time(mLastUpdate) / 100;

    while (updateTicks > 0)
    {
        scroll();
        updateTicks--;
    }
}

void ScrollArea::draw(gcn::Graphics *graphics)
{
    if (mVBarVisible)
    {
        drawUpButton(graphics);
        drawDownButton(graphics);
        drawVBar(graphics);
        drawVMarker(graphics);
    }

    if (mHBarVisible)
    {
        drawLeftButton(graphics);
        drawRightButton(graphics);
        drawHBar(graphics);
        drawHMarker(graphics);
    }

    if (mHBarVisible && mVBarVisible)
    {
        graphics->setColor(getBaseColor());
        graphics->fillRectangle(gcn::Rectangle(getWidth() - mScrollbarWidth,
                    getHeight() - mScrollbarWidth,
                    mScrollbarWidth,
                    mScrollbarWidth));
    }

    drawChildren(graphics);
}

void ScrollArea::drawFrame(gcn::Graphics *graphics)
{
    if (mOpaque)
    {
        const int bs = getFrameSize();
        const int w = getWidth() + bs * 2;
        const int h = getHeight() + bs * 2;

        static_cast<Graphics*>(graphics)->drawImageRect(0, 0, w, h, background);
    }
}

void ScrollArea::setOpaque(bool opaque)
{
    mOpaque = opaque;

    setFrameSize(mOpaque ? 2 : 0);
}

void ScrollArea::drawButton(gcn::Graphics *graphics, BUTTON_DIR dir)
{
    int state = 0;
    gcn::Rectangle dim;

    switch (dir)
    {
        case UP:
            state = mUpButtonPressed ? 1 : 0;
            dim = getUpButtonDimension();
            break;
        case DOWN:
            state = mDownButtonPressed ? 1 : 0;
            dim = getDownButtonDimension();
            break;
        case LEFT:
            state = mLeftButtonPressed ? 1 : 0;
            dim = getLeftButtonDimension();
            break;
        case RIGHT:
            state = mRightButtonPressed ? 1 : 0;
            dim = getRightButtonDimension();
            break;
    }

    static_cast<Graphics*>(graphics)->
        drawImage(buttons[dir][state], dim.x, dim.y);
}

void ScrollArea::drawUpButton(gcn::Graphics *graphics)
{
    drawButton(graphics, UP);
}

void ScrollArea::drawDownButton(gcn::Graphics *graphics)
{
    drawButton(graphics, DOWN);
}

void ScrollArea::drawLeftButton(gcn::Graphics *graphics)
{
    drawButton(graphics, LEFT);
}

void ScrollArea::drawRightButton(gcn::Graphics *graphics)
{
    drawButton(graphics, RIGHT);
}

void ScrollArea::drawVBar(gcn::Graphics *graphics)
{
    const gcn::Rectangle dim = getVerticalBarDimension();
    graphics->setColor(gcn::Color(0, 0, 0, 32));
    graphics->fillRectangle(dim);
    graphics->setColor(gcn::Color(255, 255, 255));
}

void ScrollArea::drawHBar(gcn::Graphics *graphics)
{
    const gcn::Rectangle dim = getHorizontalBarDimension();
    graphics->setColor(gcn::Color(0, 0, 0, 32));
    graphics->fillRectangle(dim);
    graphics->setColor(gcn::Color(255, 255, 255));
}

void ScrollArea::drawVMarker(gcn::Graphics *graphics)
{
    gcn::Rectangle dim = getVerticalMarkerDimension();

    static_cast<Graphics*>(graphics)->
        drawImageRect(dim.x, dim.y, dim.width, dim.height, vMarker);
}

void ScrollArea::drawHMarker(gcn::Graphics *graphics)
{
    gcn::Rectangle dim = getHorizontalMarkerDimension();

    static_cast<Graphics*>(graphics)->drawImageRect(dim.x, dim.y, dim.width,
                                                    dim.height, vMarker);
}

void ScrollArea::scroll()
{
    if (mUpButtonPressed)
        setVerticalScrollAmount(getVerticalScrollAmount() -
                                mUpButtonScrollAmount);
    else if (mDownButtonPressed)
        setVerticalScrollAmount(getVerticalScrollAmount() +
                                mDownButtonScrollAmount);
    else if (mLeftButtonPressed)
        setHorizontalScrollAmount(getHorizontalScrollAmount() -
                                  mLeftButtonScrollAmount);
    else if (mRightButtonPressed)
        setHorizontalScrollAmount(getHorizontalScrollAmount() +
                                  mRightButtonScrollAmount);

    mLastUpdate = tick_time;}

void ScrollArea::mouseWheelMovedUp(gcn::MouseEvent& mouseEvent)
{
    if (mouseEvent.isConsumed())
        return;

    if (mVBarVisible && getVerticalScrollAmount() > 0)
    {
        const int vScroll = getVerticalScrollAmount() -
                            getChildrenArea().height / 8;

        setVerticalScrollAmount(vScroll);
        mouseEvent.consume();
    }
}

void ScrollArea::mouseWheelMovedDown(gcn::MouseEvent& mouseEvent)
{
    if (mouseEvent.isConsumed())
        return;

    if (mVBarVisible && getVerticalScrollAmount() < getVerticalMaxScroll())
    {
        const int vScroll = getVerticalScrollAmount() +
                            getChildrenArea().height / 8;

        setVerticalScrollAmount(vScroll);
        mouseEvent.consume();
    }
}

void ScrollArea::mousePressed(gcn::MouseEvent &mouseEvent)
{
    int x = mouseEvent.getX();
    int y = mouseEvent.getY();

    if (getUpButtonDimension().isPointInRect(x, y))
        mUpButtonPressed = true;
    else if (getDownButtonDimension().isPointInRect(x, y))
        mDownButtonPressed = true;
    else if (getLeftButtonDimension().isPointInRect(x, y))
        mLeftButtonPressed = true;
    else if (getRightButtonDimension().isPointInRect(x, y))
        mRightButtonPressed = true;
    else if (getVerticalMarkerDimension().isPointInRect(x, y))
    {
        mIsHorizontalMarkerDragged = false;
        mIsVerticalMarkerDragged = true;

        mVerticalMarkerDragOffset = y - getVerticalMarkerDimension().y;
    }
    else if (getVerticalBarDimension().isPointInRect(x,y))
    {
        if (y < getVerticalMarkerDimension().y)
            mUpButtonPressed = true;
        else
            mDownButtonPressed = true;
    }
    else if (getHorizontalMarkerDimension().isPointInRect(x, y))
    {
        mIsHorizontalMarkerDragged = true;
        mIsVerticalMarkerDragged = false;

        mHorizontalMarkerDragOffset = x - getHorizontalMarkerDimension().x;
    }
    else if (getHorizontalBarDimension().isPointInRect(x,y))
    {
        if (x < getHorizontalMarkerDimension().x)
            mLeftButtonPressed = true;
        else
            mRightButtonPressed = true;
    }

    scroll();
}
