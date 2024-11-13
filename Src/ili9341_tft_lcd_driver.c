/** @addtogroup ili9341
 * @{
 */

#include "ili9341_tft_lcd_driver.h"

#define ILI9341_SOFTWARE_RESET_COMMAND                      (0x01)    /**< @brief Byte value that the ILI9341 interprets as the Software Reset Command. */
#define ILI9341_POWER_CONTROL_1_COMMAND                     (0xC0)    /**< @brief Byte value that the ILI9341 interprets as the Power Control 1 Command. */
#define ILI9341_VCOM_CONTROL_1_COMMAND                      (0xC5)    /**< @brief Byte value that the ILI9341 interprets as the VCOM Control 1 Command. */
#define ILI9341_VCOM_CONTROL_2_COMMAND                      (0xC7)    /**< @brief Byte value that the ILI9341 interprets as the VCOM Control 2 Command. */
#define ILI9341_MEMORY_ACCESS_CONTROL_COMMAND               (0x36)    /**< @brief Byte value that the ILI9341 interprets as the Memory Access Control Command. */
#define ILI9341_PIXEL_FORMAT_COMMAND                        (0x3A)    /**< @brief Byte value that the ILI9341 interprets as the Pixel Format Command. */
#define ILI9341_DISPLAY_FUNCTION_CONTROL_COMMAND            (0xB6)    /**< @brief Byte value that the ILI9341 interprets as the Display Function Control Command. */
#define ILI9341_SLEEP_OUT_COMMAND                           (0x11)    /**< @brief Byte value that the ILI9341 interprets as the Sleep Out Command. */
#define ILI9341_DISPLAY_ON_COMMAND                          (0x29)    /**< @brief Byte value that the ILI9341 interprets as the Display ON Command. */
#define ILI9341_COMMAND_SIZE                                (1)       /**< @brief Size in bytes that a single ILI9341 Command has. */
#define ILI9341_SINGLE_DATA_SIZE                            (1)       /**< @brief Size in bytes that a single ILI9341 Data has. */
#define ILI9341_VCOM_CONTROL_1_DATA_SIZE                    (2)       /**< @brief Size in bytes of the ILI9341 Device's VCOM Control 1 command. */
#define ILI9341_DISPLAY_FUNCTION_CONTROL_DATA_SIZE          (2)       /**< @brief Size in bytes of the ILI9341 Device's Display Function Control command. */

static SPI_HandleTypeDef *p_hspi;                                       /**< @brief Pointer to the SPI Handle Structure of the DMA-SPI that will be used in this @ref ili9341 to write/read data to/from the ILI9341 3.2" TFT LCD Module. @details This pointer's value is defined in the @ref init_ili9341_module function. */
static ILI9341_peripherals_def_t *p_ili9341_peripherals;                /**< @brief Pointer to the ILI9341 3.2" TFT LCD Device's Peripherals Definition Structure that will be used in this @ref ili9341 to control the Peripherals towards which the terminals of the ILI9341 device are connected to. @details This pointer's value is defined in the @ref init_ili9341_module function. */
static ILI9341_BPP_t ili9341_bpp_type;                                  /**< @brief ILI9341 Bits Per Pixel (BPP) Type with which the @ref ili9341 will be currently responding whenever processing ILI9341 RGB pixel colors. */
static ILI9341_Status (*p_ili9341_fill_screen)(ILI9341_COLOR color);    /**< @brief Pointer to the function that fills the screen with a single/plain color with the right Bits Per Pixel (BPP) Color Order. */

/**@brief	ILI9341 3.2" TFT LCD Device's GVDD Level values types definitions.
 *
 * @note    These definitions are defined with respect to the GVDD values types that can be managed by this
 *          @ref ili9341 .
 * @note    According to the datasheet, GVDD must be equal or lower than AVDD - 0.5.
 * @note    The author of @ref ili9341 believes that it is very likely that some voltage regulators and/or voltage
 *          step-up converters might be acting in the ILI9341 Board, after having inspected it in person, but that could
 *          not be 100% guaranteed by the author of @ref ili9341 since no datasheet for this actual board could be found.
 *          As a consequence, in case this is true, this should limit the available options of GVDD. In addition to
 *          this, the <a href=https://github.com/eziya/STM32_HAL_ILI9341>driver library from eziya</a> that was used as
 *          a base to start understanding how to program the ILI9341 Board, had configured the GVDD level to 4.6V, but
 *          unfortunately with no explanation. As a result of these aspects and due to the lack of documentation for the
 *          ILI9341 board, the author of @ref ili9341 decided to make available the GVDD level of 4.6V only since with
 *          it, the ILI9341 board seems to work fine with that value.
 */
