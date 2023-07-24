
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "platform.h"

#include "scheduler.h"

#include "twi.h"

#define MAX_PENDING_TRANSACTIONS    16


nrf_drv_twi_config_t const twi_config = {
   .scl                = I2C_SCL_PIN,
   .sda                = I2C_SDA_PIN,
   .frequency          = NRF_TWI_FREQ_400K,
   .interrupt_priority = APP_IRQ_PRIORITY_LOW
};

static app_twi_t m_app_twi = APP_TWI_INSTANCE(0);

bool twi_active = false;

void twi_powerOff_do(void *ctx) {
    //printf("twi OFF \r\n");
    twi_active = false;
    app_twi_uninit(&m_app_twi);

    //nRF52832 Rev 1 Errata [89] TWI: Static 400 µA current while using GPIOTE - Workaround
    *(volatile uint32_t *)0x40003FFC = 0;
    *(volatile uint32_t *)0x40003FFC;
    *(volatile uint32_t *)0x40003FFC = 1;

}

void twi_shutdown() {
    twi_powerOff_do(NULL);
}

/*
#define TWI_PIN_INIT_CONF     ( (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) \
                              | (GPIO_PIN_CNF_DRIVE_S0D1     << GPIO_PIN_CNF_DRIVE_Pos) \
                              | (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  \
                              | (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos) \
                              | (GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos))
*/

void twi_powerOn() {
    if(!twi_active) {
        //printf("twi ON \r\n");
        twi_active = true;
        uint32_t err_code;
        APP_TWI_INIT(&m_app_twi, &twi_config, MAX_PENDING_TRANSACTIONS, err_code);
        APP_ERROR_CHECK(err_code);

        //Disable internal pullups
        //NRF_GPIO->PIN_CNF[I2C_SCL_PIN] = TWI_PIN_INIT_CONF;
        //NRF_GPIO->PIN_CNF[I2C_SDA_PIN] = TWI_PIN_INIT_CONF;

    }
}

typedef struct {
    app_twi_transaction_t transaction;
    ret_code_t result;
    twi_cb_t cb;
    void *ctx;
} twi_transactionCtx_t;

void twi_sch_cb(void *ctx) {
    //printf("test_cb %d \r\n", result);
    twi_transactionCtx_t *transCtx = (twi_transactionCtx_t *)ctx;
    if(transCtx == NULL) {
        printf("E3! \r\n");
        return;
    }
    ret_code_t result = transCtx->result;
    twi_cb_t cb = transCtx->cb;
    void *cbctx = transCtx->ctx;
    nrf_free(transCtx);
    //printf("R1! %d\r\n", result);
    if(cb) {
        cb(result, cbctx);
    }
}

void twi_transaction_cb(ret_code_t result, void *ctx) {
    twi_transactionCtx_t *transCtx = (twi_transactionCtx_t *)ctx;
    if(transCtx == NULL) {
        printf("E2! %d\r\n", result);
        return;
    }
    transCtx->result = result;
    sch_oneshot_ctx(twi_sch_cb, 0, transCtx);
}

void twi_transaction_do(const app_twi_transfer_t *transfers, uint32_t numTransfers, twi_cb_t cb, void *ctx) {
    twi_transactionCtx_t *transCtx = (twi_transactionCtx_t *)nrf_malloc(sizeof(twi_transactionCtx_t));
    if(transCtx == NULL) {
        //cb(NRF_ERROR_NO_MEM, ctx);
        //return;
        ERR_CHECK("twi out of memory", 1);
    }
    transCtx->transaction.callback = twi_transaction_cb;
    transCtx->transaction.p_user_data = transCtx;
    transCtx->transaction.p_transfers = transfers;
    transCtx->transaction.number_of_transfers = numTransfers;
    transCtx->cb = cb;
    transCtx->ctx = ctx;
    twi_powerOn();
    uint32_t err_code = app_twi_schedule(&m_app_twi, (app_twi_transaction_t *)transCtx);
    if(err_code) {
        printf("E1! %d\r\n", err_code);
        twi_transaction_cb(err_code, transCtx);
    }
    sch_unique_oneshot(twi_powerOff_do, 1000);
}

void twi_init() {

    //
    //twi_powerOn();

}