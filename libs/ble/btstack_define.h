#ifndef _BTSTACK_DEFINE_H
#define _BTSTACK_DEFINE_H

#define BLE_ALARM_TIMER_LEN                                         28


//BLE_EVT_FORMAT
/**
 * @format 21
 * @param handle
 * @param status
 */
#define BLE_EVT_PROFILE_FIND_DONE                                   0xA0

/**
 * @format 2228
 * @param con_handle
 * @param start_group_handle
 * @param end_group_handle
 * @param uuid128
 */
#define BLE_EVT_SERVICE_FIND_RESULT                                 0xA1

/**
 * @format 222228
 * @param con_handle
 * @param start_handle
 * @param value_handle
 * @param end_handle
 * @param properties
 * @param uuid128
 */
#define BLE_EVT_CHARACTERISTIC_FIND_RESULT                          0xA2


/**
 * @format 12222
 * @param status
 * @param handle
 * @param interval
 * @param latency
 * @param timeout
 */
#define BLE_EVT_CONNECT_PARAM_UPDATE_DONE                           0xB2

/**
 * @format 16211
 * @param address_type
 * @param address
 * @param handle
 * @param role
 * @param status
 */
#define BLE_EVT_CONNECT                                             0xB3

/**
 * @format 21
 * @param handle
 * @param disconnect_reason
 */
#define BLE_EVT_DISCONNECT                                          0xB4

/**
 * @format 22
 * @param handle
 * @param MTU
 */
#define BLE_EVT_MTU_EXCHANGE_DONE                                   0xB5

 /**
  * @format 122
  * @param status
  * @param conn_handle
  * @param attribute_handle
  */
#define BLE_EVT_INDICATE_DONE                                       0xB6

/**
 * @format 121
 * @param status
 * @param conn_handle
 * @param encryption_enabled
 */
#define BLE_EVT_ENCRYPTION_CHANGE                                   0xC7

 /**
  * @format 21614
  * @param handle
  * @param addr_type
  * @param address
  * @param secure_connection - set to 1 if LE Secure Connection pairing will be used
  * @param passkey
  */
#define BLE_EVT_PASSKEY_DISPLAY                                     0xC9


 /**
  * @format 2161
  * @param handle
  * @param addr_type
  * @param address
  * @param secure_connection - set to 1 if LE Secure Connection pairing will be used
  */
#define BLE_EVT_PASSKEY_INPUT                                       0xCB

  /**
   * @format H1B14
   * @param handle
   * @param addr_type
   * @param address
   * @param secure_connection - set to 1 if LE Secure Connection pairing will be used
   * @param passkey
   */
#define BLE_EVT_NUMERIC_COMPARISON_REQUEST                          0xCC

 /**
  * @format 216
  * @param handle
  * @param addr_type
  * @param address
  */
#define BLE_EVT_IDENTITY_RESOLVING_FAIL                             0xCE

 /**
  * @brief Identify resolving succeeded
  *
  * @format 216162
  * @param handle
  * @param addr_type
  * @param address
  * @param identity_addr_type
  * @param identity_address
  * @param index
  *
  */
#define BLE_EVT_IDENTITY_RESOLVING_SUCCEED                          0xCF


/**
 * @brief Emitted to inform app that pairing has started.
 * @format 216
 * @param handle
 * @param addr_type
 * @param address
 */
#define BLE_EVT_PAIRING_START                                       0xD4

/**
  * @brief Emitted to inform app that pairing is complete.
  * @format 21611
  * @param handle
  * @param addr_type
  * @param address
  * @param status
  * @param reason
  */
#define BLE_EVT_PAIRING_DONE                                        0xD5

/**
 * @brief Emitted to inform app that ltk is missing.
 * @format 216
 * @param handle
 * @param addr_type
 * @param address
 */
#define BLE_EVT_LTK_MISSING                                         0xD8


/**
 * @format 11611N
 * @param advertising_event_type
 * @param address_type
 * @param address
 * @param rssi
 * @param data_length
 * @param data
 */
#define BLE_EVT_ADV_REPORT                                          0xE2