typedef enum
{
    ILI9341_GVDD_4V6 = 0x23    //!< ILI9341 TFT LCD Device's GVDD Level set to 4.6V.
} ILI9341_GVDD_t;

/**@brief	ILI9341 3.2" TFT LCD Device's VCOMH Voltage values types definitions.
 *
 * @note    These definitions are defined with respect to the VCOMH values types that can be managed by this
 *          @ref ili9341 .
 * @note    The author of @ref ili9341 believes that it is very likely that some voltage regulators and/or voltage
 *          step-up converters might be acting in the ILI9341 Board, after having inspected it in person, but that could
 *          not be 100% guaranteed by the author of @ref ili9341 since no datasheet for this actual board could be found.
 *          As a consequence, in case this is true, this should limit the available options of VCOMH. In addition to
 *          this, the <a href=https://github.com/eziya/STM32_HAL_ILI9341>driver library from eziya</a> that was used as
 *          a base to start understanding how to program the ILI9341 Board, had configured the VCOMH level to 4.25V, but
 *          unfortunately with no explanation. As a result of these aspects and due to the lack of documentation for the
 *          ILI9341 board, the author of @ref ili9341 decided to make available the VCOMH level of 4.25V only since with
 *          it, the ILI9341 board seems to work fine with that value.
 */
typedef enum
{
    ILI9341_VCOMH_4V25 = 0x3E    //!< ILI9341 TFT LCD Device's VCOMH Voltage set to 4.25V.
} ILI9341_VCOMH_t;

/**@brief	ILI9341 3.2" TFT LCD Device's VCOML Voltage values types definitions.
 *
 * @note    These definitions are defined with respect to the VCOML values types that can be managed by this
 *          @ref ili9341 .
 * @note    The author of @ref ili9341 believes that it is very likely that some voltage regulators and/or voltage
 *          step-up converters might be acting in the ILI9341 Board, after having inspected it in person, but that could
 *          not be 100% guaranteed by the author of @ref ili9341 since no datasheet for this actual board could be found.
 *          As a consequence, in case this is true, this should limit the available options of VCOML. In addition to
 *          this, the <a href=https://github.com/eziya/STM32_HAL_ILI9341>driver library from eziya</a> that was used as
 *          a base to start understanding how to program the ILI9341 Board, had configured the VCOML level to -1.5V, but
 *          unfortunately with no explanation. As a result of these aspects and due to the lack of documentation for the
 *          ILI9341 board, the author of @ref ili9341 decided to make available the VCOML level of -1.5V only since with
 *          it, the ILI9341 board seems to work fine with that value.
 */
typedef enum
{
    ILI9341_VCOML_minus_1V5 = 0x28    //!< ILI9341 TFT LCD Device's VCOML Voltage set to -1.5V.
} ILI9341_VCOML_t;

/**@brief	ILI9341 3.2" TFT LCD Device's VMF Offset values types definitions.
 *
 * @note    These definitions are defined with respect to the VMF values types that can be managed by this @ref ili9341 .
 * @note    The author of @ref ili9341 believes that it is very likely that some voltage regulators and/or voltage
 *          step-up converters might be acting in the ILI9341 Board, after having inspected it in person, but that could
 *          not be 100% guaranteed by the author of @ref ili9341 since no datasheet for this actual board could be found.
 *          As a consequence, in case this is true, this should limit the available options of VMF. In addition to
 *          this, the <a href=https://github.com/eziya/STM32_HAL_ILI9341>driver library from eziya</a> that was used as
 *          a base to start understanding how to program the ILI9341 Board, had configured the VMF level to -58, but
 *          unfortunately with no explanation. As a result of these aspects and due to the lack of documentation for the
 *          ILI9341 board, the author of @ref ili9341 decided to make available the VMF offset of -58 only since with
 *          it, the ILI9341 board seems to work fine with that value.
 */
typedef enum
{
    ILI9341_VMF_minus_58 = 0x86    //!< ILI9341 TFT LCD Device's VMF offset is set to -58 and -58 for VCOMH and VCOML respectively.
} ILI9341_VMF_t;

/**@brief	ILI9341 3.2" TFT LCD Driver Memory Access Control Data parameters structure.
 *
 * @details This contains all the fields required to associate all the corresponding fields that should be contained
 *          inside a given Memory Access Control Data value of a ILI9341 Device.
 */
