///
/// \file 		Shell.c
/// \brief 		[Source file]
///				
/// \author 	NOVO
///
////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <Shell.h>
#include "main.h"
#include "usart.h"
#include "stdio.h"
////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Private Variables
////////////////////////////////////////////////////////////////////////////////
uint8_t rxData[SHELL_RX_BUFFER_SIZE];
uint16_t echoManagement = 0;
////////////////////////////////////////////////////////////////////////////////
// Private functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// \brief    PUTCHAR_PROTOTYPE()
/// \details   Retargets the C library printf function to the UART2.
/// \public
/// \return   Character to put in IO printf
////////////////////////////////////////////////////////////////////////////////
#ifdef __GNUC__
// With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
// set to 'Yes') calls __io_putchar()
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif // __GNUC__
#if !IAR_SIMULATION
PUTCHAR_PROTOTYPE
{
    //Write a character to the framework UART handle and Loop until the end of transmission
    HAL_UART_Transmit(&DEBUG_PORT, (uint8_t *)&ch, 1, 0xFFFF);

    return ch;
}
#endif

/**
 * @brief Initialize the Shell Com Port
 * @details Setup a UART to communicate as a shell
 */
bool init_ShellComPort (void)
{
	bool response = false;
	HAL_UART_Read_DMA(&DEBUG_PORT, rxData, SHELL_RX_BUFFER_SIZE, &response);
    return true;
}

/**
 * @brief Check if data is ready on the shell
 */
bool isDataReady_ShellComPort (bool * isReady)
{
    char * shellChararacters;
    if (isReady == NULL) return false;
    
    *isReady = false;
    shellChararacters = strpbrk((char const *)rxData, "\r\n");
    if (shellChararacters != NULL)
    {
        // Replace \r for 0
        *shellChararacters = 0;
        
        *isReady = true;
    }
    return true;
}

/**
 * @brief Manage the echo on the shell port
 */
bool manageEcho_ShellComPort (void)
{
    uint32_t count = 0;
    if (HAL_UART_Get_RX_Count_DMA(&DEBUG_PORT, &count) == false) return false;
    
    count = SHELL_RX_BUFFER_SIZE - count;
    
    if (count > echoManagement)
    {
        printf("%s", &rxData[count - 1]);
    }
    echoManagement = count;
    
    return true;
}

/**
 * @brief Get the shell buffer handle
 */
bool getBufferHandle_ShellComPort (uint8_t ** handle)
{
    if (handle == NULL) return false;
    *handle = rxData;
    return true;
}

/**
 * @brief Clear the receive buffer
 */
bool clearRXBuffer_ShellComPort (void)
{
	bool response = false;
    echoManagement = 0;
    memset(rxData, 0, SHELL_RX_BUFFER_SIZE);
    HAL_UART_AbortReceive(&DEBUG_PORT);
    HAL_UART_Read_DMA(&DEBUG_PORT, rxData, SHELL_RX_BUFFER_SIZE, &response);
    return true;
}
