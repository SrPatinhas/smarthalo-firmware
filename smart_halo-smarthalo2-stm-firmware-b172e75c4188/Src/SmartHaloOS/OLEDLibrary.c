/*
 * OLEDLibrary.c
 *
 *  Created on: 20 Sep 2019
 *      Author: Matt
 */

#include "OLEDLibrary.h"
#include "OLEDDriver.h"
#include "event_groups.h"
#include "math.h"
#include <SystemUtilitiesTask.h>
#include "assets.h"

#define MAX_RUBIK64_ADVANCE 44
#define MAX_RUBIK52_ADVANCE 36
#define MAX_RUBIK43_ADVANCE 30
#define COLON_ID 58
#define PERIOD_ID 44
#define COMMA_ID 46

struct FontData{
    int8_t offsetX;
    int8_t offsetY;
    int8_t advance;
    uint8_t width;
    uint8_t id;
    uint8_t x;
    uint8_t y;
    uint8_t height;
}__attribute__((packed));

const static uint8_t * touchTestImages[4] = {shortTap, longTap, swipeLeft, swipeRight};

uint8_t drawingBoard[1024] = {0xFF};
static uint8_t statusQuantity = 10;
static uint8_t statusWidth = 13;
static bool isWidthLocked = false;


void unlockStatusWidth_OLEDLibrary(){
    isWidthLocked = false;
}

void updateStatusWidthLocked_OLEDLibrary(uint8_t newQuantity){
    statusWidth = round(1.f*SSD1305_LCDWIDTH/newQuantity);
    statusQuantity = newQuantity;
    isWidthLocked = true;
}

void updateStatusWidth_OLEDLibrary(uint8_t newQuantity){
    if(isWidthLocked) return;
    statusWidth = round(1.f*SSD1305_LCDWIDTH/newQuantity);
    statusQuantity = newQuantity;
}

uint8_t appendFontToDisplay(uint8_t * oledBuffer, struct FontData data, const uint8_t * font, uint8_t location, uint8_t y, int8_t extraOffset){
    uint8_t size = ceil(data.width*ceil(data.height/8.f));
    uint8_t bytes[size];
    memset(bytes,0,size);
    uint8_t yByte = floor(data.y/8.f);
    uint8_t yByteGap = data.y%8;
    uint8_t heightBytes = ceil(data.height/8.f);
    uint8_t heightByteGap = data.height%8;
    if(yByteGap || heightByteGap)
        yByte++;

    uint16_t byte = 0;
    for(int i=data.x;i<data.x+data.width;i++){
        for(int j=yByte-1;j<yByte+heightBytes-1;j++){
            uint8_t mask = j == yByte+heightBytes-2 && heightByteGap ? (0xff>>(8-heightByteGap))&0xff : 0xff;
            if(yByteGap){
                bytes[byte] += (font[i*16+j]>>yByteGap)&mask;
                bytes[byte] += (font[i*16+j+1]<<(8-yByteGap))&mask;
            }else{
                bytes[byte] += (font[i*16+j]>>yByteGap)&mask;
            }
            byte++;
            if(byte > (SSD1305_LCDWIDTH*SSD1305_LCDHEIGHT)/8-1)
                break;
        }
        if(byte > (SSD1305_LCDWIDTH*SSD1305_LCDHEIGHT)/8-1)
            break;
    }

    struct OLEDAsset asset = {.height=data.height,.width=data.width,.asset=bytes};
    //When the first character has a negative offset, remove it and compensate with
    //additional advance pixels. This Makes sure the character is added. Ignoring large
    //characters because they are injecting offsets to create even spacing
    if(location == 0 && data.offsetX < 0 && data.width < 12){
        data.advance += data.offsetX * -1;
        data.offsetX = 0;
    }
    struct OLEDLocation oledLocation = {.x=location+data.offsetX+extraOffset,.y=y-data.offsetY};
    appendAssetToDisplay_OLEDLibrary(oledBuffer, asset, oledLocation);
    return location + data.advance + extraOffset - 1;
}


struct FontData getFontData(uint8_t id, const uint8_t * structType, uint16_t size){
    uint16_t arrayPosition = 0;
    struct FontData fontData;
    while(arrayPosition < size){
        memcpy(&fontData, &structType[arrayPosition],sizeof(struct FontData));
        if(fontData.id == id)
            return fontData;
        arrayPosition+=8;
    }