typedef struct __attribute__ ((__packed__))
{
    uint8_t d0_and_d1:2;                                        //!< These two bits should always be set to zero.
    uint8_t mh:1;                                               //!< MH: Horizontal Refresh Order, which is used for LCD horizontal refreshing direction control and has the following possible values:<br>* 0 = Refreshes LCD from left to right.<br>* 1 = Refreshes LCD from right to left.
    uint8_t bgr:1;                                              //!< BGR: RGB-BGR Order, which is used as a color selector switch control, where the available options are the following:<br>* 0 = RGB color filter panel.<br>* 1 = BGR color filter panel.
    uint8_t ml:1;                                               //!< ML: Vertical Refresh Order, which is used for LCD vertical refresh direction control and has the following possible values:<br>* 0 = Refreshes LCD from top to bottom.<br>* 1 = Refreshes LCD from bottom to top.
    uint8_t wr_rd_dir:3;										//!< These 3 bits control MCU to memory write/read direction. This variable should contain the fields of @ref ILI9341_MADCTL_MCU_WRITE_READ_DIRECTION_def_t .
} ILI9341_MADCTL_def_t;

/**@brief	ILI9341 3.2" TFT LCD Driver MCU Control to Memory Write/Read Direction Data parameters structure.
 *
 * @details This contains all the fields required to associate all the corresponding fields that should be contained
 *          inside a given MCU Control to Memory Write/Read Direction Data value of a @ref ILI9341_MADCTL_def_t
 *          structure.
 */
typedef struct __attribute__ ((__packed__))
{
    uint8_t mv:1;          //!< MV: Row/Column Exchange, where the possible values are the following:<br>* 0 = Memory is displayed in LCD in the exact way that the data is stored in the ILI9341 Memory.<br>* 1 = New or subsequent data in memory is overwritten at the beginning of the LCD (i.e., from left to right).
    uint8_t mx:1;          //!< MX: Column Address Order. @note Unfortunately, the ILI9341 datasheet states no more about this bit and the effects of its possible values, but basing on the other parameters describes for the Memory Access Control, my personal opinion and intuition says that the possible values are as follows:<br>* 0 = Column address order is from left to right.<br>* 1 = Column address order is from right to left.
    uint8_t my:1;          //!< MY: Row Address Order. @note Unfortunately, the ILI9341 datasheet states no more about this bit and the effects of its possible values, but basing on the other parameters describes for the Memory Access Control, my personal opinion and intuition says that the possible values are as follows:<br>* 0 = Row address order is from top to bottom.<br>* 1 = Row address order is from bottom to top.
    uint8_t reserved:5;    //!< Reserved bits.
} ILI9341_MADCTL_MCU_WRITE_READ_DIRECTION_def_t;

/**@brief	ILI9341 3.2" TFT LCD Driver Pixel Format Data parameters structure.
 *
 * @details This contains all the fields required to associate all the corresponding fields that should be contained
 *          inside a given Pixel Format Data value of a ILI9341 Device.
 */
typedef struct __attribute__ ((__packed__))
{
    uint8_t dbi:3;    //!< DBI is the pixel format of MCU interface, where the possible values are:<br>* 0x05 = 16 bits/pixel.<br>* 0x06 = 18 bits/pixel. @note For more details, please refer to the ILI9341 datasheet.
    uint8_t d3:1;     //!< This bit should always be set to zero.
    uint8_t dpi:3;    //!< DPI is the pixel format select of RGB interface, where the possible values are:<br>* 0x05 = 16 bits/pixel.<br>* 0x06 = 18 bits/pixel. @note For more details, please refer to the ILI9341 datasheet.
    uint8_t d7:1;     //!< This bit should always be set to zero.
} ILI9341_PIXEL_FORMAT_def_t;

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
 * @note    This function will call a total of 7msec delay during its executing for both guaranteeing that this function
 *          is executed properly and so that subsequent commands can be successfully send to the ILI9341 Device.
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
 * @retval  ILI9341_EC_OK if the Software Reset request was made successfully to the ILI9341 TFT LCD Device.
 * @retval  ILI9341_EC_NR if there was no SPI response after sending the Software Reset requested to the ILI9341 TFT LCD Device.
 * @retval  ILI9341_EC_ERR or other @ref ILI9341_Status Exception codes if something else went wrong with the SPI.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    November 05, 2024.
 */
static ILI9341_Status ili9341_software_reset(void);

