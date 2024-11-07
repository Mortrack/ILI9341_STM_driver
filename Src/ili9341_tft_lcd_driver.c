/** @addtogroup ili9341
 * @{
 */

#include "ili9341_tft_lcd_driver.h"

#define ILI9341_SOFTWARE_RESET_COMMAND  (0x01)    /**< @brief Byte value that the ILI9341 interprets as the Software Reset Command. */
#define ILI9341_COMMAND_SIZE            (1)       /**< @brief Size in bytes that a single ILI9341 Command has. */

static SPI_HandleTypeDef *p_hspi;                           /**< @brief Pointer to the SPI Handle Structure of the DMA-SPI that will be used in this @ref ili9341 to write/read data to/from the ILI9341 3.2" TFT LCD Module. @details This pointer's value is defined in the @ref init_ili9341_module function. */
static ILI9341_peripherals_def_t *p_ili9341_peripherals;    /**< @brief Pointer to the ILI9341 3.2" TFT LCD Device's Peripherals Definition Structure that will be used in this @ref ili9341 to control the Peripherals towards which the terminals of the ILI9341 device are connected to. @details This pointer's value is defined in the @ref init_ili9341_module function. */

/**@brief	Sets the State of the CS pin of the ILI9341 3.2" TFT LCD Device to Reset (i.e., To Low State) so that our
 *          MCU/MPU enables SPI communication with it.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    November 05, 2024.
 */
static void enable_cs_pin(void);

/**@brief	Sets the State of the CS pin of the ILI9341 3.2" TFT LCD Device to Set (i.e., To High State) so that our
 *          MCU/MPU disables SPI communication with it.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    November 05, 2024.
 */
static void disable_cs_pin(void);

/**@brief	Applies a hardware reset in the ILI9341 3.2" TFT LCD Device.
 *
 * @note    It is necessary to wait 120msec after executing this function before sending an ILI9341 "Sleep Out" Command,
 *          according to the ILI6341 Datasheet.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    November 05, 2024.
 */
static void ili9341_hardware_reset(void);

/**@brief   Requests and applies a Software Reset to the ILI9341 TFT LCD Device.
 *
 * @note   A 5ms delay is applied at the end of this function.
 *
 * @retval  ILI9341_EC_OK if the Software request was requested successfully to the ILI9341 TFT LCD Device.
 * @retval  ILI9341_EC_NR if there was no SPI response after sending the Software Reset requested to the ILI9341 TFT LCD Device.
 * @retval  ILI9341_EC_ERR or other @ref ILI9341_Status Exception codes if something else went wrong with the SPI.
  */
static ILI9341_Status ili9341_software_reset(void);

/**@brief	Signals to the ILI9341 3.2" TFT LCD Device that the incoming SPI data will stand for an ILI9341 Data Type
 *          value.
 *
 * @note    This is achieved by setting and maintaining the D/C ILI9341 pin to a High state.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    November 05, 2024.
 */
static void set_dc_pin_to_data_mode(void);

/**@brief	Signals to the ILI9341 3.2" TFT LCD Device that the incoming SPI data will stand for an ILI9341 Command Type
 *          value.
 *
 * @note    This is achieved by setting and maintaining the D/C ILI9341 pin to a Low state.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    November 05, 2024.
 */
static void set_dc_pin_to_command_mode(void);

// TODO: Pending to determine if the size argument of the following function really requires to be 2 bytes in size.
/**@brief	Sends a desired data to the ILI9341 Device over the designated DMA-SPI that this module has been configured
 *          with.
 *
 * @note    <b style="color:red">WARNING:</b> In case there is still some data pending to be send via the DMA-SPI
 *          designated to this module, then this function will first halt until that data has been completely send. Only
 *          afterwards will it make the DMA-SPI request of sending the data requested whenever calling this function.
 *
 * @param[in] buffer    Pointer to the Memory Address containing the data that is desired to be sent to the ILI9341
 *                      Device.
 * @param size          Size in bytes to send to the ILI9341 Device.
 *
 * @retval              ILI9341_EC_OK if requesting to send the desired data over the DMA-SPI peripheral was successful.
 * @retval				ILI9341_EC_NR if there was no SPI response after sending the requested data over the SPI peripheral.
 * @retval				ILI9341_EC_ERR or other @ref ILI9341_Status Exception codes if something else went wrong with the SPI.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    November 05, 2024.
 */
static ILI9341_Status ili9341_dma_spi_tx(uint8_t *buffer, uint16_t size);

/**@brief	Gets the corresponding @ref ILI9341_Status value depending on the given @ref HAL_StatusTypeDef value.
 *
 * @param HAL_status	HAL Status value (see @ref HAL_StatusTypeDef ) that wants to be converted into its equivalent
 * 						of a @ref ILI9341_Status value.
 *
 * @retval				ILI9341_EC_NR if \p HAL_status param equals \c HAL_BUSY or \c HAL_TIMEOUT .
 * @retval				ILI9341_EC_ERR if \p HAL_status param equals \c HAL_ERROR .
 * @retval				HAL_status param otherwise.
 *
 * @note	For more details on the returned values listed, see @ref ILI9341_Status and @ref HAL_StatusTypeDef .
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    November 05, 2024.
 */