    //returning empty data to indicate it doesn't exist
    memset(&fontData,0,sizeof(struct FontData));
    return fontData;
}

uint8_t appendRubikToDisplay(uint8_t * oledBuffer, uint8_t id, uint8_t location, uint8_t y, const uint8_t * structs, uint16_t structsSize, const uint8_t * font, uint8_t maxAdvance){
    struct FontData data = getFontData(id, structs, structsSize);
    if(data.id == 0)
        return location;
    bool setFixedWidth = id != COLON_ID && id != PERIOD_ID && id != COMMA_ID;
    if(setFixedWidth)
        data.offsetX -= ((maxAdvance-data.offsetX-data.width)%2 + (maxAdvance-data.advance)/2);
    return appendFontToDisplay(oledBuffer, data, font, location, y, setFixedWidth ? (maxAdvance-data.advance) : 0);
}

uint8_t appendRubik43CharacterToDisplay(uint8_t * oledBuffer, uint8_t id, uint8_t location, uint8_t y){
    return appendRubikToDisplay(oledBuffer,
            id,
            location,
            y,
            rubik43Structs,
            rubik43Structs_len,
            rubikFont43,
            MAX_RUBIK43_ADVANCE);
}

uint8_t appendRubik52CharacterToDisplay(uint8_t * oledBuffer, uint8_t id, uint8_t location, uint8_t y){
    return appendRubikToDisplay(oledBuffer,
            id,
            location,
            y,
            rubik52Structs,
            rubik52Structs_len,
            rubikFont52,
            MAX_RUBIK52_ADVANCE);
}

uint8_t appendRubik64CharacterToDisplay(uint8_t * oledBuffer, uint8_t id, uint8_t location, uint8_t y){
    return appendRubikToDisplay(oledBuffer,
            id,
            location,
            y,
            rubik64Structs,
            rubik64Structs_len,
            rubikFont64,
            MAX_RUBIK64_ADVANCE);
}

uint8_t alignTextToCenter(uint8_t * oledBuffer, const uint8_t * text, uint8_t y, uint8_t (* textFunction)(uint8_t * oledBuffer, uint8_t id, uint8_t location, uint8_t y)){
    uint8_t textBoard[8*128] = {0x00};
    uint8_t location = 0;
    for(int i=0;text[i]!=0;i++){
        if(textFunction == &appendPixellari16CharacterToDisplay_OLEDLibrary){
            struct FontData data = getFontData(text[i],pixellari16Structs, pixellari16Structs_len);
            if(location+data.advance-1 > 118){
                location = textFunction(textBoard, '.', location, y);
                location = textFunction(textBoard, '.', location, y);
                location = textFunction(textBoard, '.', location, y);
                break;
            }
        }
        location = textFunction(textBoard, text[i], location, y);
    }
    int8_t placement = 64-location/2;
    struct OLEDAsset asset = {.height = 64,.width=128,.asset=textBoard};
    struct OLEDLocation oledLocation = {.x=placement <= 0 ? 0 :placement-1,.y=0};
    appendAssetToDisplay_OLEDLibrary(oledBuffer,
            asset,
            oledLocation);

    //returning width of text
    return location - 1;
}

uint8_t alignRubik43Centered_OLEDLibrary(uint8_t * oledBuffer, const uint8_t * text, uint8_t y){
    return alignTextToCenter(oledBuffer, text, y, &appendRubik43CharacterToDisplay);
}

uint8_t alignRubik52Centered_OLEDLibrary(uint8_t * oledBuffer, const uint8_t * text, uint8_t y){
    return alignTextToCenter(oledBuffer, text, y, &appendRubik52CharacterToDisplay);
}

uint8_t alignRubik64Centered_OLEDLibrary(uint8_t * oledBuffer, const uint8_t * text, uint8_t y){
    return alignTextToCenter(oledBuffer, text, y, &appendRubik64CharacterToDisplay);
}

uint8_t alignPixellari16TextCentered_OLEDLibrary(uint8_t * oledBuffer, const uint8_t * text, uint8_t y){
    return alignTextToCenter(oledBuffer, text, y, &appendPixellari16CharacterToDisplay_OLEDLibrary);
}

