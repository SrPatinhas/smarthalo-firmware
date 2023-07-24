#include "tsl_user.h"
#include "sh2_tsl.h"

void tsl_user_ConfigureThresholds(uint8_t * payload){
    // Thresholds
    MyLinRots_Param[0].ProxInTh = payload[0];            /**< Proximity state in threshold */
    MyLinRots_Param[0].ProxOutTh = payload[1];           /**< Proximity state out threshold */
    MyLinRots_Param[0].DetectInTh = payload[2];          /**< Detection state in threshold */
    MyLinRots_Param[0].DetectOutTh = payload[3];         /**< Detection state out threshold */
    MyLinRots_Param[0].CalibTh = payload[4];             /**< Calibration state threshold */
    // Debounce counters
    MyLinRots_Param[0].CounterDebCalib = payload[5];     /**< Debounce counter to enter in Calibration state */
    MyLinRots_Param[0].CounterDebProx = payload[6];      /**< Debounce counter to enter in Proximity state */
    MyLinRots_Param[0].CounterDebDetect = payload[7];    /**< Debounce counter to enter in Detect state */
    MyLinRots_Param[0].CounterDebRelease = payload[8];   /**< Debounce counter to enter in Release state */
    MyLinRots_Param[0].CounterDebError = payload[9];     /**< Debounce counter to enter in Error state */
    MyLinRots_Param[0].CounterDebDirection = payload[10]; /**< Debounce counter for the direction change */
    // Other parameters
    MyLinRots_Param[0].Resolution = payload[11];          /**< Position resolution */
    MyLinRots_Param[0].DirChangePos = payload[12];        /**< Direction change position threshold */
}

void tsl_user_ConfigureMaxThreshold(uint8_t max){
    uint8_t limit = max < 10 ? 10 : max;
    MyLinRots_Param[0].ProxInTh = limit-5;           /**< Proximity state in threshold */
    MyLinRots_Param[0].ProxOutTh = limit-10;          /**< Proximity state out threshold */
    MyLinRots_Param[0].DetectInTh = limit;            /**< Detection state in threshold */
    MyLinRots_Param[0].DetectOutTh = limit-5;        /**< Detection state out threshold */
    /* USER CODE END Tsl_user_SetThresholds */
}