/**@brief   Configures the ILI9341 Power Control to have a certain/desired value for the GVDD Level.
 *
 * @param gvdd_level    Data Value that the ILI9341 will interpret as a certain configuration for the GVDD Level.
 *
 * @retval  ILI9341_EC_OK if the Power Control 1 configuration request was made successfully to the ILI9341 TFT LCD Device.
 * @retval  ILI9341_EC_NR if there was no SPI response after sending the Power Control 1 requested to the ILI9341 TFT LCD Device.
 * @retval  ILI9341_EC_ERR or other @ref ILI9341_Status Exception codes if something else went wrong with the SPI.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    November 06, 2024.
 */
static ILI9341_Status ili9341_configure_power_control_1(ILI9341_GVDD_t gvdd_level);

/**@brief   Configures the ILI9341 VCOM Control 1 to have a certain/desired Voltage for both the VCOMH and VCOML.
 *
 * @param vcomh_voltage Data Value that the ILI9341 will interpret as a certain configuration for the VCOMH Voltage.
 * @param vcoml_voltage Data Value that the ILI9341 will interpret as a certain configuration for the VCOML Voltage.
 *
 * @retval  ILI9341_EC_OK if the VCOM Control 1 configuration request was made successfully to the ILI9341 TFT LCD Device.
 * @retval  ILI9341_EC_NR if there was no SPI response after sending the VCOM Control 1 requested to the ILI9341 TFT LCD Device.
 * @retval  ILI9341_EC_ERR or other @ref ILI9341_Status Exception codes if something else went wrong with the SPI.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    November 07, 2024.
 */
static ILI9341_Status ili9341_configure_vcom_control_1(ILI9341_VCOMH_t vcomh_voltage, ILI9341_VCOML_t vcoml_voltage);

/**@brief   Configures the ILI9341 VCOM Control 2 to have a certain/desired offset for both the VCOMH and VCOML of the
 *          ILI9341 Device.
 *
 * @param vmf_offset    Data Value that the ILI9341 will interpret as a certain configuration for the VMF Offset.
 *
 * @retval  ILI9341_EC_OK if the VCOM Control 2 configuration request was made successfully to the ILI9341 TFT LCD Device.
 * @retval  ILI9341_EC_NR if there was no SPI response after sending the VCOM Control 2 requested to the ILI9341 TFT LCD Device.
 * @retval  ILI9341_EC_ERR or other @ref ILI9341_Status Exception codes if something else went wrong with the SPI.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    November 07, 2024.
 */
static ILI9341_Status ili9341_configure_vcom_control_2(ILI9341_VMF_t vmf_offset);

/**@brief   Configures the ILI9341 Memory Access Control to define the desired read/write scanning of frame memory.
 *
 * @note    For more details on the parameters that are configured by the Memory Access Control command, please see
 *          @ref ILI9341_MADCTL_def_t .
 *
 * @retval  ILI9341_EC_OK if the Memory Access Control configuration request was made successfully to the ILI9341 TFT LCD Device.
 * @retval  ILI9341_EC_NR if there was no SPI response after sending the Memory Access Control requested to the ILI9341 TFT LCD Device.
 * @retval  ILI9341_EC_ERR or other @ref ILI9341_Status Exception codes if something else went wrong with the SPI.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    November 07, 2024.
 */
static ILI9341_Status ili9341_configure_memory_access_control(void);

/**@brief   Configures the ILI9341 Pixel Format to set the desired pixel format for the RGB image data used by the
 *          interface.
 *
 * @details This function has fixed setting the Pixel Format to 16 bits per pixel.
 *
 * @note    For more details on the parameters that are configured by the Pixel Format command, please see
 *          @ref ILI9341_PIXEL_FORMAT_def_t .
 *
 * @retval  ILI9341_EC_OK if the Pixel Format configuration request was made successfully to the ILI9341 TFT LCD Device.
 * @retval  ILI9341_EC_NR if there was no SPI response after sending the Pixel Format requested to the ILI9341 TFT LCD Device.
 * @retval  ILI9341_EC_ERR or other @ref ILI9341_Status Exception codes if something else went wrong with the SPI.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    November 07, 2024.
 */
static ILI9341_Status ili9341_configure_pixel_format(void);