static ILI9341_Status HAL_ret_handler(HAL_StatusTypeDef HAL_status);

ILI9341_Status init_ili9341_module(SPI_HandleTypeDef *hspi, ILI9341_peripherals_def_t *peripherals)
{
    // TODO: Determine if the following ret variable is required for this function.
    /** <b>Local \c ILI9341_Status variable ret:</b> Holds the Return value of a @ref ILI9341_Status function type. */
    //ILI9341_Status ret;

    /* Persist the pointer to the specific SPI that is desired for the ILI9341 3.2" TFT LCD module to use. */
    p_hspi = hspi;

    /* Persist the pointer to the ILI9341 3.2" TFT LCD Device's Peripherals Definition Structure. */
    p_ili9341_peripherals = peripherals;

    /* Apply a Hardware Reset in the ILI9341 3.2" TFT LCD Device. */
    disable_cs_pin(); // Make sure that the CS pin is disabled before starting the init process of the ILI9341 device.
    ili9341_hardware_reset();

    /* Apply a Software Reset in the ILI9341 3.2" TFT LCD Device. */
    if (ili9341_software_reset() != ILI9341_EC_OK)
    {
        return ILI9341_EC_ERR;
    }

    /* Eee */
    // TODO: I left evaluating if the C0h ILI9341 Command has to







    return ILI9341_EC_OK;
}

static void ili9341_hardware_reset(void)
{
    /* Make sure that the Reset pin is in high state before starting to apply an ILI9341 hardware reset. */
    HAL_GPIO_WritePin(p_ili9341_peripherals->RESET.GPIO_Port, p_ili9341_peripherals->RESET.GPIO_Pin, GPIO_PIN_SET);
    HAL_Delay(1);

    /* Apply an ILI9341 hardware reset. */
    HAL_GPIO_WritePin(p_ili9341_peripherals->RESET.GPIO_Port, p_ili9341_peripherals->RESET.GPIO_Pin, GPIO_PIN_RESET);
    HAL_Delay(1); // Datasheet states that anytime longer than 10us will be taken as a Hardware Reset.

    /* Release Reset pin. */
    HAL_GPIO_WritePin(p_ili9341_peripherals->RESET.GPIO_Port, p_ili9341_peripherals->RESET.GPIO_Pin, GPIO_PIN_SET);
    HAL_Delay(5); // Datasheet states to wait 5ms after releasing ILI9341 RESET pin before sending commands.
}

static ILI9341_Status ili9341_software_reset(void)
{
    /** <b>Local \c ILI9341_Status variable ret:</b> Holds the Return value of a @ref ILI9341_Status function type. */
    ILI9341_Status ret;
    /** <b>Local \c uint8_t variable ili9341_command:</b> Holds the ILI9341 Command that will be sent to it via the SPI-DMA peripheral. */
    uint8_t ili9341_command = ILI9341_SOFTWARE_RESET_COMMAND;

    set_dc_pin_to_command_mode();
    enable_cs_pin();
    ret = ili9341_dma_spi_tx(&ili9341_command, ILI9341_COMMAND_SIZE);
    disable_cs_pin();
    HAL_Delay(5); // Datasheet states to wait 5ms after sending an ILI9341 Software Reset Command.

    return ret;
}

static void enable_cs_pin(void)
{
    HAL_GPIO_WritePin(p_ili9341_peripherals->CS.GPIO_Port, p_ili9341_peripherals->CS.GPIO_Pin, GPIO_PIN_RESET);
}

static void disable_cs_pin(void)
{
    HAL_GPIO_WritePin(p_ili9341_peripherals->CS.GPIO_Port, p_ili9341_peripherals->CS.GPIO_Pin, GPIO_PIN_SET);
}

static void set_dc_pin_to_data_mode(void)
{
    HAL_GPIO_WritePin(p_ili9341_peripherals->DC.GPIO_Port, p_ili9341_peripherals->DC.GPIO_Pin, GPIO_PIN_SET);
}

static void set_dc_pin_to_command_mode(void)
{
    HAL_GPIO_WritePin(p_ili9341_peripherals->DC.GPIO_Port, p_ili9341_peripherals->DC.GPIO_Pin, GPIO_PIN_RESET);
}

static ILI9341_Status ili9341_dma_spi_tx(uint8_t *buffer, uint16_t size)
{
    while(!__HAL_SPI_GET_FLAG(HSPI_INSTANCE, SPI_FLAG_TXE)); // Wait if there is still an ongoing DMA-SPI transaction giving place.
    return HAL_ret_handler(HAL_SPI_Transmit_DMA(p_hspi, buffer, size));
}

static ILI9341_Status HAL_ret_handler(HAL_StatusTypeDef HAL_status)
{
    switch (HAL_status)
    {
        case HAL_BUSY:
        case HAL_TIMEOUT:
            return ILI9341_EC_NR;
        case HAL_ERROR:
            return ILI9341_EC_ERR;
        default:
            return (ILI9341_Status) HAL_status;
    }
}

/** @} */