void appendPixellari16CharactersToDisplay_OLEDLibrary(uint8_t * oledBuffer, const uint8_t * characters, uint8_t count, uint8_t location, uint8_t y){
    for(int i=0;i<count;i++){
        location = appendPixellari16CharacterToDisplay_OLEDLibrary(oledBuffer, characters[i],location,y);
    }
}

uint8_t appendPixellari16CharacterToDisplay_OLEDLibrary(uint8_t * oledBuffer, uint8_t id, uint8_t location, uint8_t y){
    struct FontData data = getFontData(id, pixellari16Structs, pixellari16Structs_len);
    if(data.id == 0)
        return location;
    return appendFontToDisplay(oledBuffer, data, pixellariFont16, location, y, 0);
}

void appendAssetToDisplay_OLEDLibrary(uint8_t * oledBuffer, struct OLEDAsset asset, struct OLEDLocation location){
    uint8_t page = location.y/8;
    uint8_t pagePartial = location.y%8;
    uint8_t pageHeight = ceil(asset.height/8.f);
    uint8_t heightPartial = asset.height%8;

    for(int xPos = location.x;xPos<=location.x+asset.width-1;xPos++){
        for(int yPos = page;yPos<=page+pageHeight-1;yPos++){
            uint16_t byteLocation = xPos*64/8+yPos;
            if(byteLocation > (SSD1305_LCDWIDTH*SSD1305_LCDHEIGHT)/8-1)
                break;
            uint16_t assetLocation = (xPos-location.x)*pageHeight+(yPos-page);
            uint8_t assetValue = asset.asset[assetLocation];
            uint8_t mask = yPos == page+pageHeight-1 && heightPartial ? (0xff>>(8-heightPartial))&0xff : 0xff;
            if(pagePartial){
                oledBuffer[byteLocation] |= ((assetValue & mask) << (pagePartial));
                oledBuffer[byteLocation+1] |= ((assetValue & mask) >> (8-pagePartial));
            }else{
                if(heightPartial && yPos == page+pageHeight-1)
                    assetValue = assetValue;
                oledBuffer[byteLocation] |= assetValue & mask;
            }
        }
    }
}

/**
 * @brief Get an OLED image from a preexisting library
 * @details A library of images exist for now that can be accessed via category and type
 */
const uint8_t * getOLEDImage_OLEDLibrary(oled_cmd_t * poled_cmd){
    if (poled_cmd->u8ImageCategory == oled_tests &&
            poled_cmd->u8ImageType >= oled_test_touch_short_tap &&
            poled_cmd->u8ImageType <= oled_test_display){
        if(poled_cmd->u8ImageType == oled_test_touch_complete){
            memset(drawingBoard, 0x00, 1024);
            return drawingBoard;
        }else if(poled_cmd->u8ImageType == oled_test_display){
            memset(drawingBoard, 0xff, 1024);
            return drawingBoard;
        }else{
            return (uint8_t*)touchTestImages[poled_cmd->u8ImageType];
        }
    }

    //OLED image does not exist
    return NULL;
}

/**
 * @brief Manipulate pixels on the OLED
 */
uint8_t * updateDrawingBoard_OLEDLibrary(uint16_t location, uint8_t value){
    drawingBoard[location] = value;
    return drawingBoard;
}

/**
 * @brief       OLEDDriverMergeBuffersVert()
 * @details     Function to merge 2 buffer when moving in vertical direction.
 * @public
 * @param[in]	bufferBottom: bottom buffer
 * @param[in]	bufferTop: top buffer
 * @param[out]	bufferOut: output buffer
 * @param[in]	rowIndex: row index.
 * @return      bool: true if success, false otherwise.
 */