/**@brief   Configures the ILI9341 Display Function Control with all its default values, but by only changing the
 *          source/VCOM's "Source output on non-display area" from AGND and AGND to V63 and V0 respectively and
 *          its "VCOM output on non-display area" from AGND and AGND to VCOML and VCOMH respectively.
 *
 * @note    For more details on the parameters that are configured by the Pixel Format command, please see the ILI9341
 *          datasheet.
 *
 * @retval  ILI9341_EC_OK if the Display Function Control configuration request was made successfully to the ILI9341 TFT LCD Device.
 * @retval  ILI9341_EC_NR if there was no SPI response after sending the Display Function Control requested to the ILI9341 TFT LCD Device.
 * @retval  ILI9341_EC_ERR or other @ref ILI9341_Status Exception codes if something else went wrong with the SPI.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    November 07, 2024.
 */
static ILI9341_Status ili9341_configure_display_function_control(void);

/**@brief   Sends requests to ILI9341 for exiting sleep mode.
 *
 * @note    This function calls a 5ms delay at the end since, as stated in the ILI9341 datasheet, it is the time
 *          required after the ILI9341 Device exits sleep mode so that another command can be send to it.
 * @note    It is necessary to wait 120msec after executing this function before sending an ILI9341 "Sleep Out" Command,
 *          according to the ILI6341 Datasheet.
 *
 * @retval  ILI9341_EC_OK if the Sleep Out request was made successfully to the ILI9341 TFT LCD Device.
 * @retval  ILI9341_EC_NR if there was no SPI response after sending the Sleep Out requested to the ILI9341 TFT LCD Device.
 * @retval  ILI9341_EC_ERR or other @ref ILI9341_Status Exception codes if something else went wrong with the SPI.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    November 07, 2024.
 */
static ILI9341_Status ili9341_exit_sleep_mode(void);

/**@brief   Sends requests to ILI9341 for turning its Display On.
 *
 * @note    Whenever the ILI9341 Display is turned Off, its Display will still emit light as usual, with a white
 *          background, and the only difference is that no images will be shown until the display is turned On.
 *
 * @retval  ILI9341_EC_OK if the Display ON request was made successfully to the ILI9341 TFT LCD Device.
 * @retval  ILI9341_EC_NR if there was no SPI response after sending the Display ON requested to the ILI9341 TFT LCD Device.
 * @retval  ILI9341_EC_ERR or other @ref ILI9341_Status Exception codes if something else went wrong with the SPI.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    November 07, 2024.
 */
static ILI9341_Status ili9341_turn_display_on(void);

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
    /** <b>Local \c ILI9341_Status variable ret:</b> Holds the Return value of a @ref ILI9341_Status function type. */
    ILI9341_Status ret;

    /* Persist the pointer to the specific SPI that is desired for the ILI9341 3.2" TFT LCD module to use. */
    p_hspi = hspi;

    /* Persist the pointer to the ILI9341 3.2" TFT LCD Device's Peripherals Definition Structure. */
    p_ili9341_peripherals = peripherals;

    /* Apply a Hardware Reset in the ILI9341 3.2" TFT LCD Device. */
    disable_cs_pin(); // Make sure that the CS pin is disabled before starting the init process of the ILI9341 device.
    ili9341_hardware_reset();

    /* Apply a Software Reset in the ILI9341 3.2" TFT LCD Device. */
    ret = ili9341_software_reset();
    if (ret != ILI9341_EC_OK)
    {
        return ret;
    }

    /* Configure the ILI9341 Power Control 1 to have its GVDD Level set to 4.6V. */
    ret = ili9341_configure_power_control_1(ILI9341_GVDD_4V6);
    if (ret != ILI9341_EC_OK)
    {
        return ret;
    }

    /* Configure the ILI9341 Power Control 2. */
    // Default ILI9341 configuration will be left here.

    /* Configure the VCOM Control 1 to have its VCOMH and VCOML voltages set to 4.25V and -1.5V respectively.  */
    ret = ili9341_configure_vcom_control_1(ILI9341_VCOMH_4V25, ILI9341_VCOML_minus_1V5);
    if (ret != ILI9341_EC_OK)
    {
        return ret;
    }

    /* Configure the VCOM Control 2 so that VMH and VML have an offset of -58 and -58 respectively. */
    ret = ili9341_configure_vcom_control_2(ILI9341_VMF_minus_58);
    if (ret != ILI9341_EC_OK)
    {
        return ret;
    }

    /* Configure the Memory Access Control. */
    ret = ili9341_configure_memory_access_control();
    if (ret != ILI9341_EC_OK)
    {
        return ret;
    }

    /* Configure the Pixel Format. */
    ret = ili9341_configure_pixel_format();
    if (ret != ILI9341_EC_OK)
    {
        return ret;
    }

    /* Configure the Frame Rate Control. */
    // Default ILI9341 configuration will be left here.

    /* Configure Read Display MADCTL. */
    // Default ILI9341 configuration will be left here (meaning that MADCTL's B5=0 or, i.e., that the maximum column and row in the frame memory where the ILI9341's MCU can access will be 240 and 320 respectively).

    /* Configure the Display Function Control. */
    ret = ili9341_configure_display_function_control();
    if (ret != ILI9341_EC_OK)
    {
        return ret;
    }

    /* Configure the desired Gamma Curve for the ILI9341 Display. */
    // Default ILI9341 configuration will be left here.

    /* Configure Positive Gamma Correction. */
    // Default ILI9341 configuration will be left here.

    /* Configure Negative Gamma Correction. */
    // Default ILI9341 configuration will be left here.

    /* Make ILI9341 exit Sleep Mode. */
    ret = ili9341_exit_sleep_mode();
    if (ret != ILI9341_EC_OK)
    {
        return ret;
    }

    /* Turn ILI9341 Display On. */
    ret = ili9341_turn_display_on();
    if (ret != ILI9341_EC_OK)
    {
        return ret;
    }

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

