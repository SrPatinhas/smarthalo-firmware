/*
 * HardwareTests.c
 *
 *	Running tests on the hardware, visual and communication tests
 *
 */

#include <CommunicationTask.h>
#include <SystemUtilitiesTask.h>
#include "GraphicsTask.h"
#include "BLEDriver.h"
#include "SensorsTask.h"
#include "Power.h"

void TestHandleTouch(uint16_t u16MessageLength, uint8_t * pu8Data){
    send_touch_test_update_SensorsTask();
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

void TestHandleHalo(uint16_t u16MessageLength, uint8_t * pu8Data){
    if(u16MessageLength < 1){
        genericResponse_CommunicationTask(eCOM_RETURN_STATUS_FAIL);
        return;
    }
    uint8_t brightness = 255;
    if(u16MessageLength > 1){
        brightness = pu8Data[1];
    }
    uint8_t offLED = 255;
    if(u16MessageLength > 2){
        offLED = pu8Data[2];
    }

    startLEDTest_GraphicsTask(pu8Data[0], brightness, offLED);
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

void TestHandleFront(uint16_t u16MessageLength, uint8_t * pu8Data){
    if(u16MessageLength == 1){
        testNightLight_GraphicsTask(pu8Data[0],0);
        genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
    }else{
        genericResponse_CommunicationTask(eCOM_RETURN_STATUS_FAIL);
    }
}

void TestHandleHardware(uint16_t u16MessageLength, uint8_t * pu8Data){
    uint8_t length = 2;
    uint8_t response[length];
    response[0] = eCOM_RETURN_STATUS_OK;
    response[1] = hardwareTests_SystemUtilities();
    sendData_CommunicationTask(BLE_TYPE_MSG, BLE_TX_COMMAND_BLE_RESPONSE, length, response);
}

void TestHandleOLED(uint16_t u16MessageLength, uint8_t * pu8Data){
    if(u16MessageLength == 0){
        oled_cmd_t oledCMD;
        oledCMD.statusBarPosition = -1;
        oledCMD.direction = oled_no_animation;
        oledCMD.animation = false;
        oledCMD.animationTypeMask = slidingAnimation;
        oledCMD.u8ImageCategory = oled_tests;
        oledCMD.u8ImageType = oled_test_display;
        setImage_GraphicsTask(&oledCMD);
        genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
        return;
    }

    if(u16MessageLength == 1){
        uint8_t u8Value = pu8Data[0];
        if(u8Value == 0) testOLEDByTurningOff_GraphicsTask();
        genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
        return;
    }

    if(u16MessageLength == 3){
        uint16_t u16Position = (pu8Data[0]<<8)+pu8Data[1];
        uint8_t u8Value = pu8Data[2];
        editDebugImage_GraphicsTask(u16Position, u8Value);
        genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
        return;
    }
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_FAIL);
}

void TestHandlePhoto(uint16_t u16MessageLength, uint8_t * pu8Data){
    bool bResp = false;
    uint16_t u16Ch1, u16Ch2;
    getPhotoData_SensorsTask(&u16Ch1, &u16Ch2, &bResp);

    uint8_t length = 5;
    uint8_t response[5] = {eCOM_RETURN_STATUS_OK,
            u16Ch1 >> 8,
            u16Ch1 & 0xff,
            u16Ch2 >> 8,
            u16Ch2 & 0xff};
    sendData_CommunicationTask(BLE_TYPE_MSG, BLE_TX_COMMAND_BLE_RESPONSE, length, response);
}

void TestHandlePower(uint16_t msglen, uint8_t *msg)
{
    extern bool disableAutoPower;

    if (msglen == 0) {
        genericResponse_CommunicationTask(eCOM_RETURN_STATUS_FAIL);
        return;
    }

    bool	onoff = (bool)(msg[0] ? true : false);

    // We send the response _before_ doing the deed on the
    // possibility that we decide to reduce/cut BLE power
    // in this command
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);

    disableAutoPower = onoff;
    setState_Power(onoff);
    startLEDTest_GraphicsTask(0,255,255);
    testOLEDByTurningOff_GraphicsTask();
    enableEarlyHalt_SystemUtilities();
}

/**
 * @brief Setup all the hardware test functions from BLE
 */
void init_HardwareTests(){
    assignFunction_CommunicationTask(COM_TEST, TEST_TOUCH, &TestHandleTouch);
    assignFunction_CommunicationTask(COM_TEST, TEST_HALO, &TestHandleHalo);
    assignFunction_CommunicationTask(COM_TEST, TEST_FRONT, &TestHandleFront);
    assignFunction_CommunicationTask(COM_TEST, TEST_HARDWARE, &TestHandleHardware);
    assignFunction_CommunicationTask(COM_TEST, TEST_OLED, &TestHandleOLED);
    assignFunction_CommunicationTask(COM_TEST, TEST_PHOTO, &TestHandlePhoto);
    assignFunction_CommunicationTask(COM_TEST, TEST_POWER, TestHandlePower);
}