/**
 * @format 21B1111121BJV
 * @param advertising_event_type
 * @param address_type
 * @param address
 * @param primary_phy
 * @param secondary_phy
 * @param advertising_sid
 * @param tx_power
 * @param rssi
 * @param periodic_advertising_interval
 * @param direct_address_type
 * @param direct_address
 * @param data_length
 * @param data
 */
#define BLE_EVT_EXTEND_ADV_REPORT                                   0xE3u


//Attribute Property
#define ATT_PROPERTY_BROADCAST                                      0x01
#define ATT_PROPERTY_READ                                           0x02
#define ATT_PROPERTY_WRITE_WITHOUT_RESPONSE                         0x04
#define ATT_PROPERTY_WRITE                                          0x08
#define ATT_PROPERTY_NOTIFY                                         0x10
#define ATT_PROPERTY_INDICATE                                       0x20
#define ATT_PROPERTY_AUTHENTICATED_SIGNED_WRITE                     0x40
#define ATT_PROPERTY_EXTENDED_PROPERTIES                            0x80


//GATT_CLIENT_CHARACTERISTIC_CONFIGURATION
#define GATT_CCC_NOTIFICATION                                       0x01
#define GATT_CCC_INDICATION                                         0x02


//SM_AUTHREQ
#define SM_AUTHREQ_NO_BONDING                                       0x00
#define SM_AUTHREQ_BONDING                                          0x01
#define SM_AUTHREQ_MITM_PROTECTION                                  0x04
#define SM_AUTHREQ_SECURE_CONNECTION                                0x08
#define SM_AUTHREQ_KEYPRESS                                         0x10
#define SM_AUTHREQ_CT2                                              0x20


// ATT_ERR_CODE
typedef enum {
    ATT_ERROR_NO_ERROR                                          = 0x00,
    ATT_ERROR_INVALID_HANDLE                                    = 0x01,
    ATT_ERROR_READ_NOT_PERMITTED                                = 0x02,
    ATT_ERROR_WRITE_NOT_PERMITTED                               = 0x03,
    ATT_ERROR_INVALID_PDU                                       = 0x04,
    ATT_ERROR_INSUFFICIENT_AUTHENTICATION                       = 0x05,
    ATT_ERROR_REQUEST_NOT_SUPPORTED                             = 0x06,
    ATT_ERROR_INVALID_OFFSET                                    = 0x07,
    ATT_ERROR_INSUFFICIENT_AUTHORIZATION                        = 0x08,
    ATT_ERROR_PREPARE_QUEUE_FULL                                = 0x09,
    ATT_ERROR_ATTRIBUTE_NOT_FOUND                               = 0x0a,
    ATT_ERROR_ATTRIBUTE_NOT_LONG                                = 0x0b,
    ATT_ERROR_INSUFFICIENT_ENCRYPTION_KEY_SIZE                  = 0x0c,
    ATT_ERROR_INVALID_ATTRIBUTE_VALUE_LENGTH                    = 0x0d,
    ATT_ERROR_UNLIKELY_ERROR                                    = 0x0e,
    ATT_ERROR_INSUFFICIENT_ENCRYPTION                           = 0x0f,
    ATT_ERROR_UNSUPPORTED_GROUP_TYPE                            = 0x10,
    ATT_ERROR_INSUFFICIENT_RESOURCES                            = 0x11,
    ATT_ERROR_VALUE_NOT_ALLOWED                                 = 0x13,

    ATT_ERROR_IN_WRONG_STATE                                    = 0x80,
    ATT_ERROR_NOTIFICATION_NOT_SUPPORTED                        = 0x81,
    ATT_ERROR_INDICATION_NOT_SUPPORTED                          = 0x82,
    ATT_ERROR_INDICATION_IN_PORGRESS                            = 0x83,
    ATT_ERROR_TIMEOUT                                           = 0x84,
    ATT_ERROR_DISCONNECT                                        = 0x85,
    ATT_ERROR_DATA_MISMATCH                                     = 0x86,
} ATT_ERR_CODE;