static ILI9341_Status ili9341_configure_power_control_1(ILI9341_GVDD_t gvdd_level)
{
    /** <b>Local \c ILI9341_Status variable ret:</b> Holds the Return value of a @ref ILI9341_Status function type. */
    ILI9341_Status ret;
    /** <b>Local \c uint8_t variable ili9341_command_or_data_value:</b> Holds either a Command or a Data that will be sent to the ILI9341 Device via the SPI-DMA peripheral. */
    uint8_t ili9341_command_or_data_value = ILI9341_POWER_CONTROL_1_COMMAND;

    set_dc_pin_to_command_mode();
    enable_cs_pin();
    ret = ili9341_dma_spi_tx(&ili9341_command_or_data_value, ILI9341_COMMAND_SIZE);
    if (ret != ILI9341_EC_OK)
    {
        disable_cs_pin();
        return ret;
    }

    set_dc_pin_to_data_mode();
    ili9341_command_or_data_value = (uint8_t) gvdd_level;
    ret = ili9341_dma_spi_tx(&ili9341_command_or_data_value, ILI9341_SINGLE_DATA_SIZE);
    disable_cs_pin();

    return ret;
}

static ILI9341_Status ili9341_configure_vcom_control_1(ILI9341_VCOMH_t vcomh_voltage, ILI9341_VCOML_t vcoml_voltage)
{
    /** <b>Local \c ILI9341_Status variable ret:</b> Holds the Return value of a @ref ILI9341_Status function type. */
    ILI9341_Status ret;
    /** <b>Local \c uint8_t variable ili9341_command:</b> Holds the ILI9341 Command that will be sent to it via the SPI-DMA peripheral. */
    uint8_t ili9341_command = ILI9341_VCOM_CONTROL_1_COMMAND;

    set_dc_pin_to_command_mode();
    enable_cs_pin();
    ret = ili9341_dma_spi_tx(&ili9341_command, ILI9341_COMMAND_SIZE);
    if (ret != ILI9341_EC_OK)
    {
        disable_cs_pin();
        return ret;
    }

    set_dc_pin_to_data_mode();
    /** <b>Local \c uint8_t 2-bytes array variable ili9341_data_value:</b> Holds both the VCOMH, in the first byte, and VCOML Voltages, in the second byte, with which it is desired to configure the VCOM Control 1 of the ILI9341 Device and that will be sent to it via the SPI-DMA peripheral. */
    uint8_t ili9341_data_value[ILI9341_VCOM_CONTROL_1_DATA_SIZE] = {(uint8_t) vcomh_voltage, (uint8_t) vcoml_voltage};
    ret = ili9341_dma_spi_tx(ili9341_data_value, ILI9341_VCOM_CONTROL_1_DATA_SIZE);
    disable_cs_pin();

    return ret;
}

static ILI9341_Status ili9341_configure_vcom_control_2(ILI9341_VMF_t vmf_offset)
{
    /** <b>Local \c ILI9341_Status variable ret:</b> Holds the Return value of a @ref ILI9341_Status function type. */
    ILI9341_Status ret;
    /** <b>Local \c uint8_t variable ili9341_command_or_data_value:</b> Holds either a Command or a Data that will be sent to the ILI9341 Device via the SPI-DMA peripheral. */
    uint8_t ili9341_command_or_data_value = ILI9341_VCOM_CONTROL_2_COMMAND;

    set_dc_pin_to_command_mode();
    enable_cs_pin();
    ret = ili9341_dma_spi_tx(&ili9341_command_or_data_value, ILI9341_COMMAND_SIZE);
    if (ret != ILI9341_EC_OK)
    {
        disable_cs_pin();
        return ret;
    }

    set_dc_pin_to_data_mode();
    ili9341_command_or_data_value = (uint8_t) vmf_offset;
    ret = ili9341_dma_spi_tx(&ili9341_command_or_data_value, ILI9341_SINGLE_DATA_SIZE);
    disable_cs_pin();

    return ret;
}

