/*
 *  The Mana World
 *  Copyright (C) 2006  The Mana World Development Team
 *
 *  This file is part of Aethyra based on code from The Mana World.
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

#include <typeinfo>

#include "textpreview.h"

#include "../gui.h"
#include "../palette.h"
#include "../skin.h"
#include "../textrenderer.h"
#include "../truetypefont.h"

float TextPreview::mAlpha = 1.0;

TextPreview::TextPreview(const std::string* text)
{
    mText = text;
    mTextAlpha = false;
    mFont = gui->getFont();
    mTextColor = &guiPalette->getColor(Palette::TEXT);
    mTextBGColor = NULL;
    mBGColor = &guiPalette->getColor(Palette::BACKGROUND);
    mOpaque = false;
}

void TextPreview::draw(gcn::Graphics* graphics)
{
    if (Skin::getAlpha() != mAlpha)
        mAlpha = Skin::getAlpha();

    const int alpha = (int) (mAlpha * 255.0f);
    const int textAlpha = mTextAlpha ? alpha : 255;

    if (mOpaque)
    {
        graphics->setColor(gcn::Color((int) mBGColor->r,
                                      (int) mBGColor->g,
                                      (int) mBGColor->b,
                                      alpha));
        graphics->fillRectangle(gcn::Rectangle(0, 0, getWidth(), getHeight()));
    }

    if (mTextBGColor && typeid(*mFont) == typeid(TrueTypeFont))
    {
        TrueTypeFont *font = static_cast<TrueTypeFont*>(mFont);
        int x = font->getWidth(*mText) + 1 + 2 * ((mOutline || mShadow) ? 1 :0);
        int y = font->getHeight() + 1 + 2 * ((mOutline || mShadow) ? 1 : 0);
        graphics->setColor(gcn::Color((int) mTextBGColor->r,
                                      (int) mTextBGColor->g,
                                      (int) mTextBGColor->b,
                                      alpha));
        graphics->fillRectangle(gcn::Rectangle(1, 1, x, y));
    }

    TextRenderer::renderText(graphics, *mText, 2, 2,  gcn::Graphics::LEFT,
                             gcn::Color(mTextColor->r, mTextColor->g,
                                        mTextColor->b, textAlpha),
                             mFont, mOutline, mShadow);
}
