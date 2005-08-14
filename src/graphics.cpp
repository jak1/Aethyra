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

#include "graphics.h"

#ifdef USE_OPENGL
#include <guichan/imagefont.hpp>
#endif

#include "log.h"
#include "main.h"

#include "graphic/imagerect.h"

#include "resources/image.h"

extern volatile int framesToDraw;


Graphics::Graphics():
    mScreen(0)
{
}

Graphics::~Graphics()
{
    _endDraw();
}

bool Graphics::setVideoMode(int w, int h, int bpp, bool fs, bool hwaccel)
{
    int displayFlags = SDL_ANYFORMAT;

    mFullscreen = fs;
    mHWAccel = hwaccel;

    if (fs) {
        displayFlags |= SDL_FULLSCREEN;
    }

#ifdef USE_OPENGL
    if (useOpenGL) {
        displayFlags |= SDL_OPENGL;
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    } else
#endif
    {
        if (hwaccel) {
            displayFlags |= SDL_HWSURFACE | SDL_DOUBLEBUF;
        } else {
            displayFlags |= SDL_SWSURFACE;
        }
    }

    mScreen = SDL_SetVideoMode(w, h, bpp, displayFlags);

    if (!mScreen) {
        return false;
    }

    char videoDriverName[64];

    if (SDL_VideoDriverName(videoDriverName, 64)) {
        logger->log("Using video driver: %s", videoDriverName);
    }
    else {
        logger->log("Using video driver: unkown");
    }

    const SDL_VideoInfo *vi = SDL_GetVideoInfo();

    logger->log("Possible to create hardware surfaces: %s",
            ((vi->hw_available) ? "yes" : "no"));
    logger->log("Window manager available: %s",
            ((vi->wm_available) ? "yes" : "no"));
    logger->log("Accelerated hardware to hardware blits: %s",
            ((vi->blit_hw) ? "yes" : "no"));
    logger->log("Accelerated hardware to hardware colorkey blits: %s",
            ((vi->blit_hw_CC) ? "yes" : "no"));
    logger->log("Accelerated hardware to hardware alpha blits: %s",
            ((vi->blit_hw_A) ? "yes" : "no"));
    logger->log("Accelerated software to hardware blits: %s",
            ((vi->blit_sw) ? "yes" : "no"));
    logger->log("Accelerated software to hardware colorkey blits: %s",
            ((vi->blit_sw_CC) ? "yes" : "no"));
    logger->log("Accelerated software to hardware alpha blits: %s",
            ((vi->blit_sw_A) ? "yes" : "no"));
    logger->log("Accelerated color fills: %s",
            ((vi->blit_fill) ? "yes" : "no"));
    logger->log("Available video memory: %d", vi->video_mem);

#ifdef USE_OPENGL
    if (useOpenGL) {
        // Setup OpenGL
        glViewport(0, 0, w, h);
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
        int gotDoubleBuffer;
        SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &gotDoubleBuffer);
        logger->log("Using OpenGL %s double buffering.",
                (gotDoubleBuffer ? "with" : "without"));

        setTargetPlane(w, h);
    } else
#endif
    {
        setTarget(mScreen);
    }

    return true;
}

bool Graphics::setFullscreen(bool fs)
{
    if (mFullscreen == fs) {
        return true;
    }

    return setVideoMode(mScreen->w, mScreen->h,
            mScreen->format->BitsPerPixel, fs, mHWAccel);
}

int Graphics::getWidth()
{
    return mScreen->w;
}

int Graphics::getHeight()
{
    return mScreen->h;
}

bool Graphics::drawImage(Image *image, int x, int y)
{
    return drawImage(image, 0, 0, x, y, image->bounds.w, image->bounds.h);
}

bool Graphics::drawImage(Image *image, int srcX, int srcY, int dstX, int dstY,
        int width, int height)
{
    srcX += image->bounds.x;
    srcY += image->bounds.y;

    if (!useOpenGL) {
        // Check that preconditions for blitting are met.
        if (!mScreen || !image->image) return false;

        SDL_Rect dstRect;
        SDL_Rect srcRect;
        dstRect.x = dstX; dstRect.y = dstY;
        srcRect.x = srcX; srcRect.y = srcY;
        srcRect.w = width;
        srcRect.h = height;

        if (SDL_BlitSurface(image->image, &srcRect, mScreen, &dstRect) < 0) {
            return false;
        }
    }
#ifdef USE_OPENGL
    else {
        // Find OpenGL texture coordinates
        float texX1 = srcX / (float)image->texWidth;
        float texY1 = srcY / (float)image->texHeight;
        float texX2 = (srcX + width) / (float)image->texWidth;
        float texY2 = (srcY + height) / (float)image->texHeight;

        glColor4f(1.0f, 1.0f, 1.0f, image->alpha);
        glBindTexture(GL_TEXTURE_2D, image->glimage);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);

        // Draw a textured quad -- the image
        glBegin(GL_QUADS);
        glTexCoord2f(texX1, texY1);
        glVertex3i(dstX, dstY, 0);

        glTexCoord2f(texX2, texY1);
        glVertex3i(dstX + width, dstY, 0);

        glTexCoord2f(texX2, texY2);
        glVertex3i(dstX + width, dstY + height, 0);

        glTexCoord2f(texX1, texY2);
        glVertex3i(dstX, dstY + height, 0);
        glEnd();

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
#endif
    return true;
}

