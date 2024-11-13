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

/**@brief	ILI9341 TFT LCD driver Exception Codes.
 *
 * @details	These Exception Codes are returned by the functions of the @ref ili9341 to indicate the resulting
 *          status of having executed the process contained in each of those functions. For example, to indicate that
 *          the process executed by a certain function was successful or that it has failed.
 */
typedef enum
{
    ILI9341_EC_OK      = 0U,	//!< ILI9341 TFT LCD Driver Process was successful.
    ILI9341_EC_STOP    = 1U,    //!< ILI9341 TFT LCD Driver Process has been stopped.
    ILI9341_EC_NR      = 2U,	//!< ILI9341 TFT LCD Driver Process has concluded with no response.
    ILI9341_EC_NA      = 3U,    //!< ILI9341 TFT LCD Driver Data received or to be received Not Applicable.
    ILI9341_EC_ERR     = 4U     //!< ILI9341 TFT LCD Driver Process has failed.
} ILI9341_Status;

/**@brief	ILI9341 Bits Per Pixel types definitions.
 *
 * @note    These definitions are defined with respect to the Bits Per Pixel types that can be managed by the ILI9341
 *          TFT LCD Device.
 */
typedef enum
{
    ILI9341_BPP_16 = 0,    //!< ILI9341 16 Bits Per Pixel.
    ILI9341_BPP_18 = 1     //!< ILI9341 18 Bits Per Pixel.
} ILI9341_BPP_t;

/**@brief	ILI9341 TFT LCD driver Bit Color Order.
 *
 * @details	These Bit Color Orders are used in some functions of the @ref ili9341 to hold the corresponding RGB colors
 *          for both the 16 bit and 18 bit per pixel color order as managed in the ILI9341 TFT LCD Device according to
 *          its datasheet.
 */
union ILI9341_COLOR
{
    uint32_t 18_bpp;    //!< ILI9341 18 bit per pixel color order (i.e., Red = 6 bit, Green = 6 bit and Blue = 6 bit; or 262'144 colors), where the bits for each color should be arranged in the following manner:<br>- Bits 0 and 1 = Don't care.<br>- Bits 2 up to 7 = Color Blue.<br>- Bits 8 and 9 = Don't care.<br>- Bits 10 up to 15 = Color Green.<br>- Bits 16 and 17 = Don't care.<br>- Bits 18 up to 23 = Color Red.
    uint16_t 16_bpp;    //!< ILI9341 16 bit per pixel color order (i.e., Red = 5 bit, Green = 6 bit and Blue = 5 bit; or 65'536 colors), where the bits for each color should be arranged in the following manner:<br>- Bits 0 up to 4 = Color Blue.<br>- Bits 5 up to 10 = Color Green.<br>- Bits 11 up to 15 = Color Red.
};

/**@brief	ILI9341 3.2" TFT LCD Driver GPIO Definition parameters structure.
 *
 * @details This contains all the fields required to associate a certain GPIO pin to the Chip Select pin (i.e., The CS
 *          pin) of the ILI9341 Device Hardware.
 */
typedef struct __attribute__ ((__packed__))
{
    GPIO_TypeDef *GPIO_Port;	//!< Type Definition of the GPIO peripheral port to which this @ref ILI9341_GPIO_def_t structure will be associated with.
    uint16_t GPIO_Pin;			//!< Pin number of the GPIO peripheral from to this @ref ILI9341_GPIO_def_t structure will be associated with.
} ILI9341_GPIO_def_t;

/**@brief	ILI9341 3.2" TFT LCD Device's Peripherals Definition parameters structure.
 *
 * @details This contains all the fields required to associate the corresponding peripheral pins of our MCU towards
 *          which the terminals of the ILI9341 Device are connected to.
 */
typedef struct __attribute__ ((__packed__))
{
    ILI9341_GPIO_def_t CS;	     //!< Type Definition of the GPIO peripheral port to which the CS terminal of the ILI9341 device is connected to.
    ILI9341_GPIO_def_t RESET;    //!< Type Definition of the GPIO peripheral port to which the RESET terminal of the ILI9341 device is connected to.
    ILI9341_GPIO_def_t DC;       //!< Type Definition of the GPIO peripheral port to which the D/C terminal of the ILI9341 device is connected to.
} ILI9341_peripherals_def_t;

/**@brief   Initializes the @ref ili9341 and its designated ILI9341 3.2" TFT LCD Device.
 *
 * @details This function will first update several of its Global Static Pointers and Variables that are used in the
 *          @ref ili9341 .
 *
 * @note    This Global Static Pointers and variables that will be updated, initialized and/or persisted in the
 *          @ref ili9341 by this function are the following:<br>
 *          - The @ref p_hspi Global Static Pointer to point to the address towards which the \p hspi param points to.
 *          - The @ref p_ili9341_peripherals Pointer to point to the address, which should contain the required data of the GPIO peripherals towards which the 3.2" TFT LCD Device is connected to with respect to our MCU/MPU.
 * @note    <b>This function must be called only once</b> before calling any other function of the @ref ili9341 .
 *
 * @details Subsequently, this function will configure the ILI3.2" TFT LCD Device by following the next steps in that
 *          orderly fashion:
 *          1. Apply an ILI9341 Hardware Reset.
 *          2. Apply an ILI9341 Software Reset.
 *          3. Set the ILI9341 Power Control 1 to have its GVDD Level to 4.6V.
 *          4. Set the ILI9341 VCOM Control 1 to have its VCOMH and VCOML voltages set to 4.25V and -1.5V respectively.
 *          5. Set the ILI9341 VCOM Control 2 so t hat the VMH and VML have an offset of -58 and -58 respectively.
 *          6. Configure the Memory Access Control.
 *          7. Configure the Pixel Format to 16 bit per pixel (i.e., 65k color mode).
 *          8. Configure the ILI9341 Display Function Control with all its default values and changing only the source/ VCOM's "Source output on non-display area" from AGND and AGND to V63 and V0 respectively and its "VCOM output on non-display area" from AGND and AGND to VCOML and VCOMH respectively.
 *          9. Exit ILI9341 from Sleep Mode.
 *          10. Turn On the ILI9341 Display.
 *
 * @param[in] hspi          Pointer to the SPI is desired for the @ref ili9341 to use for exchanging information with
 *                          the ILI9341 3.2" TFT LCD via the SPI Communication Protocol.
 * @param[in] peripherals   Pointer to the ILI9341 3.2" TFT LCD Device's Peripherals Definition parameters structure
 *                          that should contain the required data of the Pin Peripherals at which the ILI9341 TFT LCD
 *                          Device is expected to be connected at with respect to our MCU/MPU.
 *
 * @retval  MLX90614_EC_OK  If the @ref ili9341 has been successfully initialized.
 * @retval  MLX90614_EC_NR  If either the ILI9341 3.2" TFT LCD Device wired to our MPU/MCU is not ready for SPI
 *                          Communication (if this happens check that the wiring is correct, that your ILI9341 Device is
 *                          functioning correctly and that your MCU/MPU SPI peripheral has been correctly configured),
 *                          or it the ILI9341 Device stops responding via the SPI Protocol during the initialization
 *                          process of this function.
 * @retval  MLX90614_EC_ERR The @ref ili9341 was not initialized due to that something went wrong either with the
 *                          ILI9341 Device or with the SPI Communication established between that device and our
 *                          MCU/MPU.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    November 12, 2024.
 */
ILI9341_Status init_ili9341_module(SPI_HandleTypeDef *hspi, ILI9341_peripherals_def_t *peripherals);

#endif /* ILI9341_TFT_LCD_DRIVER_H_ */

/** @} */