static bool OLEDMergeBuffersVert(uint8_t * bufferBottom, uint8_t * bufferTop, uint8_t * bufferOut, uint32_t rowIndex)
{
    uint32_t u32PageIndex = rowIndex / 8;
    uint32_t u32BitIndex = rowIndex % 8;
    uint32_t u32Column = 0;
    uint32_t u32Page = 0;
    uint32_t u32Index = 0;
    uint32_t u32IndexTop = 0;
    uint32_t u32IndexBot = 0;

    if ((bufferBottom == NULL) || (bufferTop == NULL) || (bufferOut == NULL)) return false;

    for (u32Column = 0; u32Column < SSD1305_LCDWIDTH; u32Column++){
        for (u32Page = 0; u32Page < SSD1305_LCDHEIGHT / 8; u32Page++){
            u32Index = u32Page + u32Column * SSD1305_LCDHEIGHT / 8;
            u32IndexTop = ((u32Page - u32PageIndex + 7)) + u32Column * SSD1305_LCDHEIGHT / 8;
            u32IndexBot = ((u32Page - u32PageIndex)) + u32Column * SSD1305_LCDHEIGHT / 8;
            bufferOut[u32Index] = 0;
            if (u32Page < u32PageIndex){	// Use only Top buffer
                bufferOut[u32Index] = (bufferTop[u32IndexTop] >> (8 - u32BitIndex)) + (bufferTop[u32IndexTop + 1] << u32BitIndex);
            }else if (u32Page > u32PageIndex){	// Use Only Bottom buffer
                bufferOut[u32Index] = (bufferBottom[u32IndexBot - 1] >> (8 - u32BitIndex)) + (bufferBottom[u32IndexBot] << u32BitIndex);
            } else {
                bufferOut[u32Index] = (bufferBottom[u32IndexBot] << u32BitIndex) + (bufferTop[u32IndexTop] >> (8 - u32BitIndex));	// Not certain about the TOP... Cannot validate
            }
        }
    }
    return true;
}

/**
 * @brief       OLEDDriverMergeBuffersHorz()
 * @details     Function to merge 2 buffer when moving in horizontal direction.
 * @public
 * @param[in]	bufferLeft: Left buffer
 * @param[in]	bufferRight: Rigth buffer
 * @param[out]	bufferOut: output buffer
 * @param[in]	columnIndex: column index.
 * @return      bool: true if success, false otherwise.
 */
static bool OLEDMergeBuffersHorz(uint8_t * bufferLeft,  uint8_t * bufferRight, uint8_t * bufferOut, uint32_t columnIndex)
{
    uint32_t u32Row = 0;
    uint32_t u32Col = 0;
    uint32_t u32Index = 0;
    if ((bufferLeft == NULL) || (bufferRight == NULL) || (bufferOut == NULL)) return false;

    for (u32Col = 0; u32Col < SSD1305_LCDWIDTH; u32Col++) {
        for (u32Row = 0; u32Row < SSD1305_LCDHEIGHT / 8; u32Row++) {
            u32Index = u32Row + u32Col*(SSD1305_LCDHEIGHT/8);

            if (u32Index < columnIndex*SSD1305_LCDHEIGHT / 8) {	// Use Left buffer
                bufferOut[u32Index] = bufferLeft[u32Index + (SSD1305_LCDWIDTH-1)*8 - (columnIndex-1)*8];
            } else {	// Use Right buffer
                bufferOut[u32Index] = bufferRight[u32Index - columnIndex*SSD1305_LCDHEIGHT / 8];
            }
        }
    }
    return true;
}

/**
 * @brief       OLEDDriverAddStatusBar()
 * @details     Function to add a status bar
 * @public
 * @param[in]	pu8Buffer: buffer that contains the image
 * @param[in]	u8Position:
 * @param[in]	u8Delta:
 * @return      bool: true if success, false otherwise.
 */
static bool OLEDAddStatusBar(uint8_t * pu8Buffer, uint8_t u8Position, int8_t u8Delta)
{
    uint32_t u32RealPos = 0;
    int32_t u32Index = 0;
    if (pu8Buffer == NULL) return false;
    if(u8Position == 255) return true;

    u32RealPos = statusWidth*u8Position;
    for (u32Index = 0; u32Index < SSD1305_LCDWIDTH; ++u32Index)
    {
        if ((u32Index >= (u32RealPos + u8Delta)
                && u32Index < (u32RealPos + u8Delta + statusWidth)
                && u8Position < statusQuantity)){ //sliding
            pu8Buffer[8 * u32Index - 1] |= 0x30;
        }else if((u32Index < (u8Delta) //sliding from the first position
                && u8Position == statusQuantity-1)){
            pu8Buffer[8 * u32Index - 1] |= 0x30;
        }else if((u32Index > (SSD1305_LCDWIDTH + u8Delta) //sliding out to the end
                && u8Position == 0)){
            pu8Buffer[8 * u32Index - 1] |= 0x30;
        }else if( u32Index < statusWidth + u8Delta
                && u8Delta < 0
                && u8Position == 0){ //sliding in from the end
            pu8Buffer[8 * u32Index - 1] |= 0x30;
        }else {	// Erase 2 lower pixels, no slider here
            pu8Buffer[8 * u32Index - 1] &= 0xCF;
        }
    }
    return true;
}

