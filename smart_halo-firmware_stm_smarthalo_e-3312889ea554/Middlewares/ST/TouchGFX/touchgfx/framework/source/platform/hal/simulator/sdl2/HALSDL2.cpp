/**
  ******************************************************************************
  * This file is part of the TouchGFX 4.10.0 distribution.
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#include <platform/hal/simulator/sdl2/HALSDL2.hpp>
#include <vector>
#include <cmath>
#include <string>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_shape.h>
#include <SDL2/SDL_syswm.h>
#include <touchgfx/Utils.hpp>

#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
#elif defined(__GNUC__)
#include <sys/types.h>
#include <sys/stat.h>
#endif

#ifdef __GNUC__
#define sprintf_s snprintf
#define freopen_s(pFile,filename,mode,pStream) (((*(pFile))=freopen((filename),(mode),(pStream)))==NULL)
#define localtime_s(timeinfo,rawtime) memcpy(timeinfo,localtime(rawtime),sizeof(tm))
#define strncpy_s(dst,dstsize,src,srcsize) strncpy(dst,src,dstsize<srcsize?dstsize:srcsize)
#define wcstombs_s(result,dst,dstsize,src,srcsize) *result=wcstombs(dst,src,dstsize<srcsize?dstsize:srcsize)
#define memcpy_s(dst,dstsize,src,srcsize) memcpy(dst,src,dstsize<srcsize?dstsize:srcsize)
#endif

namespace touchgfx
{
static bool isAlive = true;
static bool sdl_initialized = false;
static int screenshotcount = 0;
static uint8_t* rotated = NULL;
static uint8_t* tft24bpp = NULL;
static SDL_Window* simulatorWindow = 0;
static SDL_Renderer* simulatorRenderer = 0;

static uint16_t* tft;

void HALSDL2::renderLCD_FrameBufferToMemory(const Rect& _rectToUpdate, uint8_t* frameBuffer)
{
    Rect rectToUpdate = _rectToUpdate;

    if (isSkinActive && currentSkin != 0 && (currentSkin->isOpaque || currentSkin->hasSemiTransparency))
    {
        // Opaque skin must be drawn before the framebuffer
        SDL_Texture* currentSkinTexture = SDL_CreateTextureFromSurface(simulatorRenderer, currentSkin->surface);
        SDL_RenderClear(simulatorRenderer);
        SDL_RenderCopy(simulatorRenderer, currentSkinTexture, NULL, NULL);
        // The skin will overwrite everything, so expand the rect to the entire framebuffer
        rectToUpdate.x = 0;
        rectToUpdate.y = 0;
        rectToUpdate.width = DISPLAY_WIDTH;
        rectToUpdate.height = DISPLAY_HEIGHT;
        SDL_DestroyTexture(currentSkinTexture);
    }

    // This is a hack because SDL2 does not redraw its framebuffer on screen after screensaver
    rectToUpdate.x = 0;
    rectToUpdate.y = 0;
    rectToUpdate.width = DISPLAY_WIDTH;
    rectToUpdate.height = DISPLAY_HEIGHT;

    if (flashInvalidatedRect)
    {
        SDL_Delay(1);
    }

    // Now draw the requested area
    SDL_Surface* framebufferSurface = SDL_CreateRGBSurfaceFrom((void*)frameBuffer, DISPLAY_WIDTH, DISPLAY_HEIGHT, 24, 3 * DISPLAY_WIDTH, 0, 0, 0, 0);
    SDL_Texture* framebufferTexture = SDL_CreateTextureFromSurface(simulatorRenderer, framebufferSurface);

    SDL_Rect srcRect;
    srcRect.x = rectToUpdate.x;
    srcRect.y = rectToUpdate.y;
    srcRect.h = rectToUpdate.height;
    srcRect.w = rectToUpdate.width;

    SDL_Rect dstRect = srcRect;
    dstRect.x = rectToUpdate.x + getCurrentSkinX();
    dstRect.y = rectToUpdate.y + getCurrentSkinY();

    SDL_RenderCopy(simulatorRenderer, framebufferTexture, &srcRect, &dstRect);

    if (isSkinActive && currentSkin != 0 && !(currentSkin->isOpaque || currentSkin->hasSemiTransparency))
    {
        // Non-opaque skin must be drawn last
        SDL_Texture* currentSkinTexture = SDL_CreateTextureFromSurface(simulatorRenderer, currentSkin->surface);
        SDL_RenderCopy(simulatorRenderer, currentSkinTexture, NULL, NULL);
        SDL_DestroyTexture(currentSkinTexture);
    }

    SDL_RenderPresent(simulatorRenderer);
    SDL_DestroyTexture(framebufferTexture);
    SDL_FreeSurface(framebufferSurface);
}

static void sdlCleanup2()
{
    if (sdl_initialized)
    {
        sdl_initialized = false; // Make sure we don't get in here again
        SDL_DestroyRenderer(simulatorRenderer);
        SDL_DestroyWindow(simulatorWindow);
        SDL_VideoQuit();
        SDL_Quit();
    }
}

Uint32 myTimerCallback2(Uint32 interval, void* param);

Uint32 myTimerCallback2(Uint32 interval, void* param)
{
    (void)param; // Unused

    SDL_Event event;
    SDL_UserEvent userevent;

    /* In this example, our callback pushes an SDL_USEREVENT event
     * into the queue, and causes ourself to be called again at the
     * same interval: */

    userevent.type = static_cast<Uint32>(SDL_USEREVENT);
    userevent.code = 0;
    userevent.data1 = 0;
    userevent.data2 = 0;

    event.type = static_cast<Uint32>(SDL_USEREVENT);
    event.user = userevent;

    SDL_PushEvent(&event);

    return interval;
}