//BLE WORK MODE
typedef enum {
    MODE_NORMAL     = 0,        //normal mode
    MODE_FCC_TEST,              //fcc test mode, use uart for communication
    MODE_BQB_RF_BLE,            //bqb test mode，use uart for communication
    MODE_IODM_TEST,             //iodm test mode，use uart for communication
    MODE_WIRELESS,              //wireless mode
} BT_WORK_MODE;


typedef enum {
    FCC_TX_TONE = 1,
    FCC_TX_TEST,
    FCC_RX_TEST,
} FCC_RF_MODE;

typedef enum {
    ATT_TRANSACTION_WRITE = 0x00,
    ATT_TRANSACTION_PREPARE_WRITE,
    ATT_TRANSACTION_WRITE_EXECUTE,
    ATT_TRANSACTION_WRITE_CANCEL,
} ATT_TRANSACTION_MODE;

typedef enum {
    BLE_TXPWR_MINUS_6DB = 0x86,
    BLE_TXPWR_MINUS_4DB = 0x84,
    BLE_TXPWR_0DB = 0x00,
    BLE_TXPWR_4DB = 0x04,
    BLE_TXPWR_8DB = 0x08,
} ble_txpwr_sel_t;

enum {
    TXPKT_KICK_DISABLE = 0,
    TXPKT_KICK_ENABLE,
    TXPKT_KICK_AUTO,
};

typedef enum {
    BT_ALARM_ERR_CODE_NO_ERR = 0x00,
    BT_ALARM_ERR_CODE_UNINITIALIZE,
    BT_ALARM_ERR_CODE_NOT_FREE_MEM,
    BT_ALARM_ERR_CODE_NOT_EXIST_ID,
    BT_ALARM_ERR_CODE_ALREADY_ACTIVE,
    BT_ALARM_ERR_CODE_ALREADY_STOP,
    BT_ALARM_ERR_CODE_INTV_OUT_RANGE,
} bt_timer_err_code_t;

/// Specify what PHY Host prefers to use for RX or TX HCI:7.8.48 / HCI:7.8.49
enum le_phy_mask
{
    /// The Host prefers to use the LE 1M transmitter/receiver PHY (possibly among others)
    PHY_1MBPS_BIT       = (1<<0),
    /// The Host prefers to use the LE 2M transmitter/receiver PHY (possibly among others)
    PHY_2MBPS_BIT       = (1<<1),
    /// The Host prefers to use the LE Coded transmitter/receiver PHY (possibly among others)
    PHY_CODED_BIT       = (1<<2),
};

enum le_phy_value
{
    PHY_UNDEF_VALUE    = 0,
    PHY_1MBPS_VALUE    = 1,
    PHY_2MBPS_VALUE    = 2,
    PHY_CODED_VALUE    = 3,
    PHY_CODED_S2_VALUE = 4,
};

/// Advertising properties bit field bit positions
enum le_adv_prop_bf
{
    // Indicate that advertising is connectable, reception of CONNECT_REQ or AUX_CONNECT_REQ
    // PDUs is accepted. Not applicable for periodic advertising.
    LE_ADV_PROP_CONNECTABLE_POS             = 0,
    LE_ADV_PROP_CONNECTABLE_BIT             = (1UL << LE_ADV_PROP_CONNECTABLE_POS),

    // Indicate that advertising is scannable, reception of SCAN_REQ or AUX_SCAN_REQ PDUs is
    // accepted
    LE_ADV_PROP_SCANNABLE_POS               = 1,
    LE_ADV_PROP_SCANNABLE_BIT               = (1UL << LE_ADV_PROP_SCANNABLE_POS),

    // Indicate that advertising targets a specific device. Only apply in following cases:
    //   - Legacy advertising: if connectable
    //   - Extended advertising: connectable or (non connectable and non discoverable)
    LE_ADV_PROP_DIRECTED_POS                = 2,
    LE_ADV_PROP_DIRECTED_BIT                = (1UL << LE_ADV_PROP_DIRECTED_POS),

    // Indicate that High Duty Cycle has to be used for advertising on primary channel
    // Apply only if created advertising is not an extended advertising
    LE_ADV_PROP_HDC_POS                     = 3,
    LE_ADV_PROP_HDC_BIT                     = (1UL << LE_ADV_PROP_HDC_POS),