static ILI9341_Status ili9341_configure_memory_access_control(void)
{
    /** <b>Local \c ILI9341_Status variable ret:</b> Holds the Return value of a @ref ILI9341_Status function type. */
    ILI9341_Status ret;
    /** <b>Local \c uint8_t variable ili9341_command:</b> Holds the ILI9341 Command that will be sent to it via the SPI-DMA peripheral. */
    uint8_t ili9341_command = ILI9341_MEMORY_ACCESS_CONTROL_COMMAND;

    set_dc_pin_to_command_mode();
    enable_cs_pin();
    ret = ili9341_dma_spi_tx(&ili9341_command, ILI9341_COMMAND_SIZE);
    if (ret != ILI9341_EC_OK)
    {
        disable_cs_pin();
        return ret;
    }

    set_dc_pin_to_data_mode();
    /** <b>Local \c ILI9341_MADCTL_def_t variable ili9341_data_value:</b> Holds the configuration desired for the Memory Access Control Data, which is to be sent to the ILI9341 Device via the SPI-DMA peripheral. */
    ILI9341_MADCTL_def_t ili9341_data_value = {0};
    ili9341_data_value.mh = 0;
    ili9341_data_value.bgr = 1;
    ili9341_data_value.ml = 0;
    ILI9341_MADCTL_MCU_WRITE_READ_DIRECTION_def_t wr_rd_dir = {0};
    wr_rd_dir.mv = 0;
    wr_rd_dir.mx = 1;
    wr_rd_dir.my = 0;
    ili9341_data_value.wr_rd_dir = *((uint8_t *) &wr_rd_dir);
    ret = ili9341_dma_spi_tx(((uint8_t *) &ili9341_data_value), ILI9341_SINGLE_DATA_SIZE);
    disable_cs_pin();

    return ret;
}

// TODO: Pending to give versatility to this function so that it can allow its implementer to choose in between having 16-bits per pixel and 18-bits per pixel in order so that the color Union made in this library makes more sense.
static ILI9341_Status ili9341_configure_pixel_format(void)
{
    /** <b>Local \c ILI9341_Status variable ret:</b> Holds the Return value of a @ref ILI9341_Status function type. */
    ILI9341_Status ret;
    /** <b>Local \c uint8_t variable ili9341_command:</b> Holds the ILI9341 Command that will be sent to it via the SPI-DMA peripheral. */
    uint8_t ili9341_command = ILI9341_PIXEL_FORMAT_COMMAND;

    /* Update the @ref p_ili9341_fill_screen pointer and update @ref ili9341_bpp_type (for now, this is fixed to 16bpp; switch-case with 18bpp configuration is pending to be made). */
    p_ili9341_fill_screen = &ili9341_fill_screen_16bpp;
    ili9341_bpp_type = ILI9341_BPP_16;

    /* Configure the ILI9341 Pixel Format with the desired Bits Per Pixel (BPP). */
    set_dc_pin_to_command_mode();
    enable_cs_pin();
    ret = ili9341_dma_spi_tx(&ili9341_command, ILI9341_COMMAND_SIZE);
    if (ret != ILI9341_EC_OK)
    {
        disable_cs_pin();
        return ret;
    }

    set_dc_pin_to_data_mode();
    /** <b>Local \c ILI9341_PIXEL_FORMAT_def_t variable ili9341_data_value:</b> Holds the configuration desired for the Pixel Format Data, which is to be sent to the ILI9341 Device via the SPI-DMA peripheral. */
    ILI9341_PIXEL_FORMAT_def_t ili9341_data_value = {0};
    ili9341_data_value.dbi = 0x05;
    ili9341_data_value.dpi = 0x05;
    ret = ili9341_dma_spi_tx(((uint8_t *) &ili9341_data_value), ILI9341_SINGLE_DATA_SIZE);
    disable_cs_pin();

    return ret;
}

