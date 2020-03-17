/**
 * Copyright (c) 2014 - 2018, Nordic Semiconductor ASA
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
/** @file
 *
 * @defgroup ble_sdk_app_hids_mouse_main main.c
 * @{
 * @ingroup ble_sdk_app_hids_mouse
 * @brief HID Mouse Sample Application main file.
 *
 * This file contains is the source code for a sample application using the HID, Battery and Device
 * Information Service for implementing a simple mouse functionality. This application uses the
 * @ref app_scheduler.
 *
 * Also it would accept pairing requests from any peer device. This implementation of the
 * application will not know whether a connected central is a known device or not.
 */

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "ble_gap.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_drv_wdt.h"
#include "app_timer.h"
#include "ble_nus.h"
#include "app_uart.h"
#include "app_util_platform.h"
#include "bsp_btn_ble.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_pwm.h"
#include "nrf_delay.h"
#include "nrf_drv_gpiote.h"

//Added by NDevice
#include "custom_board.h"
#include "ndevice_twim.h"
#include "process_data.h"
#include "ndevice_timer.h"
#include "ndevice_pwm.h"
#include "ndevice_nvmc.h"
#include "func.h"
#include "process_data.h"
#include "ndevice_spi.h"
#include "ndevice_eeprom.h"
#include "ndevice_gpio.h"
#include "ndevice_ble.h"
#include "ndevice_uart.h"
#include "shoponair.h"
#include "SEGGER_RTT.h"
//End of addition

#if defined (UART_PRESENT)
#include "nrf_uart.h"
#endif
#if defined (UARTE_PRESENT)
#include "nrf_uarte.h"
#endif

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

//Defined by NDevice
#define DEVICE_NAME                     "ShopOnair"                                /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME               "(c)ShopOnAir"                                       /**< Manufacturer. Will be passed to Device Information Service. */
//End of defines

#define APP_BLE_CONN_CFG_TAG            1                                           /**< A tag identifying the SoftDevice BLE configuration. */

#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the Nordic UART Service (vendor specific). */

#define APP_BLE_OBSERVER_PRIO           3                                           /**< Application's BLE observer priority. You shouldn't need to modify this value. */

#define APP_ADV_INTERVAL                80                  //MIN 50                /**< The advertising interval (in units of 0.625 ms. This value corresponds to 100 ms). */

#define APP_ADV_DURATION                0                                           /**< The advertising duration (180 seconds) in units of 10 milliseconds. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(50, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (40 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(100, UNIT_1_25_MS)            /**< Maximum acceptable connection interval (100 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                      /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */

#define BLE_UUID_POSCVT_SERVICE         0x4cc6                                      /**< The UUID of the POS_CVT. */

BLE_NUS_DEF(m_nus, NRF_SDH_BLE_TOTAL_LINK_COUNT);                                   /**< BLE NUS service instance. */
NRF_BLE_GATT_DEF(m_gatt);                                                           /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                                             /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising);                                                 /**< Advertising module instance. */

static uint16_t   m_conn_handle          = BLE_CONN_HANDLE_INVALID;                 /**< Handle of the current connection. */
static uint16_t   m_ble_nus_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - 3;            /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */
static ble_uuid_t m_adv_uuids[]          =                                          /**< Universally unique service identifier. */
{
  {BLE_UUID_POSCVT_SERVICE, NUS_SERVICE_UUID_TYPE}
};

//Added by NDevice
bool advertising_g = false;
uint8_t rx_buff_g[20];
APP_TIMER_DEF(data_gather);
static void timer_handler(void * p_context);
nrf_drv_wdt_channel_id m_channel_id;
bool ble_disconnecting = false;

//BLE Auto cut off 기능추가 2020.02.07 ksd
int EVTCH_RSSI; 						// EVT_RSSI_CHANGED RSSI RECEIVE
uint8_t macaddr[6]={0,};
extern uint32_t wait_cnt;

//사용처 megapay 는 테이블 오더로 인해 RSSI limit OFF 200216 KSD
//#define RSSI_LIMIT -128 // MEGAPAY TABLE ORDER 전용
#define RSSI_LIMIT -60 // NORMAL STATE