bool HALSDL2::sdl_init(int argcount, char** args)
{
    (void)argcount; // Unused

    if (sdl_initialized)
    {
        touchgfx_printf("SDL already initialized\n");
        return false;
    }

#if defined(WIN32) || defined(_WIN32)
    strncpy_s(programPath, sizeof(programPath), args[0], strlen(args[0]));
    char* filenamePos = strrchr(programPath, '\\');
    if (filenamePos)
    {
        filenamePos++;    // Skip path separator
    }
    else
    {
        filenamePos = programPath;
    }
    *filenamePos = '\0';
#else
    strncpy_s(programPath, sizeof(programPath), args[0], strlen(args[0]));
    char* filenamePos = strrchr(programPath, '/');
    if (filenamePos)
    {
        filenamePos++;    // Skip path separator
    }
    else
    {
        filenamePos = programPath;
    }
    *filenamePos = '\0';
#endif

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    {
        touchgfx_printf("Unable to init SDL: %s\n", SDL_GetError());
        return false;
    }

    uint8_t bitDepth = lcd().bitDepth();
    // Allocate frame buffers
    if (bitDepth < 16)
    {
        // Round up to nearest byte alignment, then divide by 2 (rounded up) as it is measured in uint16's
        tft = new uint16_t[(((FRAME_BUFFER_WIDTH * bitDepth + 7) / 8) * FRAME_BUFFER_HEIGHT + 1) / 2 * 3];
        setFrameBufferStartAddress(tft, bitDepth);
        tft24bpp = new uint8_t[FRAME_BUFFER_WIDTH * FRAME_BUFFER_HEIGHT * 3];
    }
    else if (bitDepth == 16)
    {
        // Allocate size for three frame buffers
        tft = new uint16_t[FRAME_BUFFER_WIDTH * FRAME_BUFFER_HEIGHT * 3];
        setFrameBufferStartAddress(tft, bitDepth);
        tft24bpp = new uint8_t[FRAME_BUFFER_WIDTH * FRAME_BUFFER_HEIGHT * 3];
    }
    else if (bitDepth == 24 || bitDepth == 32)
    {
        // Allocate size for three frame buffers
        tft = reinterpret_cast<uint16_t*>(new uint8_t[(bitDepth / 8) * FRAME_BUFFER_WIDTH * FRAME_BUFFER_HEIGHT * 3]);
        setFrameBufferStartAddress(tft, bitDepth);
        tft24bpp = NULL;
    }
    rotated = new uint8_t[FRAME_BUFFER_WIDTH * FRAME_BUFFER_HEIGHT * 3];

    recreateWindow(false);
    if (simulatorWindow == NULL)
    {
        touchgfx_printf("Unable to set video mode: %s\n", SDL_GetError());
        return false;
    }

    SDL_SetWindowTitle(simulatorWindow, getWindowTitle());

    SDL_Surface* iconSurface = SDL_CreateRGBSurfaceFrom(icon, 32, 32, 16, 32 * 2, 0xf000, 0x0f00, 0x00f0, 0x000f);
    SDL_SetWindowIcon(simulatorWindow, iconSurface);
    SDL_FreeSurface(iconSurface);

#if defined(WIN32) || defined(_WIN32)
    FILE* stream;
    //sdl has hijacked output and error on windows
    const char* confile = "CONOUT$";
    // ignore error codes from calling freopen_s
    if (!freopen_s(&stream, confile, "w", stdout)) {}
    if (!freopen_s(&stream, confile, "w", stderr)) {}
#endif

    lcd().init();
    lockDMAToFrontPorch(false);
    atexit(sdlCleanup2);
    sdl_initialized = true;

    return true;
}