static ILI9341_Status ili9341_configure_display_function_control(void)
{
    /** <b>Local \c ILI9341_Status variable ret:</b> Holds the Return value of a @ref ILI9341_Status function type. */
    ILI9341_Status ret;
    /** <b>Local \c uint8_t variable ili9341_command:</b> Holds the ILI9341 Command that will be sent to it via the SPI-DMA peripheral. */
    uint8_t ili9341_command = ILI9341_DISPLAY_FUNCTION_CONTROL_COMMAND;

    set_dc_pin_to_command_mode();
    enable_cs_pin();
    ret = ili9341_dma_spi_tx(&ili9341_command, ILI9341_COMMAND_SIZE);
    if (ret != ILI9341_EC_OK)
    {
        disable_cs_pin();
        return ret;
    }

    set_dc_pin_to_data_mode();
    /** <b>Local \c ILI9341_PIXEL_FORMAT_def_t variable ili9341_data_value:</b> Holds the configuration desired for the Pixel Format Data, which is to be sent to the ILI9341 Device via the SPI-DMA peripheral. */
    uint8_t ili9341_data_value[ILI9341_DISPLAY_FUNCTION_CONTROL_DATA_SIZE];
    // TODO: For now, these following 3 bytes are going to be set in a fixed manner by following what it is stated in the ILI9341 datasheet. However, It is still pending to make proper enums and/or structs so that all the inner fields are properly documented and can be customized in a friendly manner.
    ili9341_data_value[0] = 0x08;
    ili9341_data_value[1] = 0x82;
    ili9341_data_value[2] = 0x27;
    ret = ili9341_dma_spi_tx(ili9341_data_value, ILI9341_DISPLAY_FUNCTION_CONTROL_DATA_SIZE);
    disable_cs_pin();

    return ret;
}

// TODO: Pending to add the "enter to sleep mode" function.
static ILI9341_Status ili9341_exit_sleep_mode(void)
{
    /** <b>Local \c ILI9341_Status variable ret:</b> Holds the Return value of a @ref ILI9341_Status function type. */
    ILI9341_Status ret;
    /** <b>Local \c uint8_t variable ili9341_command:</b> Holds the ILI9341 Command that will be sent to it via the SPI-DMA peripheral. */
    uint8_t ili9341_command = ILI9341_SLEEP_OUT_COMMAND;

    set_dc_pin_to_command_mode();
    enable_cs_pin();
    ret = ili9341_dma_spi_tx(&ili9341_command, ILI9341_COMMAND_SIZE);
    disable_cs_pin();
    HAL_Delay(5);

    return ret;
}

// TODO: Pending to add the  "turn display off" function.
static ILI9341_Status ili9341_turn_display_on(void)
{
    /** <b>Local \c ILI9341_Status variable ret:</b> Holds the Return value of a @ref ILI9341_Status function type. */
    ILI9341_Status ret;
    /** <b>Local \c uint8_t variable ili9341_command:</b> Holds the ILI9341 Command that will be sent to it via the SPI-DMA peripheral. */
    uint8_t ili9341_command = ILI9341_DISPLAY_ON_COMMAND;

    set_dc_pin_to_command_mode();
    enable_cs_pin();
    ret = ili9341_dma_spi_tx(&ili9341_command, ILI9341_COMMAND_SIZE);
    disable_cs_pin();

    return ret;
}

// TODO: Pending to pass the following "public" functions to the header file and to document them.
ILI9341_Status set_ili9341_bpp_type(ILI9341_BPP_t bpp)
{
    /* Update the @ref p_ili9341_fill_screen pointer and update @ref ili9341_bpp_type . */
    switch (bpp)
    {
        case ILI9341_BPP_16:
            p_ili9341_fill_screen = &ili9341_fill_screen_16bpp;
            break;
        case ILI9341_BPP_18:
            p_ili9341_fill_screen = &ili9341_fill_screen_18bpp;
            break;
        default:
            return ILI9341_EC_ERR; // The requested BPP type is not recognized. Therefore, send Error Exception Code.
    }
    ili9341_bpp_type = bpp;

    return ILI9341_EC_OK;
}

ILI9341_Status ili9341_fill_screen(ILI9341_COLOR color)
{
    // TODO: Write set_address function here.
    return (*p_ili9341_fill_screen)(color);
}

static ILI9341_Status ili9341_fill_screen_18bpp(ILI9341_COLOR color)
{
    // Write code here.
}

static ILI9341_Status ili9341_fill_screen_16bpp(ILI9341_COLOR color)
{
    // TODO: Write the function for coloring the bg of the ILI9341 TFT LCD Display.
}

// ##### LAST TODO UP TO HERE ##### //

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