//unsigned char segnum[10] = {
//	0b11111100,  00111111
//	0b01100000,  00000110
//	0b11011010,  01011011
//	0b11110010,  01001111
//	0b01100110,  01100110
//	0b10110110,  01101101
//	0b00111110,  01111100
//	0b11100100,  00100111
//	0b11111110,  01111111
//	0b11100110   01100111
//};


//TWI I2C CODE
#define pinSDA 5
#define pinSCL 6

void begin(void);
extern uint8_t rxBuffer[256];
//End of addition

 void begin(void){
	// I2C 통신을 활성화 시키는 함수
	// 기본 통신 속도는 400kHz
	// 기본 설정 핀 : SCL(6번), SDA(5번)

	// I2C 통신 비활성화
	NRF_TWIS0->ENABLE = 0;

	// SCL/SDA핀 설정
	NRF_TWIS0->PSEL.SCL = pinSCL;
	NRF_TWIS0->PSEL.SDA = pinSDA;

	// TWIS 주소 설정
	NRF_TWIS0->ADDRESS[0] = 0x08;

	// TWIS 주소 활성화 여부 설정
	NRF_TWIS0->CONFIG = 0x01;	// ADDRESS[0]과 매칭

	// 인터럽트 비활성화
	NRF_TWIS0->INTENCLR = (0x01 << 1) | (0x01 << 9) | (0x01 << 19) |
			(0x01 << 20) | (0x01 << 25) | (0x01 << 26);

	
	// TWIS 활성화
	NRF_TWIS0->ENABLE = 9;
}

/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyse
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for initializing the timer module.
 */
static void timers_init(void)
{
  ret_code_t err_code;

  err_code = app_timer_init();
  APP_ERROR_CHECK(err_code);
        
  err_code = app_timer_create(&data_gather, APP_TIMER_MODE_REPEATED , timer_handler);
  APP_ERROR_CHECK(err_code);
}

void timers_stop(void)
{
  app_timer_stop(data_gather);
}

void wdt_event_handler(void)
{

}

static void initWatchdog(void)
{
    uint32_t err_code = NRF_SUCCESS;
    
    nrf_drv_wdt_config_t config = NRF_DRV_WDT_DEAFULT_CONFIG;
    err_code = nrf_drv_wdt_init(&config, wdt_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_wdt_channel_alloc(&m_channel_id);
    APP_ERROR_CHECK(err_code);
    nrf_drv_wdt_enable();
}

/**@brief Function for the GAP initialization.
 *
 * @details This function will set up all the necessary GAP (Generic Access Profile) parameters of
 *          the device. It also sets the permissions and appearance.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *) DEVICE_NAME, strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_evt       Nordic UART Service event.
 */
/**@snippet [Handling the data received over BLE] */
static void nus_data_handler(ble_nus_evt_t * p_evt)
{

    if (p_evt->type == BLE_NUS_EVT_RX_DATA)
    {
        parser_data(p_evt->params.rx_data.p_data, p_evt->params.rx_data.length, &m_nus, m_conn_handle);
    }

}
/**@snippet [Handling the data received over BLE] */


/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    uint32_t           err_code;
    ble_nus_init_t     nus_init;
    nrf_ble_qwr_init_t qwr_init = {0};

    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    // Initialize NUS.
    memset(&nus_init, 0, sizeof(nus_init));

    nus_init.data_handler = nus_data_handler;

    //err_code = ble_nus_init(&m_nus, &nus_init);
    err_code = ble_nd_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling an event from the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module
 *          which are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply setting
 *       the disconnect_on_fail config parameter, but instead we use the event handler
 *       mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;
    
    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        //SEGGER_RTT_printf(0,"Evt:0x%x\n",err_code);
        if((err_code != NRF_SUCCESS) &&
        (err_code != NRF_ERROR_INVALID_STATE) &&
        (err_code != NRF_ERROR_RESOURCES) &&
        (err_code != NRF_ERROR_BUSY)){
            APP_ERROR_CHECK(err_code);
        }
    }
}

/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            advertising_g = true;
            break;
        case BLE_ADV_EVT_IDLE:
            break;
        default:
            break;
    }
}

uint16_t ble_get_handle_state(void)
{
  return m_conn_handle;
}

void advertising_start(void);

