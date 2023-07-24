/**
 * Copyright (c) 2017 - 2017, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include "app_error.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include <stdarg.h>

#define DEBUG_BUFFER_SIZE          256u

void sys_assert_handler(const char * condition, const int line, const char * file)
{
    if ((condition == NULL) || (file == NULL))
    {
        APP_ERROR_HANDLER(NRF_ERROR_NULL);
    }

    NRF_LOG_ERROR("Assertion fault: !(%s) line: %i file: %s ",
                  NRF_LOG_PUSH((char *) condition),
                  line,
                  NRF_LOG_PUSH((char *) file));

    assert_info_t assert_info =
    {
        .line_num    = line,
        .p_file_name = (uint8_t *) file
    };
    app_error_fault_handler(NRF_FAULT_ID_SDK_ASSERT, 0, (uint32_t)(&assert_info));

    UNUSED_VARIABLE(assert_info);
}

void sys_assert_info_handler(const char * condition,
                             const int    line,
                             const char * file,
                             const char * info_fmt,
                             ...)
{
    int     err_code                   = 0;
    char    msg_buf[DEBUG_BUFFER_SIZE] = "";
    va_list args                       = {0};

    if (info_fmt == NULL)
    {
        APP_ERROR_HANDLER(NRF_ERROR_NULL);
    }

    va_start(args, info_fmt);
    err_code = vsnprintf(msg_buf, sizeof(msg_buf), info_fmt, args);
    if ((err_code < 0) || (err_code >= (int) sizeof(msg_buf)))
    {
        APP_ERROR_HANDLER(NRF_ERROR_INTERNAL);
    }
    va_end(args);

    NRF_LOG_INFO("Assertion info: %s", NRF_LOG_PUSH(msg_buf));
    sys_assert_handler(condition, line, file);
}
