/*
 * OLEDLIbrary.h
 *
 *  Created on: 20 Sep 2019
 *      Author: Matt
 */

#ifndef SMARTHALOOS_OLEDLIBRARY_H_
#define SMARTHALOOS_OLEDLIBRARY_H_

#include "GraphicsTask.h"
#include "OLEDLibrary.h"

struct OLEDLocation{
    uint8_t x;
    uint8_t y;
}__attribute__((packed));

struct OLEDLayout{
    struct OLEDLocation location[10];
}__attribute__((packed));

struct OLEDAsset{
    uint8_t width;
    uint8_t height;
    const uint8_t * asset;
}__attribute__((packed));

void unlockStatusWidth_OLEDLibrary();
void updateStatusWidthLocked_OLEDLibrary(uint8_t newQuantity);
void updateStatusWidth_OLEDLibrary(uint8_t newQuantity);
uint8_t alignRubik43Centered_OLEDLibrary(uint8_t * oledBuffer, const uint8_t * text, uint8_t y);
uint8_t alignRubik52Centered_OLEDLibrary(uint8_t * oledBuffer, const uint8_t * text, uint8_t y);
uint8_t alignRubik64Centered_OLEDLibrary(uint8_t * oledBuffer, const uint8_t * text, uint8_t y);
uint8_t alignPixellari16TextCentered_OLEDLibrary(uint8_t * oledBuffer, const uint8_t * text, uint8_t y);
void appendPixellari16CharactersToDisplay_OLEDLibrary(uint8_t * oledBuffer, const uint8_t * characters, uint8_t count, uint8_t location, uint8_t y);
uint8_t appendPixellari16CharacterToDisplay_OLEDLibrary(uint8_t * oledBuffer, uint8_t id, uint8_t location, uint8_t y);
void appendAssetToDisplay_OLEDLibrary(uint8_t * oledBuffer, struct OLEDAsset asset, struct OLEDLocation location);
const uint8_t * getOLEDImage_OLEDLibrary(oled_cmd_t * poled_cmd);
uint8_t * updateDrawingBoard_OLEDLibrary(uint16_t location, uint8_t value);
void getDisplayWithAnimation_OLEDLibrary(uint8_t * firstScreen, uint8_t * secondScreen, TickType_t delayTick, TickType_t durationTick, oledDirections_e direction, TickType_t curTick, uint8_t * display);
bool getOLEDNewImageState_OLEDLibrary(oled_cmd_t oledCmd, uint8_t * pOldImage, uint8_t * pNewImage, uint8_t * pBuffer, TickType_t curTick);

#endif /* SMARTHALOOS_OLEDLIBRARY_H_ */