/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    uint32_t err_code;
    static ble_gap_addr_t gap_addr;
    
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            advertising_g = false;
						wait_cnt = 0;	//0212 KSD
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);
				
            //BLE Auto cut off 기능추가 2020.02.07 ksd
            sd_ble_gap_addr_get(&gap_addr);
            macaddr[5] = p_ble_evt->evt.gap_evt.params.connected.peer_addr.addr[5];
            macaddr[4] = p_ble_evt->evt.gap_evt.params.connected.peer_addr.addr[4];
            macaddr[3] = p_ble_evt->evt.gap_evt.params.connected.peer_addr.addr[3];
            macaddr[2] = p_ble_evt->evt.gap_evt.params.connected.peer_addr.addr[2];
            macaddr[1] = p_ble_evt->evt.gap_evt.params.connected.peer_addr.addr[1];
            macaddr[0] = p_ble_evt->evt.gap_evt.params.connected.peer_addr.addr[0];
            sd_ble_gap_rssi_start(p_ble_evt->evt.gap_evt.conn_handle, 0 , 0);
				
            setTimeoutStart();
            break;

        case BLE_GAP_EVT_RSSI_CHANGED:
            EVTCH_RSSI = p_ble_evt->evt.gap_evt.params.rssi_changed.rssi;
            SEGGER_RTT_printf(0,"\nRSSI: %d",EVTCH_RSSI);
            if (p_ble_evt->evt.gap_evt.params.rssi_changed.rssi < RSSI_LIMIT) //RSSI_LIMIT NORMAL = -70 , MEGAPAY = -128 , 200216 KSD
            {
                PoscvtCfg_g.rssi_limit = true;
            }
            else
            {
                PoscvtCfg_g.rssi_limit = false;
            }
            sd_ble_gap_rssi_stop(p_ble_evt->evt.gap_evt.conn_handle);
            break;
				
        case BLE_GAP_EVT_DISCONNECTED:
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            ble_disconnecting = false;
            if(getReceiptFSM() == RECEIPT_FSM_READ)
            {
              //To avoid clean receipt
              receiptFSM(RECEIPT_FSM_READY);
              receiptFSM(RECEIPT_FSM_READY);      
              //End of avoiding
            }
            clrTagReceived();
            clr_SysEvent(~(SYSEVT_TIMEOUTSTART | SYSEVT_BLINK_LED));
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for the SoftDevice initialization.
 *
 * @details This function initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;
    
    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}


/**@brief Function for handling events from the GATT library. */
void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
    if ((m_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        NRF_LOG_INFO("\nData len is set to 0x%X(%d)", m_ble_nus_max_data_len, m_ble_nus_max_data_len);
    }
    NRF_LOG_DEBUG("\nATT MTU exchange completed. central 0x%x peripheral 0x%x",
                  p_gatt->att_mtu_desired_central,
                  p_gatt->att_mtu_desired_periph);
}


/**@brief Function for initializing the GATT library. */
void gatt_init(void)
{
    ret_code_t err_code;

    err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    uint32_t               err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance = false;
    init.advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.srdata.uuids_complete.p_uuids  = m_adv_uuids;

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;
    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
  //
}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for starting advertising.
 */
void advertising_start(void)
{
    uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for starting advertising.
 */
void advertising_stop(void)
{
   uint32_t err_code = sd_ble_gap_adv_stop(m_advertising.adv_handle);
   APP_ERROR_CHECK(err_code);
}

static void timer_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    cb_tick_handler();
}

/*
* @brief : connection state
*/
bool IsAdvertised(void)
{
  return advertising_g;
}

/*
* @brief : connection state
*/
bool IsConnected(void)
{
  if(m_conn_handle != BLE_CONN_HANDLE_INVALID) return true;
  else return false;
}

/*
* @brief : disconnect current connection
*/
void nd_disconnect(void)
{
    uint32_t err_code;

    if(IsConnected() != true) return;
    if(ble_disconnecting)
      return;
    err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
    if((err_code != NRF_SUCCESS) &&
        (err_code != NRF_ERROR_INVALID_STATE) &&
        (err_code != NRF_ERROR_RESOURCES) &&
        (err_code != NRF_ERROR_BUSY)){
        APP_ERROR_CHECK(err_code);
    }
    ble_disconnecting = true;
}

void spi_rcvHandler(void)
{
    //SEGGER_RTT_printf(0, "%02x %02x %02x", rx_buff_g[0], rx_buff_g[1], rx_buff_g[2]);
}