void Graphics::drawImagePattern(Image *image, int x, int y, int w, int h)
{
    int iw = image->getWidth();
    int ih = image->getHeight();
    if (iw == 0 || ih == 0) return;

    int px = 0;                       // X position on pattern plane
    int py = 0;                       // Y position on pattern plane

    while (py < h) {
        while (px < w) {
            int dw = (px + iw >= w) ? w - px : iw;
            int dh = (py + ih >= h) ? h - py : ih;
            drawImage(image, 0, 0, x + px, y + py, dw, dh);
            px += iw;
        }
        py += ih;
        px = 0;
    }
}

void Graphics::drawImageRect(
        int x, int y, int w, int h,
        Image *topLeft, Image *topRight,
        Image *bottomLeft, Image *bottomRight,
        Image *top, Image *right,
        Image *bottom, Image *left,
        Image *center)
{
    // Draw the center area
    drawImagePattern(center,
            x + topLeft->getWidth(), y + topLeft->getHeight(),
            w - topLeft->getWidth() - topRight->getWidth(),
            h - topLeft->getHeight() - bottomLeft->getHeight());

    // Draw the sides
    drawImagePattern(top,
            x + topLeft->getWidth(), y,
            w - topLeft->getWidth() - topRight->getWidth(), top->getHeight());
    drawImagePattern(bottom,
            x + bottomLeft->getWidth(), y + h - bottom->getHeight(),
            w - bottomLeft->getWidth() - bottomRight->getWidth(),
            bottom->getHeight());
    drawImagePattern(left,
            x, y + topLeft->getHeight(),
            left->getWidth(),
            h - topLeft->getHeight() - bottomLeft->getHeight());
    drawImagePattern(right,
            x + w - right->getWidth(), y + topRight->getHeight(),
            right->getWidth(),
            h - topRight->getHeight() - bottomRight->getHeight());

    // Draw the corners
    drawImage(topLeft, x, y);
    drawImage(topLeft, x, y);
    drawImage(topRight, x + w - topRight->getWidth(), y);
    drawImage(bottomLeft, x, y + h - bottomLeft->getHeight());
    drawImage(bottomRight,
            x + w - bottomRight->getWidth(),
            y + h - bottomRight->getHeight());
}

void Graphics::drawImageRect(
        int x, int y, int w, int h,
        const ImageRect &imgRect)
{
    drawImageRect(x, y, w, h,
            imgRect.grid[0], imgRect.grid[2], imgRect.grid[6], imgRect.grid[8],
            imgRect.grid[1], imgRect.grid[5], imgRect.grid[7], imgRect.grid[3],
            imgRect.grid[4]);
}

void Graphics::updateScreen()
{
    if (useOpenGL) {
#ifdef USE_OPENGL
        glFlush();
        glFinish();
        SDL_GL_SwapBuffers();
#endif
    }
    else {
        SDL_Flip(mScreen);
    }

    // Decrement frame counter when using framerate limiting
    if (framesToDraw > 1) framesToDraw--;

    // Wait while we're not allowed to draw next frame yet
    while (framesToDraw == 1)
    {
        SDL_Delay(10);
    }
}

#ifdef USE_OPENGL
void Graphics::_beginDraw()
{
    if (useOpenGL) {
        gcn::OpenGLGraphics::_beginDraw();
    } else {
        gcn::SDLGraphics::_beginDraw();
    }
}

void Graphics::_endDraw()
{
    if (useOpenGL) {
        gcn::OpenGLGraphics::_endDraw();
    } else {
        gcn::SDLGraphics::_endDraw();
    }
}

void Graphics::setFont(gcn::ImageFont *font)
{
    if (!useOpenGL) {
        gcn::SDLGraphics::setFont(font);
    } else {
        gcn::OpenGLGraphics::setFont(font);
    }
}

void Graphics::drawText(const std::string &text,
        int x, int y, unsigned int alignment)
{
    if (!useOpenGL) {
        gcn::SDLGraphics::drawText(text, x, y, alignment);
    } else {
        gcn::OpenGLGraphics::drawText(text, x, y, alignment);
    }
}

void Graphics::setColor(gcn::Color color)
{
    if (!useOpenGL) {
        gcn::SDLGraphics::setColor(color);
    } else {
        gcn::OpenGLGraphics::setColor(color);
    }
}

void Graphics::popClipArea()
{
    if (!useOpenGL) {
        gcn::SDLGraphics::popClipArea();
    } else {
        gcn::OpenGLGraphics::popClipArea();
    }
}

bool Graphics::pushClipArea(gcn::Rectangle area)
{
    if (!useOpenGL) {
        return gcn::SDLGraphics::pushClipArea(area);
    } else {
        return gcn::OpenGLGraphics::pushClipArea(area);
    }
}

void Graphics::fillRectangle(const gcn::Rectangle &rectangle)
{
    if (!useOpenGL) {
        gcn::SDLGraphics::fillRectangle(rectangle);
    } else {
        gcn::OpenGLGraphics::fillRectangle(rectangle);
    }
}
#endif