/**
 * @brief Create animated state of an OLED screen
 * @details this will return an OLED screen build out of two screens with a transition in the specified direction.
 * The input screens here are assumed to be 128 by 64 in size.
 */
void getDisplayWithAnimation_OLEDLibrary(uint8_t * firstScreen, uint8_t * secondScreen, TickType_t delayTick, TickType_t durationTick, oledDirections_e direction, TickType_t curTick, uint8_t * display){
    if(curTick <= delayTick){
        memcpy(display,firstScreen,1024);
    }else if(curTick >= durationTick){
        memcpy(display,secondScreen,1024);
    }else{
        uint8_t progress = 100*(curTick-delayTick)/(durationTick-delayTick);
        uint16_t breakPoint = 8*round((128/2)*(progress)/100.f);
        if(direction == oled_right){
            memcpy(display,firstScreen,1024);
            memcpy(display,secondScreen,breakPoint);
            memcpy(&display[512],&secondScreen[512], breakPoint);
        }else{
            memcpy(display,firstScreen,1024);
            memcpy(&display[1024-breakPoint],&secondScreen[1024-breakPoint],breakPoint);
            memcpy(&display[512-breakPoint],&secondScreen[512-breakPoint], breakPoint);
        }
    }
}

/**
 * @brief Update OLED image with an animation state
 * @details this returns all the states of an animation one frame at a time
 */
bool getOLEDNewImageState_OLEDLibrary(oled_cmd_t oledCmd, uint8_t * pOldImage, uint8_t * pNewImage, uint8_t * pBuffer, TickType_t curTick){
    if(oledCmd.animation &&
            pOldImage != NULL &&
            oledCmd.direction != oled_no_animation){
        if(oledCmd.direction == oled_up || oledCmd.direction == oled_down){
            uint8_t progress = (SSD1305_LCDHEIGHT) * (curTick)/(1.f*oledCmd.animationTime);
            if(progress > SSD1305_LCDHEIGHT)
            	progress = SSD1305_LCDHEIGHT;
            if (oledCmd.direction == oled_up) {
                OLEDMergeBuffersVert(pNewImage, pOldImage, pBuffer, SSD1305_LCDHEIGHT - progress);
            }else{
                OLEDMergeBuffersVert(pOldImage, pNewImage, pBuffer, progress);
            }
            OLEDAddStatusBar(pBuffer, oledCmd.statusBarPosition, 0);
            return curTick < oledCmd.animationTime;
        }else{
            uint8_t progress = (SSD1305_LCDWIDTH) * (curTick*1.f)/(1.f*oledCmd.animationTime);
            if(progress > SSD1305_LCDWIDTH)
                progress = SSD1305_LCDWIDTH;
            if (oledCmd.direction == oled_left){
                OLEDMergeBuffersHorz(pOldImage, pNewImage,  pBuffer, SSD1305_LCDWIDTH - progress);
                OLEDAddStatusBar(pBuffer, oledCmd.statusBarPosition, (progress * statusWidth / (SSD1305_LCDWIDTH - 1)) - statusWidth);
            }else{
                OLEDMergeBuffersHorz(pNewImage, pOldImage,  pBuffer, progress);
                OLEDAddStatusBar(pBuffer, oledCmd.statusBarPosition, statusWidth - (progress * statusWidth / (SSD1305_LCDWIDTH - 1)));
            }
            return curTick < oledCmd.animationTime;
        }
    }else{
        memcpy(pBuffer, pNewImage, 1024);
        OLEDAddStatusBar(pBuffer, oledCmd.statusBarPosition, 0);
        return false;
    }
}
