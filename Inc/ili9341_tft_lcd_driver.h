/**@file
 * @brief	ILI9341 240RGBx320 resolution and 262k color 3.2" TFT LCD's driver Header file.
 *
 * @defgroup ili9341 ILI9341 240RGBx320 resolution and 262k color 3.2" TFT LCD Driver module
 * @{
 *
 * @brief   This module provides the functions, definitions, structures and variables that together work as the driver
 *          for the ILI9341 240RGBx320 resolution and 262k color 3.2" TFT LCD with the purpose of being used by the
 * 			application.
 *
 * @note    Another thing to highlight is that this @ref ili9341 has included the "stm32f1xx_hal.h" header file
 *          to be able to use the SPI in this module. However, this header file is specifically meant for the STM32F1
 *          series devices. If yours is from a different type, then you will have to substitute the right one here for
 *          your particular STMicroelectronics device. However, if you cant figure out what the name of that header file
 *          is, then simply substitute that line of code from this @ref ili9341 by: #include "main.h"
 *
 * @details <b><u>Code Example for using the @ref ili9341:</u></b>
 *
 * @code
  #include <stdio.h>	// Library from which "printf" is located at.
  #include <stdint.h>   // This library contains the aliases: uint8_t, uint16_t, uint32_t, etc.
  #include "ili9341_tft_lcd_driver.h" // This custom Mortrack's library contains the functions, definitions and variables that together operate as the driver for the ILI9341 Device.

  // ################################################ //
  // ##### INITIALIZATION OF THE ILI9341 MODULE ##### //
  // ################################################ //
  
  printf("The ILI9341 Driver Library validation test has successfully concluded!\r\n");


  while(1); // Stop the program here.
 * @endcode
 *
 * @author 	Cesar Miranda Meza (cmirandameza3@hotmail.com)
 * @date	MONTH DAY, YEAR.
 */

#ifndef ILI9341_TFT_LCD_DRIVER_H_
#define ILI9341_TFT_LCD_DRIVER_H_

#include "stm32f1xx_hal.h" // This is the HAL Driver Library for the STM32F1 series devices. If yours is from a different type, then you will have to substitute the right one here for your particular STMicroelectronics device. However, if you cant figure out what the name of that header file is, then simply substitute this line of code by: #include "main.h"
#include <stdint.h> // This library contains the aliases: uint8_t, uint16_t, uint32_t, etc.

/**@brief	MLX90614 TFT LCD Driver Exception codes.
 *
 * @details	These Exception Codes are returned by the functions of the @ref ili9341 to indicate the resulting
 *          status of having executed the process contained in each of those functions. For example, to indicate that
 *          the process executed by a certain function was successful or that it has failed.
 */
typedef enum
{
    ILI9341_EC_OK      = 0U,	//!< MLX90614 TFT LCD Driver Process was successful.
    ILI9341_EC_STOP    = 1U,    //!< MLX90614 TFT LCD Driver Process has been stopped.
    ILI9341_EC_NR      = 2U,	//!< MLX90614 TFT LCD Driver Process has concluded with no response.
    ILI9341_EC_NA      = 3U,    //!< MLX90614 TFT LCD Driver Data received or to be received Not Applicable.
    ILI9341_EC_ERR     = 4U     //!< MLX90614 TFT LCD Driver Process has failed.
} ILI9341_Status;

ILI9341_Status init_ili9341_module(void);

#endif /* ILI9341_TFT_LCD_DRIVER_H_ */

/** @} */