void HALSDL2::setWindowTitle(const char* title)
{
    customTitle = title;
}

const char* HALSDL2::getWindowTitle() const
{
    if (customTitle != 0)
    {
        return customTitle;
    }
    return "TouchGFX simulator";
}

void HALSDL2::loadSkin(touchgfx::DisplayOrientation orientation, int x, int y)
{
    char path[300];

    assert(sdl_initialized && "Please call sdl_init() before loading a skin");

    SkinInfo* skin;
    const char* name;

    if (orientation == ORIENTATION_PORTRAIT)
    {
        skin = &portraitSkin;
        name = "portrait";
    }
    else
    {
        skin = &landscapeSkin;
        name = "landscape";
    }
    skin->offsetX = 0;
    skin->offsetY = 0;
    sprintf_s(path, 300, "%s%s%s", programPath, name, ".png");
    skin->surface = IMG_Load(path);
    if (skin->surface == 0)
    {
        touchgfx_printf("Unable to load skin image from %s\n", programPath);
    }
    else
    {
        skin->offsetX = x;
        skin->offsetY = y;
        alphaChannelCheck(skin->surface, skin->isOpaque, skin->hasSemiTransparency);
    }
    if (getDisplayOrientation() == orientation)
    {
        updateCurrentSkin();
        recreateWindow(false);
    }
}

void HALSDL2::performDisplayOrientationChange()
{
    HAL::performDisplayOrientationChange();
    updateCurrentSkin();
    recreateWindow(false);
}

void HALSDL2::updateCurrentSkin()
{
    currentSkin = 0;
    if (getDisplayOrientation() == ORIENTATION_PORTRAIT)
    {
        if (portraitSkin.surface != 0)
        {
            currentSkin = &portraitSkin;
        }
    }
    else
    {
        if (landscapeSkin.surface != 0)
        {
            currentSkin = &landscapeSkin;
        }
    }
}

int HALSDL2::getCurrentSkinX() const
{
    return (isSkinActive && currentSkin != 0 && currentSkin->surface != 0) ? currentSkin->offsetX : 0;
}

int HALSDL2::getCurrentSkinY() const
{
    return (isSkinActive && currentSkin != 0 && currentSkin->surface != 0) ? currentSkin->offsetY : 0;
}

int32_t HALSDL2::_xMouse = 0;
int32_t HALSDL2::_yMouse = 0;
int32_t HALSDL2::_x = 0;
int32_t HALSDL2::_y = 0;
bool HALSDL2::isWindowBeingDragged = false;
int HALSDL2::initialWindowX;
int HALSDL2::initialWindowY;
int HALSDL2::initialMouseX;
int HALSDL2::initialMouseY;
bool HALSDL2::_lastTouch = false;
bool HALSDL2::_touches[5] = { false, false, false, false, false };
int HALSDL2::_numTouches = 0;

void HALSDL2::pushTouch(bool down) const
{
    if (_numTouches == 0)
    {
        // Save touch
        _touches[_numTouches++] = down;
    }
    else if ((_numTouches < 4) && (_touches[_numTouches - 1] ^ down)) //lint !e514
    {
        // Only save touch if is different from the last one recorded
        _touches[_numTouches++] = down;
    }
}

bool HALSDL2::popTouch() const
{
    if (_numTouches < 1)
    {
        // Keep returning the same state
        return _lastTouch;
    }
    // Get first item in queue
    _lastTouch = _touches[0];
    // Move items in queue
    for (int i = 0; i < 4; i++)
    {
        _touches[i] = _touches[i + 1];
    }
    _numTouches--;
    return _lastTouch;
}