    // Use legacy advertising PDUs
    LE_ADV_PROP_LEGACY_POS                  = 4,
    LE_ADV_PROP_LEGACY_BIT                  = (1UL << LE_ADV_PROP_LEGACY_POS),

    // Omit advertiser's address from all PDUs("anonymous advertising").
    // Valid only if created advertising is an extended advertising
    LE_ADV_PROP_ANONYMOUS_POS               = 5,
    LE_ADV_PROP_ANONYMOUS_BIT               = (1UL << LE_ADV_PROP_ANONYMOUS_POS),

    // Include TxPower in the extented header of at least one advertisint PDU.
    // Valid only if created advertising is not a legacy advertising
    LE_ADV_PROP_TX_PWR_POS                  = 6,
    LE_ADV_PROP_TX_PWR_BIT                  = (1UL << LE_ADV_PROP_TX_PWR_POS),
};

/// Advertising properties configurations for extended advertising
enum le_ext_adv_prop
{
    // Non connectable non scannable extended advertising
    LE_EXT_ADV_PROP_NON_CONN_NON_SCAN_MASK  = 0x0000,
    // Non connectable scannable extended advertising
    LE_EXT_ADV_PROP_NON_CONN_SCAN_MASK      = LE_ADV_PROP_SCANNABLE_BIT,
    // Non connectable scannable directed extended advertising
    LE_EXT_ADV_PROP_NON_CONN_SCAN_DIR_MASK  = LE_ADV_PROP_SCANNABLE_BIT | LE_ADV_PROP_DIRECTED_BIT,
    // Non connectable anonymous directed extended advertising
    LE_EXT_ADV_PROP_ANONYM_DIR_MASK         = LE_ADV_PROP_ANONYMOUS_BIT | LE_ADV_PROP_DIRECTED_BIT,
    // Undirected connectable extended advertising
    LE_EXT_ADV_PROP_UNDIR_CONN_MASK         = LE_ADV_PROP_CONNECTABLE_BIT,
    // Directed connectable extended advertising
    LE_EXT_ADV_PROP_DIR_CONN_MASK           = LE_ADV_PROP_CONNECTABLE_BIT | LE_ADV_PROP_DIRECTED_BIT,
    // Legacy un directed connectable and scannable advertising
    LE_LEGACY_ADV_PROP_MASK                 = LE_ADV_PROP_CONNECTABLE_BIT | LE_ADV_PROP_SCANNABLE_BIT | LE_ADV_PROP_LEGACY_BIT,
};

enum le_initiator_filter_policy
{
    LE_INIT_FILT_IGNORE_WLST = 0x00,
    LE_INIT_FILT_USE_WLST = 0x01,
};

///Advertising filter policy
enum le_adv_filter_policy
{
    ///Allow both scan and connection requests from anyone
    LE_ADV_ALLOW_SCAN_ANY_CON_ANY    = 0x00,
    ///Allow both scan req from White List devices only and connection req from anyone
    LE_ADV_ALLOW_SCAN_WLST_CON_ANY,
    ///Allow both scan req from anyone and connection req from White List devices only
    LE_ADV_ALLOW_SCAN_ANY_CON_WLST,
    ///Allow scan and connection requests from White List devices only
    LE_ADV_ALLOW_SCAN_WLST_CON_WLST,
};

enum adv_channel_map
{
    ///Byte value for advertising channel map for channel 37 enable
    ADV_CHNL_37_EN                = 0x01,
    ///Byte value for advertising channel map for channel 38 enable
    ADV_CHNL_38_EN                = 0x02,
    ///Byte value for advertising channel map for channel 39 enable
    ADV_CHNL_39_EN                = 0x04,
    ///Byte value for advertising channel map for channel 37, 38 and 39 enable
    ADV_ALL_CHNLS_EN              = 0x07,
};

typedef enum {
    LE_CONN_INITIATE_ERR_NO_ERR,
    LE_CONN_INITIATE_ERR_CONN_EXISTS,
    LE_CONN_INITIATE_ERR_MEMORY_CAPA_EXCEED,
    LE_CONN_INITIATE_ERR_INVALID_PARAM,
} le_conn_initiate_err_code_t;