void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    uint32_t err_code;
    error_info_t *p_error_info = (error_info_t *)info;
    err_code = p_error_info->err_code;
    SEGGER_RTT_printf(0,"\n%d app_error_fault_handler / err_code = %d", get_tickcount() ,err_code);
    if((err_code != NRF_SUCCESS) &&
       (err_code != NRF_ERROR_INVALID_STATE) &&
       (err_code != NRF_ERROR_RESOURCES) &&
       (err_code != NRF_ERROR_BUSY) &&
       (err_code != NRF_ERROR_NOT_FOUND)){
        NVIC_SystemReset();
    }
}



extern bool auto_baud_rate_run(void);
extern void load_qr_code(void);
extern void init_parameter(void);
extern void pos_cnt_uart_parser(void);
/*
*@brief Function for application main entry.
*/
int main(void)
{
    uint32_t i;
    uint32_t tempUINT32;
    nd_gpio_init();                                   //GPIO initial
    log_init();
    timers_init();                                  	//Timer initial
    uart_init(TX_PIN_NUMBER, RX_PIN_NUMBER);          //UART initial        
    nd_spi_init(DEF_SPI_SRAM_SCK, DEF_SPI_SRAM_MISO, 
           DEF_SPI_SRAM_MOSI, DEF_SPI_SRAM_CS);	       //SPI initial
    power_management_init();                        	//Power initial
    ble_stack_init();                                 //BLE initial
    gap_params_init();                                //BLE initial
    gatt_init();                                      //BLE initial
    services_init();                                  //BLE initial
    advertising_init();                               //Advertising initial
    conn_params_init();                               //Connection parameter initial
    
    //Initialization by NDevice
    init_fstorage();                                  //Flash memory initial

    initWatchdog();                                   //Watchdog initial

    //End of initialization

    // Start execution.
    app_timer_start(data_gather, APP_TIMER_TICKS(10), NULL);
    
    init_play_sound();
    init_pos_parser();
    
    init_process();     //Clear Public Queue Index
    load_qr_code();     // Load QR Code in RAM

    init_stick_var();   //Clear Public Variables & Load Option Parameter

    if(auto_baud_rate_run()){
        init_stick_var();   //Clear Public Variables & Load Option Parameter
    }
    else //0212 KSD , auto baudrate 안하는 경우 메모리된 값으로 변경	
    {
        switch(PoscvtCfg_g.baudrate)
        {
            case CFG_BAUD_9600:
                uart_setBaudrate(9600);
                break;
	
            case CFG_BAUD_19200:
                uart_setBaudrate(19200);
                break;

            case CFG_BAUD_38400:
                uart_setBaudrate(38400);
                break;

            case CFG_BAUD_57600:
                uart_setBaudrate(57600);
                break;

            case CFG_BAUD_115200:
                uart_setBaudrate(115200);
                break;
					
            default:
                uart_setBaudrate(9600);
                break;
         }
    }
    //display LED for starting
    tempUINT32 = get_tickcount();
    nrf_gpio_pin_set(DEF_GPIO_RED1_PWM);
    for(i=0; i<6; )              //display starting
    {
      if((get_tickcount() - tempUINT32) > 50)
      {
        tempUINT32 = get_tickcount();
        
        NRF_WDT->RR[0] = WDT_RR_RR_Reload;                                                //clear watchdog        
        nrf_gpio_pin_toggle(DEF_GPIO_RED1_PWM);
        ++i;
      }
    }
    nrf_gpio_pin_set(DEF_GPIO_RED1_PWM);
    //End of display of starting    
    gpiote_init();                                       //Button Switch input interrupt initial
    init_parameter();                                    //Print relevant initial
    SoundPU();                                           //Sound module power up

    receiptFSM(RECEIPT_FSM_READY);                       //Recipt FSM initial
    advertising_start();                                 //Advertising start
    set_advertising_active();
    begin();
    i = 0;
    SEGGER_RTT_printf(0, "\r\n Start App..");
	
    while(1)
    {      
      NRF_WDT->RR[0] = WDT_RR_RR_Reload;                                                //clear watchdog
      stick_fsm(false);                                  //State machine active
      pos_cnt_uart_parser();                             //UART data processing
    }    
}


/**
 * @}
 */