void HALSDL2::updateTitle(int32_t x, int32_t y) const
{
    char title[500];
    int length = sprintf_s(title, 500, "%s", getWindowTitle());
    if (flashInvalidatedRect)
    {
        length += sprintf_s(title + length, 500 - length, " (flashmode)");
    }
    if (debugInfoEnabled)
    {
        length += sprintf_s(title + length, 500 - length, " @ %d,%d", x, y);
    }
    SDL_SetWindowTitle(simulatorWindow, title);
}

void HALSDL2::alphaChannelCheck(SDL_Surface* surface, bool& isOpaque, bool& hasSemiTransparency)
{
    isOpaque = true;
    hasSemiTransparency = false;
    if (surface->format->BitsPerPixel < 32 || surface->format->BytesPerPixel < 4)
    {
        return;
    }
    uint32_t alpha = surface->format->Amask;
    uint8_t* data = (uint8_t*)surface->pixels;
    for (int y = 0; y < surface->h; y++)
    {
        for (int x = 0; x < surface->w; x++)
        {
            uint32_t a = alpha & *(uint32_t*)(data + y * surface->pitch + x * 4);
            if (a == 0)
            {
                isOpaque = false;
                if (hasSemiTransparency)
                {
                    return;
                }
            }
            else if (a < alpha)
            {
                hasSemiTransparency = true;
                if (!isOpaque)
                {
                    return;
                }
            }
        }
    }
    return;
}

bool HALSDL2::doSampleTouch(int32_t& x, int32_t& y) const
{
    x = _x - getCurrentSkinX();
    y = _y - getCurrentSkinY();

    if (HAL::DISPLAY_ROTATION == rotate90)
    {
        int32_t tmp = x;
        x = y;
        y = DISPLAY_WIDTH - tmp;
    }
    return popTouch();
}

uint8_t HALSDL2::keyPressed = 0;

bool HALSDL2::sampleKey(uint8_t& key)
{
    if (keyPressed)
    {
        key = keyPressed;
        keyPressed = 0;
        return true;
    }
    return false;
}

void HALSDL2::taskEntry()
{
    uint32_t lastTick = SDL_GetTicks();
    SDL_AddTimer(1, myTimerCallback2, 0); // Start timer

    SDL_Event event;
    while (SDL_WaitEvent(&event) && isAlive)
    {
        switch (event.type)
        {
        case SDL_USEREVENT:
            {
                uint32_t thisTick = SDL_GetTicks();
                int msSinceLastTick = thisTick - lastTick;
                lastTick = thisTick;

                msPassed += msSinceLastTick;
                if (msPassed >= msBetweenTicks)
                {
                    while (msPassed >= msBetweenTicks)
                    {
                        msPassed -= msBetweenTicks;
                        vSync();
                    }
                    backPorchExited();
                    frontPorchEntered();
                    if (screenshotcount > 0)
                    {
                        screenshotcount--;
                        saveScreenshot();
                    }
                }
                break;
            }

        case SDL_MOUSEMOTION:
            {
                _xMouse = event.motion.x;
                _yMouse = event.motion.y;
                if (debugInfoEnabled)
                {
                    updateTitle(_xMouse, _yMouse);
                }
                if ((event.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0) //lint !e778 !e845
                {
                    _x = _xMouse;
                    _y = _yMouse;
                    pushTouch(true);
                }
                if (isWindowBeingDragged)
                {
                    int newMouseX;
                    int newMouseY;
                    SDL_GetGlobalMouseState(&newMouseX, &newMouseY);
                    SDL_SetWindowPosition(simulatorWindow, initialWindowX + (newMouseX - initialMouseX), initialWindowY + (newMouseY - initialMouseY));
                }
                break;
            }

        case SDL_MOUSEBUTTONDOWN:
            {
                SDL_CaptureMouse(SDL_TRUE);
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    _x = event.motion.x;
                    _y = event.motion.y;
                    pushTouch(true);
                }
                isWindowBeingDragged = (event.button.button == SDL_BUTTON_RIGHT);
                if (isWindowBeingDragged)
                {
                    SDL_GetWindowPosition(simulatorWindow, &initialWindowX, &initialWindowY);
                    SDL_GetGlobalMouseState(&initialMouseX, &initialMouseY);
                }
                break;
            }

        case SDL_MOUSEBUTTONUP:
            {
                SDL_CaptureMouse(SDL_FALSE);
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    pushTouch(false);
                }
                if (isWindowBeingDragged)
                {
                    int newMouseX;
                    int newMouseY;
                    SDL_GetGlobalMouseState(&newMouseX, &newMouseY);
                    SDL_SetWindowPosition(simulatorWindow, initialWindowX + (newMouseX - initialMouseX), initialWindowY + (newMouseY - initialMouseY));
                    isWindowBeingDragged = false;
                }
                break;
            }

        case SDL_TEXTINPUT:
            if (strlen(event.text.text) == 1)
            {
                keyPressed = (uint8_t)(event.text.text[0]);
            }
            break;

        case SDL_KEYUP:
            {
                if (event.key.keysym.sym == SDLK_F1)
                {
                    debugInfoEnabled = !debugInfoEnabled;
                    updateTitle(_xMouse, _yMouse);
                }
                else if (event.key.keysym.sym == SDLK_F2)
                {
                    flashInvalidatedRect = !flashInvalidatedRect;
                    updateTitle(_xMouse, _yMouse);
                }
                else if (event.key.keysym.sym == SDLK_F3)
                {
                    if (event.key.keysym.mod & KMOD_CTRL)
                    {
                        // Repeat
                        saveNextScreenshots(50);
                    }
                    else if (event.key.keysym.mod & KMOD_SHIFT)
                    {
                        // clipboard
                        copyScreenshotToClipboard();
                    }
                    else if (event.key.keysym.mod & KMOD_ALT)
                    {
                        // Do nothing
                    }
                    else if (event.key.keysym.mod & KMOD_GUI)
                    {
                        // Do nothing
                    }
                    else
                    {
                        // No modifiers
                        saveScreenshot();
                    }
                }
                else if (event.key.keysym.sym == SDLK_F4)
                {
                    if (currentSkin != 0 && currentSkin->surface)
                    {
                        isSkinActive = !isSkinActive;
                    }
                    else
                    {
                        borderless = !borderless;
                    }
                    recreateWindow();
                }
                else if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    isAlive = false;
                }
                break;
            }

        case SDL_QUIT:
            {
                isAlive = false;
                break;
            }

        default:
            break;
        }
    }
}