// IO Capability Values
enum {
    IO_CAPABILITY_DISPLAY_ONLY = 0,
    IO_CAPABILITY_DISPLAY_YES_NO,
    IO_CAPABILITY_KEYBOARD_ONLY,
    IO_CAPABILITY_NO_INPUT_NO_OUTPUT,
    IO_CAPABILITY_KEYBOARD_DISPLAY, // not used by secure simple pairing
};

//ADDRESS TYPE
typedef enum {
    GAP_PUBLIC_ADDRESS_TYPE = 0,
    GAP_RANDOM_ADDRESS_TYPE_STATIC,
    GAP_RANDOM_ADDRESS_NON_RESOLVABLE,
    GAP_RANDOM_ADDRESS_RESOLVABLE,
} gap_random_address_type_t;

typedef enum {
    BD_ADDR_TYPE_PUBLIC = 0,
    BD_ADDR_TYPE_RANDOM = 1,
} bd_addr_type_t;


//ADV_TYPE
enum {
    ADV_TYPE_IND = 0,
    ADV_TYPE_DIRECT_HI_IND,
    ADV_TYPE_SCAN_IND,
    ADV_TYPE_NONCONN_IND,
    ADV_TYPE_DIRECT_LO_IND,
};

//SCAN STA
typedef enum {
    LE_SCAN_STA_DISABLE,
    LE_SCAN_STA_ENABLE,
    LE_SCAN_STA_W4_DISABLE,
    LE_SCAN_STA_W4_ENABLE,
} le_scanning_state_t;

typedef uint8_t sm_key_t[16];

typedef uint16_t ble_con_handle_t;

typedef uint8_t bt_timer_handle_t;

typedef void (*kick_func_t)(uint16_t con_handle, uint16_t config);

typedef uint16_t (*att_read_callback_t)(uint16_t con_handle, uint16_t attribute_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size);

typedef int (*att_write_callback_t)(uint16_t con_handle, uint16_t attribute_handle, ATT_TRANSACTION_MODE trans_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);

typedef uint16_t (*att_notify_callback_t)(uint16_t con_handle, uint16_t attribute_handle, uint8_t * buffer, uint16_t buffer_size);

typedef void (*att_event_callback_t) (uint8_t event_type, uint8_t *param, uint16_t size);

typedef void (*ble_init_callback_t)(void);

typedef void (*bb_timer_proc_callback_t)(uint32_t time);

typedef void (*ble_vs_tbox_init_callback_t)(void);

typedef struct {
    uint8_t adv_en;
    uint8_t adv_data[31];
    uint8_t adv_len;
    uint8_t scan_rsp[31];
    uint8_t scan_rsp_len;
    uint16_t adv_int_min;           //advertisement interval
    uint16_t adv_int_max;           //advertisement interval
    uint8_t adv_type;
    uint8_t direct_address_typ;     //only use for direct adv
    uint8_t direct_address[6];      //only use for direct adv
    uint8_t channel_map;
    uint8_t filter_policy;
} adv_param_t;

typedef struct __attribute__((packed)){
    uint8_t     event_type;
    uint8_t     addr_type;
    uint8_t     addr[6];
    char        rssi;
    uint8_t     report_len;
    const uint8_t * report;
    uint8_t     offset;
} adv_report_t;

typedef struct __attribute__((packed)){
    uint16_t    evt_type;
    uint8_t     addr_type;
    uint8_t     addr[6];
    uint8_t     phy;
    uint8_t     phy2;
    uint8_t     adv_sid;
    uint8_t     tx_pwr;
    char        rssi;
    uint16_t     intv;
    ///Direct address type
    uint8_t     dir_addr_type;
    ///Direct address value
    uint8_t     dir_addr[6];
    uint8_t     report_len;
    const uint8_t * report;
    uint8_t     offset;
} ext_adv_report_t;

typedef struct {
    uint8_t adv_en;
    uint8_t adv_data[31];
    uint8_t adv_len;
    uint8_t scan_rsp[31];
    uint8_t scan_rsp_len;
    uint8_t peer_addr_type;
    uint8_t peer_addr[6];
    uint8_t channel_map;
    uint8_t filter_policy;

    // Used to identify an advertising set, range:0x00 ~ 0xef
    uint8_t adv_hdl;
    // BIT(0)-Connectable advertising / BIT(1)-Scannable advertising / BIT(2)-Directed advertising
    // BIT(3)-High Duty Directed Connectable advertising(less than or equl to 3.75ms Advertising Interval)
    // BIT(4)-Use legacy advertising PDUs / BIT(5)-Omit advertiser's address from all PDUs("anonymous advertising")
    // BIT(6)-Include TxPower in the extented header of at least one advertisint PDU
    uint8_t adv_evt_prop;
    // Minimum advertising interval for undirected and low duty cycle directed advertising.Range:0x20 ~ 0xffffff.Unit 0.625ms
    uint32_t prim_adv_int_min;
    // Maximum advertising interval for undirected and low duty cycle directed advertising.Range:0x20 ~ 0xffffff.Unit 0.625ms
    uint32_t prim_adv_int_max;
    // Range -127 to +20.Unit:dbm
    int8_t max_tx_pwr;
    // 0x01-Primary advertising PHY is LE 1M / 0x03-Primary advertising PHY is LE Coded
    uint8_t prim_phy;
    // 0x00-AUX_ADV_IND shall be sent prior to the next advertising event
    // 0x01-Maximum advertsing events the Controller can skip before sending the AUX_ADV_IND packets on the secondary advertising physical channel.
    uint8_t max_skip;
    // 0x01-Secondary advertising PHY is LE 1M / 0x02-Secondary advertising PHY is LE 2M
    // 0x03-Secondary advertising PHY is LE Coded:125K / 0x03-Secondary advertising PHY is LE Coded:500K
    uint8_t second_phy;
    // Value of the Advertising SID subfield in the ADI field of the PDU
    uint8_t adv_sid;
    // 0x00-Scan request notifications disabled / 0x01-Scan request notifications enabled
    uint8_t scan_req_notif_en;
} ext_adv_param_t;

typedef struct {
     uint8_t   adv_type;
     uint8_t   data_len;
     uint8_t*  data;
} adv_struct_t;

typedef struct {
    uint8_t scan_type;
    uint16_t scan_interval;
    uint16_t scan_window;
} scan_param_t;

typedef struct {
    // 0x00-Basic unfiltered scanning filter policy / 0x01-Basic filtered scanning filter policy
    // 0x02-Extented unfiltered scanning filter policy / 0x03-Extented filtered scanning filter policy
    uint8_t scan_filter_policy;
    // BIT(0)-Scan advertising on the LE 1M PHY
    // BIT(1)-Scan advertising on the LE 2M PHY
    // BIT(2)-Scan advertising on the LE Coded PHY
    uint8_t scan_phys;
        // 0x00-Passive Scanning.No scan request PDUs shall be sent.
        // 0x01-Active Scanning.Scan request PDUs shall be sent.
    uint8_t scan_type;
    uint16_t scan_interval;
    uint16_t scan_window;
} ext_scan_param_t;

typedef struct __attribute__((packed))
{
    ///Initiator filter policy
    uint8_t     init_filter_policy;
    ///Own address type public/random/rpa
    //uint8_t     own_addr_type;
    ///Peer address type public/random/rpa
    uint8_t     peer_addr_type;
    ///Peer address
    uint8_t     peer_addr[6];
    ///Indicates the PHY(s) on which the advertising packets should be received
    uint8_t     init_phys;
    ///Paramaters for PHY(s), PHY_1M / PHY_2M / PHY_CODED
    ///Scan interval (N * 0.625 ms)
    //uint16_t    scan_interval;
    ///Scan window size (N * 0.625 ms)
    //uint16_t    scan_window;
    ///Minimum of connection interval (N * 1.25 ms)
    uint16_t    con_intv_min;
    ///Maximum of connection interval (N * 1.25 ms)
    uint16_t    con_intv_max;
    ///Connection latency
    uint16_t    con_latency;
    ///Link supervision timeout
    uint16_t    superv_to;
    ///Minimum CE length (N * 0.625 ms)
    uint16_t    ce_len_min;
    ///Maximum CE length (N * 0.625 ms)
    uint16_t    ce_len_max;
} ext_conn_create_param_t;