void HALSDL2::recreateWindow(bool updateContent /*= true*/)
{
    int windowX = SDL_WINDOWPOS_UNDEFINED;
    int windowY = SDL_WINDOWPOS_UNDEFINED;
    if (simulatorWindow != 0)
    {
        // Save previous coordinates
        SDL_GetWindowPosition(simulatorWindow, &windowX, &windowY);
        SDL_DestroyRenderer(simulatorRenderer);
        SDL_DestroyWindow(simulatorWindow);
    }
    int width = HAL::DISPLAY_WIDTH;
    int height = HAL::DISPLAY_HEIGHT;
    if (isSkinActive && currentSkin != 0)
    {
        width = currentSkin->surface->w;
        height = currentSkin->surface->h;
    }
    if (isSkinActive && currentSkin != 0)
    {
        simulatorWindow = SDL_CreateShapedWindow(getWindowTitle(), windowX, windowY, width, height, SDL_WINDOW_BORDERLESS);
        SDL_WindowShapeMode mode;
        mode.mode = ShapeModeBinarizeAlpha;
        mode.parameters.binarizationCutoff = 255;
        SDL_SetWindowShape(simulatorWindow, currentSkin->surface, &mode);
        SDL_SetWindowSize(simulatorWindow, width, height);
        SDL_SetWindowPosition(simulatorWindow, windowX, windowY);
    }
    else
    {
        simulatorWindow = SDL_CreateWindow(getWindowTitle(), windowX, windowY, width, height, borderless ? SDL_WINDOW_BORDERLESS : 0);
    }
    simulatorRenderer = SDL_CreateRenderer(simulatorWindow, -1, 0);
    SDL_SetRenderDrawBlendMode(simulatorRenderer, SDL_BLENDMODE_ADD);
    if (updateContent)
    {
        Rect display(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
        renderLCD_FrameBufferToMemory(display, doRotate(scaleTo24bpp(getTFTFrameBuffer(), DISPLAY_WIDTH, DISPLAY_HEIGHT, lcd().bitDepth()), DISPLAY_WIDTH, DISPLAY_HEIGHT));
    }
    updateTitle(_xMouse, _yMouse);
    // Re-add window icon in case
    SDL_Surface* iconSurface = SDL_CreateRGBSurfaceFrom(icon, 32, 32, 16, 32 * 2, 0xf000, 0x0f00, 0x00f0, 0x000f);
    SDL_SetWindowIcon(simulatorWindow, iconSurface);
    SDL_FreeSurface(iconSurface);
}

uint16_t* HALSDL2::getTFTFrameBuffer() const
{
    return tft;
}

static Rect dirty(0, 0, 0, 0);

uint8_t* HALSDL2::scaleTo24bpp(uint16_t* src, uint16_t width, uint16_t height, uint16_t depth)
{
    if (depth == 24)
    {
        return reinterpret_cast<uint8_t*>(src);
    }
    if (depth < 1 || (depth & (depth - 1)) != 0)
    {
        assert(0 && "unsupported screen depth");
    }
    if (HAL::DISPLAY_ROTATION == rotate90)
    {
        uint16_t tmp = width;
        width = height;
        height = tmp;
    }
    assert(tft24bpp != NULL && "Output buffer for TFT not allocated");
    uint8_t* dest = tft24bpp;
    uint16_t pixelsPerByte = 8 / depth;
    uint16_t pixelFactor = (depth == 1) ? 0xFF : ((depth == 2) ? 0x55 : ((depth == 4) ? 0x11 : 0x01));
    uint8_t* buffer = reinterpret_cast<uint8_t*>(src);
    if (depth == 1) // Bits are stored in the wrong order in the framebuffer :-/
    {
        for (int srcY = 0; srcY < height; srcY++)
        {
            for (int srcXbyte = 0; srcXbyte < width * depth / 8; srcXbyte++)
            {
                uint8_t bufbyte = *buffer++;
                for (int srcXpixel = 0; srcXpixel < pixelsPerByte; srcXpixel++)
                {
                    uint8_t pixel = ((bufbyte << (srcXpixel * depth)) & 0xFF) >> (8 - depth);
                    uint8_t pixelByte = pixel * pixelFactor;
                    *dest++ = pixelByte;
                    *dest++ = pixelByte;
                    *dest++ = pixelByte;
                }
            }
            // Check if there is a partial byte left
            if ((width * depth) % 8 != 0)
            {
                uint8_t bufbyte = *buffer++;
                for (int srcXpixel = 0; srcXpixel < (width * depth % 8) / depth; srcXpixel++)
                {
                    uint8_t pixel = ((bufbyte << (srcXpixel * depth)) & 0xFF) >> (8 - depth);
                    uint8_t pixelByte = pixel * pixelFactor;
                    *dest++ = pixelByte;
                    *dest++ = pixelByte;
                    *dest++ = pixelByte;
                }
            }
        }
    }
    else if (depth <= 8)
    {
        for (int srcY = 0; srcY < height; srcY++)
        {
            for (int srcXbyte = 0; srcXbyte < width * depth / 8; srcXbyte++)
            {
                uint8_t bufbyte = *buffer++;
                for (int srcXpixel = 0; srcXpixel < pixelsPerByte; srcXpixel++)
                {
                    uint8_t pixel = bufbyte & ((1 << depth) - 1);
                    bufbyte >>= depth;
                    uint8_t pixelByte = pixel * pixelFactor;
                    *dest++ = pixelByte;
                    *dest++ = pixelByte;
                    *dest++ = pixelByte;
                }
            }
            // Check if there is a partial byte left
            if ((width * depth) % 8 != 0)
            {
                uint8_t bufbyte = *buffer++;
                for (int srcXpixel = 0; srcXpixel < (width * depth % 8) / depth; srcXpixel++)
                {
                    uint8_t pixel = bufbyte & ((1 << depth) - 1);
                    bufbyte >>= depth;
                    uint8_t pixelByte = pixel * pixelFactor;
                    *dest++ = pixelByte;
                    *dest++ = pixelByte;
                    *dest++ = pixelByte;
                }
            }
        }
    }
    else if (depth == 16)
    {
        for (int srcY = 0; srcY < height; srcY++)
        {
            for (int srcX = 0; srcX < width; srcX++)
            {
                uint16_t bufword = *src++;
                uint8_t r = (bufword >> 8) & 0xF8;
                uint8_t g = (bufword >> 3) & 0xFC;
                uint8_t b = (bufword << 3) & 0xF8;
                *dest++ = b | (b >> 5);
                *dest++ = g | (g >> 6);
                *dest++ = r | (r >> 5);
            }
        }
    }

    return tft24bpp;
}

uint8_t* HALSDL2::doRotate(uint8_t* src, int16_t width, int16_t height)
{
    if (HAL::DISPLAY_ROTATION == rotate0)
    {
        return src;
    }
    else if (HAL::DISPLAY_ROTATION == rotate90)
    {
        uint8_t* source = reinterpret_cast<uint8_t*>(src);
        uint8_t* dest = reinterpret_cast<uint8_t*>(rotated);
        for (int srcX = 0; srcX < height; srcX++)
        {
            for (int srcY = 0; srcY < width; srcY++)
            {
                int dstX = width - 1 - srcY;
                int dstY = srcX;
                for (int i = 0; i < 3; i++)
                {
                    dest[(dstX + dstY * width) * 3 + i] = source[(srcX + srcY * height) * 3 + i];
                }
            }
        }
        return rotated;
    }
    else
    {
        return 0;
    }
}

void HALSDL2::setTFTFrameBuffer(uint16_t* adr)
{
    tft = adr;
    renderLCD_FrameBufferToMemory(dirty, doRotate(scaleTo24bpp(adr, DISPLAY_WIDTH, DISPLAY_HEIGHT, lcd().bitDepth()), DISPLAY_WIDTH, DISPLAY_HEIGHT));
    dirty = Rect(0, 0, 0, 0);
}

void HALSDL2::flushFrameBuffer()
{
    Rect display(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    flushFrameBuffer(display);
}

void HALSDL2::flushFrameBuffer(const Rect& rect)
{
    if (flashInvalidatedRect)
    {
        SDL_Rect flashRect;
        flashRect.x = rect.x + getCurrentSkinX();
        flashRect.y = rect.y + getCurrentSkinY();
        flashRect.w = rect.width;
        flashRect.h = rect.height;

        SDL_SetRenderDrawBlendMode(simulatorRenderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(simulatorRenderer, 0, 0, 0, 127);
        SDL_RenderFillRect(simulatorRenderer, &flashRect);
        SDL_RenderPresent(simulatorRenderer);
        SDL_SetRenderDrawBlendMode(simulatorRenderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(simulatorRenderer, 255, 255, 255, 127);
        SDL_RenderFillRect(simulatorRenderer, &flashRect);
        SDL_RenderPresent(simulatorRenderer);
    }

    dirty.expandToFit(rect);
    HAL::flushFrameBuffer(rect);
}

bool HALSDL2::blockCopy(void* RESTRICT dest, const void* RESTRICT src, uint32_t numBytes)
{
    return HAL::blockCopy(dest, src, numBytes);
}

void HALSDL2::blitSetTransparencyKey(uint16_t key)
{
    (void)key; // Unused
}

void HALSDL2::setVsyncInterval(float ms)
{
    msBetweenTicks = ms;
    msPassed = 0.0f;
}

void HALSDL2::saveScreenshot(char* folder, char* filename)
{
    const char* dir = "screenshots";
#if defined(WIN32) || defined(_WIN32)
    CreateDirectory(dir, 0);
#elif defined(__GNUC__)
    mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

    char fullPathAndName[100];
    if (folder)
    {
        sprintf_s(fullPathAndName, sizeof(fullPathAndName), "%s/%s", dir, folder);
#if defined(WIN32) || defined(_WIN32)
        CreateDirectory(fullPathAndName, 0);
#elif defined(__GNUC__)
        mkdir(fullPathAndName, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
        sprintf_s(fullPathAndName, sizeof(fullPathAndName), "%s/%s/%s", dir, folder, filename);
    }
    else
    {
        sprintf_s(fullPathAndName, sizeof(fullPathAndName), "%s/%s", dir, filename);
    }

    int width;
    int height;
    if (SDL_GetRendererOutputSize(simulatorRenderer, &width, &height) == 0)
    {
        // Create an empty surface that will be used to create the screenshot bmp file
        SDL_Surface* windowSurface = SDL_CreateRGBSurface(0, width, height, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
        if (windowSurface != 0)
        {
            // Read the pixels from the current render target and save them onto the surface
            SDL_RenderReadPixels(simulatorRenderer, NULL, SDL_GetWindowPixelFormat(simulatorWindow), windowSurface->pixels, windowSurface->pitch);

            // Create the bmp screenshot file
            SDL_SaveBMP(windowSurface, fullPathAndName);

            // Destroy the screenshot surface
            SDL_FreeSurface(windowSurface);
        }
    }
}

void HALSDL2::saveScreenshot()
{
    static char lastBaseName[100] = { 0 };
    static int counter = 0;

    // current date/time based on current system
    time_t t = time(0);
    tm localt;
    localtime_s(&localt, &t);

    char baseName[100];
    sprintf_s(baseName, sizeof(baseName), "img_%04d%02d%02d_%02d%02d%02d",
              1900 + localt.tm_year, localt.tm_mon + 1, localt.tm_mday,
              localt.tm_hour, localt.tm_min, localt.tm_sec);

    if (strncmp(baseName, lastBaseName, sizeof(baseName)) == 0)
    {
        // Same as previous time stamp. Add counter.
        counter++;
        sprintf_s(baseName, sizeof(baseName), "%s_%d.bmp", lastBaseName, counter);
    }
    else
    {
        // New time stamp. Save it and clear counter.
        strncpy_s(lastBaseName, sizeof(lastBaseName), baseName, sizeof(baseName));
        counter = 0;
        sprintf_s(baseName, sizeof(baseName), "%s.bmp", lastBaseName);
    }

    saveScreenshot(0, baseName);
}

void HALSDL2::saveNextScreenshots(int n)
{
    screenshotcount += n;
}

//copy 24 bit framebuffer to clipboard on Win32
void HALSDL2::copyScreenshotToClipboard()
{
#ifdef __linux__
    touchgfx_printf("Copying to clipboard has not been implemented for Linux\n");
#else
    if (!OpenClipboard(NULL))
    {
        touchgfx_printf("Unable to OpenClipboard\n");
        return;
    }

    if (!EmptyClipboard())
    {
        touchgfx_printf("Unable to EmptyClipboard\n");
        return;
    }

    uint8_t* buffer24 = doRotate(scaleTo24bpp(getTFTFrameBuffer(), DISPLAY_WIDTH, DISPLAY_HEIGHT, lcd().bitDepth()), DISPLAY_WIDTH, DISPLAY_HEIGHT);
    DWORD size_pixels = DISPLAY_WIDTH * DISPLAY_HEIGHT * 3;

    HGLOBAL hMem = GlobalAlloc(GHND, sizeof(BITMAPV5HEADER) + size_pixels);
    if (!hMem)
    {
        touchgfx_printf("Error allocating memory for bitmap data");
        return;
    }

    BITMAPV5HEADER* hdr = (BITMAPV5HEADER*)GlobalLock(hMem);
    if (!hdr)
    {
        touchgfx_printf("Error locking memory for bitmap data");
        GlobalFree(hMem);
        return;
    }

    memset(hdr, 0, sizeof(BITMAPV5HEADER));

    hdr->bV5Size = sizeof(BITMAPV5HEADER);
    hdr->bV5Width = DISPLAY_WIDTH;
    hdr->bV5Height = -DISPLAY_HEIGHT;
    hdr->bV5Planes = 1;
    hdr->bV5BitCount = 24;
    hdr->bV5Compression = BI_RGB;
    hdr->bV5SizeImage = size_pixels;
    hdr->bV5Intent = LCS_GM_GRAPHICS;
    hdr->bV5CSType = 0x57696E20;

    CopyMemory(hdr + 1, buffer24, size_pixels);
    GlobalUnlock(hMem);

    if (!SetClipboardData(CF_DIBV5, hMem))
    {
        touchgfx_printf("Unable to SetClipboardData\n");
    }

    CloseClipboard();
#endif
}

#ifndef __linux__
char** HALSDL2::getArgv(int* argc)
{
    LPWSTR cmdline = GetCommandLineW();
    LPWSTR* argvw = CommandLineToArgvW(cmdline, argc);
    char** argv = new char* [*argc];
    for (int i = 0; i < *argc; i++)
    {
        char buffer[1000];
        size_t numChars = wcslen(argvw[0]) + 1;
        wcstombs_s(&numChars, buffer, sizeof(buffer), argvw[i], numChars);
        argv[i] = new char[numChars];
        memcpy_s(argv[i], numChars, buffer, numChars);
        argv[i][numChars] = '\0';
    }
    return argv;
}
#endif
} // namespace touchgfx