typedef uint8_t bd_addr_t[6];

typedef struct {
    // Key value MSB -> LSB
    uint8_t key[16];
} gap_sec_key_t;

// Do not modify!!
typedef struct {
    // Address type of device. 0 public/1 random
    uint8_t addr_type;
    // BD Address of device
    bd_addr_t addr;
} gap_bdaddr_t;

/**
 * @brief Identity Resolving Key Information
 */
typedef struct {
    // Identify Resolving Key
    gap_sec_key_t irk;
    // Identity broadcast address of device
    gap_bdaddr_t addr;
} gapc_irk_t;

/**
 * @brief Resolving list information about a device.
 */
typedef struct {
    // Privacy Mode
    uint8_t priv_mode;  //PRIV_TYPE_NETWORK = 0x00, Controller accept identity address only
                        //PRIV_TYPE_DEVICE  = 0x01, Controller accept both identity address and resolvable address.
    // Device Identity address
    gap_bdaddr_t addr;
    // Peer device identity resolve key
    uint8_t peer_irk[16];
} gap_ral_dev_info_t;

typedef struct {
    uint8_t rpa_en;
    uint8_t privacy_mode;
    uint16_t renew_to;
    gap_sec_key_t peer_irk;
    gap_bdaddr_t addr;
} rpa_param_t;

typedef struct {
    gap_bdaddr_t addr;
} wlist_param_t;

typedef struct {
    uint16_t        address_type;
    uint16_t        real_address_type;
    bd_addr_t       address;
    gap_sec_key_t   peer_irk;
} sm_bonding_info_t;

// Read & Write Callbacks for handle range
typedef struct att_service_handler {
    void * item;
    uint16_t start_handle;
    uint16_t end_handle;
    att_read_callback_t read_callback;
    att_write_callback_t write_callback;
    att_event_callback_t event_handler;
} att_service_handler_t;

typedef struct att_client_handler {
    void * item;
    att_read_callback_t read_callback;
    att_notify_callback_t notify_callback;
    att_event_callback_t event_handler;
} att_client_handler_t;

typedef struct {
    uint16_t start_group_handle;
    uint16_t end_group_handle;
    uint16_t uuid16;
    uint8_t  uuid128[16];
} gatt_client_service_t;

typedef struct {
    uint16_t start_handle;
    uint16_t value_handle;
    uint16_t end_handle;
    uint16_t properties;
    uint16_t uuid16;
    uint8_t  uuid128[16];
} gatt_client_characteristic_t;

struct txbuf_tag {
    list_hdr_t  hdr;
    uint16_t    len;
    uint16_t    att_handle;
    uint16_t    con_handle;
    uint16_t    config;
    uint8_t     ptr[0];
};

typedef struct {
    uint8_t rf_mode;
    uint8_t tx_power;
    uint8_t tx_freq;
    uint8_t rx_freq;
    uint8_t phy_type;
} fcc_cfg_t;


typedef struct le_device_memory_db {
    // Identification
    uint16_t addr_type;
    uint16_t real_addr_type;
    bd_addr_t addr;
    sm_key_t irk;

    // Stored pairing information allows to re-establish an enncrypted connection
    // with a peripheral that doesn't have any persistent memory
    sm_key_t ltk;
    uint16_t ediv;
    uint8_t  rand[8];
    uint8_t  key_size;
    uint8_t  authenticated;
    uint8_t  authorized;
} le_device_memory_db_t;

typedef struct
{
    uint8_t  vers;
    uint16_t compid;
    uint16_t subvers;
} ll_version_t;

///Supported LE Features structure
typedef struct
{
    ///8-byte array for LE features
    uint8_t feats[8];
} ll_features_t;

typedef struct {
    uint8_t mem[28];
} bt_timer_mem_t;


#endif
