/*
 *  property_types.h - BCU 2 property types of EIB objects.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#pragma once

#include <stdint.h>
#include "save_restore.h"

/** The data type of a property. */
enum PropertyDataType
{
    PDT_CONTROL            = 0x00, //!< length: 1 read, 10 write
    PDT_CHAR               = 0x01, //!< length: 1
    PDT_UNSIGNED_CHAR      = 0x02, //!< length: 1
    PDT_INT                = 0x03, //!< length: 2
    PDT_UNSIGNED_INT       = 0x04, //!< length: 2
    PDT_KNX_FLOAT          = 0x05, //!< length: 2
    PDT_DATE               = 0x06, //!< length: 3
    PDT_TIME               = 0x07, //!< length: 3
    PDT_LONG               = 0x08, //!< length: 4
    PDT_UNSIGNED_LONG      = 0x09, //!< length: 4
    PDT_FLOAT              = 0x0a, //!< length: 4
    PDT_DOUBLE             = 0x0b, //!< length: 8
    PDT_CHAR_BLOCK         = 0x0c, //!< length: 10
    PDT_POLL_GROUP_SETTING = 0x0d, //!< length: 3
    PDT_SHORT_CHAR_BLOCK   = 0x0e, //!< length: 5
    PDT_DATE_TIME          = 0x0f, //!< length: 8
    PDT_VARIABLE_LENGTH    = 0x10, 
    PDT_GENERIC_01         = 0x11, //!< length: 1
    PDT_GENERIC_02         = 0x12, //!< length: 2
    PDT_GENERIC_03         = 0x13, //!< length: 3
    PDT_GENERIC_04         = 0x14, //!< length: 4
    PDT_GENERIC_05         = 0x15, //!< length: 5
    PDT_GENERIC_06         = 0x16, //!< length: 6
    PDT_GENERIC_07         = 0x17, //!< length: 7
    PDT_GENERIC_08         = 0x18, //!< length: 8
    PDT_GENERIC_09         = 0x19, //!< length: 9
    PDT_GENERIC_10         = 0x1a, //!< length: 10
    PDT_GENERIC_11         = 0x1b, //!< length: 11
    PDT_GENERIC_12         = 0x1c, //!< length: 12
    PDT_GENERIC_13         = 0x1d, //!< length: 13
    PDT_GENERIC_14         = 0x1e, //!< length: 14
    PDT_GENERIC_15         = 0x1f, //!< length: 15
    PDT_GENERIC_16         = 0x20, //!< length: 16
    PDT_GENERIC_17         = 0x21, //!< length: 17
    PDT_GENERIC_18         = 0x22, //!< length: 18
    PDT_GENERIC_19         = 0x23, //!< length: 19
    PDT_GENERIC_20         = 0x24, //!< length: 20
    PDT_UTF8               = 0x2f,  //!< length: 3
    PDT_VERSION            = 0x30,  //!< length: 3
    PDT_ALARM_INFO         = 0x31,  //!< length: 3
    PDT_BINARY_INFORMATION = 0x32,  //!< length: 3
    PDT_BITSET8            = 0x33,  //!< length: 3
    PDT_BITSET16           = 0x34,  //!< length: 3
    PDT_ENUM8              = 0x35,  //!< length: 3
    PDT_SCALING            = 0x36,  //!< length: 3
    PDT_NE_VL              = 0x3c,  //!< length: 3
    PDT_NE_FL              = 0x3d,  //!< length: 3
    PDT_FUNCTION           = 0x3e,  //!< length: 3
    PDT_ESCAPE             = 0x3f,  //!< length: 3
};

enum PropertyID
{
    /** Interface Object Type independent Properties */
    PID_OBJECT_TYPE = 1,
    PID_LOAD_STATE_CONTROL = 5,
    PID_RUN_STATE_CONTROL = 6,
    PID_TABLE_REFERENCE = 7,
    PID_SERVICE_CONTROL = 8,
    PID_FIRMWARE_REVISION = 9,
    PID_SERIAL_NUMBER = 11,
    PID_MANUFACTURER_ID = 12,
    PID_PROG_VERSION = 13,
    PID_DEVICE_CONTROL = 14,
    PID_ORDER_INFO = 15,
    PID_PEI_TYPE = 16,
    PID_PORT_CONFIGURATION = 17,
    PID_TABLE = 23,
    PID_VERSION = 25,
    PID_MCB_TABLE = 27,
    PID_ERROR_CODE = 28,
    PID_OBJECT_INDEX = 29,
    PID_DOWNLOAD_COUNTER = 30,
    
    /** Properties in the Device Object */
    PID_ROUTING_COUNT = 51,
    PID_PROG_MODE = 54,
    PID_MAX_APDU_LENGTH = 56,
    PID_SUBNET_ADDR = 57,
    PID_DEVICE_ADDR = 58,
    PID_IO_LIST = 71,
    PID_HARDWARE_TYPE = 78,
    PID_RF_DOMAIN_ADDRESS_CEMI_SERVER = 82,
    PID_DEVICE_DESCRIPTOR = 83,

    /** Properties in the RF Medium Object */
    PID_RF_MULTI_TYPE = 51,
    PID_RF_DOMAIN_ADDRESS = 56,
    PID_RF_RETRANSMITTER = 57,
    PID_RF_FILTERING_MODE_SUPPORT = 58,
    PID_RF_FILTERING_MODE_SELECT = 59,
    PID_RF_BIDIR_TIMEOUT = 60,
    PID_RF_DIAG_SA_FILTER_TABLE = 61,
    PID_RF_DIAG_BUDGET_TABLE = 62,
    PID_RF_DIAG_PROBE = 63,

    /** KNXnet/IP Parameter Object */
    PID_PROJECT_INSTALLATION_ID = 51,
    PID_KNX_INDIVIDUAL_ADDRESS = 52,
    PID_ADDITIONAL_INDIVIDUAL_ADDRESSES = 53,
    PID_CURRENT_IP_ASSIGNMENT_METHOD = 54,
    PID_IP_ASSIGNMENT_METHOD = 55,
    PID_IP_CAPABILITIES = 56,
    PID_CURRENT_IP_ADDRESS = 57,
    PID_CURRENT_SUBNET_MASK = 58,
    PID_CURRENT_DEFAULT_GATEWAY = 59,
    PID_IP_ADDRESS = 60,
    PID_SUBNET_MASK = 61,
    PID_DEFAULT_GATEWAY = 62,
    PID_DHCP_BOOTP_SERVER = 63,
    PID_MAC_ADDRESS = 64,
    PID_SYSTEM_SETUP_MULTICAST_ADDRESS = 65,
    PID_ROUTING_MULTICAST_ADDRESS = 66,
    PID_TTL = 67,
    PID_KNXNETIP_DEVICE_CAPABILITIES = 68,
    PID_KNXNETIP_DEVICE_STATE = 69,
    PID_KNXNETIP_ROUTING_CAPABILITIES = 70,
    PID_PRIORITY_FIFO_ENABLED = 71,
    PID_QUEUE_OVERFLOW_TO_IP = 72,
    PID_QUEUE_OVERFLOW_TO_KNX = 73,
    PID_MSG_TRANSMIT_TO_IP = 74,
    PID_MSG_TRANSMIT_TO_KNX = 75,
    PID_FRIENDLY_NAME = 76,
    PID_ROUTING_BUSY_WAIT_TIME = 78,
    PID_CUSTOM_RESERVED_TUNNELS_CTRL = 201, // custom propertiy to control the stacks behaviour for reserverd tunnels, not in Spec (PID >= 200)
    PID_CUSTOM_RESERVED_TUNNELS_IP = 202, // custom propertiy to control the stacks behaviour for reserverd tunnels, not in Spec (PID >= 200)

    /** cEMI Server Object */
    PID_MEDIUM_TYPE = 51,
    PID_COMM_MODE = 52,
    PID_MEDIUM_AVAILABILITY = 53,
    PID_ADD_INFO_TYPES = 54,
    PID_TIME_BASE = 55,
    PID_TRANSP_ENABLE = 56,
    PID_CLIENT_SNA = 57,
    PID_CLIENT_DEVICE_ADDRESS = 58,
    PID_BIBAT_NEXTBLOCK = 59,
    PID_RF_MODE_SELECT = 60,
    PID_RF_MODE_SUPPORT = 61,
    PID_RF_FILTERING_MODE_SELECT_CEMI_SERVER = 62,
    PID_RF_FILTERING_MODE_SUPPORT_CEMI_SERVER = 63,
    PID_COMM_MODES_SUPPORTED = 64,
    PID_FILTERING_MODE_SUPPORT = 65,
    PID_FILTERING_MODE_SELECT = 66,
    PID_MAX_INTERFACE_APDU_LENGTH = 68,
    PID_MAX_LOCAL_APDU_LENGTH = 69,

    /** Security Interface Object */
    PID_SECURITY_MODE = 51,                     // Enable and disable the Security Mode
    PID_P2P_KEY_TABLE = 52,                     // Security keys used for securing point-to-point and broadcast communication
    PID_GRP_KEY_TABLE = 53,                     // Security keys used for securing standard mode group communication
    PID_SECURITY_INDIVIDUAL_ADDRESS_TABLE = 54, // IAs and last valid sequence numbers of communication partners with secure links
    PID_SECURITY_FAILURES_LOG = 55,             // Provides security failure information
    PID_TOOL_KEY = 56,                          // Stores the security information for the central MaC in S-Mode and Ctrl-Mode
    PID_SECURITY_REPORT = 57,                   // KNX Data Security-related status and diagnostic information
    PID_SECURITY_REPORT_CONTROL = 58,           // Control the spontaneous communication of the security report through DMP_InterfaceObject-InfoReport_RCl
    PID_SEQUENCE_NUMBER_SENDING = 59,           // Sequence Number used for the next outgoing secure communication
    PID_ZONE_KEY_TABLE = 60,                    // Security keys used for securing zone addressing communication
    PID_GO_SECURITY_FLAGS = 61,                 // Defines the required security requirements for each group object
    PID_ROLE_TABLE = 62,                        // Role table
    PID_TOOL_SEQUENCE_NUMBER_SENDING = 250,     // Sequence Number used for the next outgoing secure communication (Tool Access only, non-standardized!)

    /** Router Object */
    PID_MEDIUM_STATUS = 51,
    PID_MAIN_LCCONFIG = 52,
    PID_SUB_LCCONFIG = 53,
    PID_MAIN_LCGRPCONFIG = 54,
    PID_SUB_LCGRPCONFIG = 55,
    PID_ROUTETABLE_CONTROL = 56,
    PID_COUPLER_SERVICES_CONTROL = 57,
    PID_MAX_APDU_LENGTH_ROUTER = 58,
    PID_L2_COUPLER_TYPE = 59,                   // Only interesting for mask 0x0912 (TP1/TP1 coupler)
    PID_HOP_COUNT = 61,                         // Only interesting in primary if other medium(secondary) is open medium without hopcount
    PID_MEDIUM = 63,
    PID_FILTER_TABLE_USE = 67,
    PID_RF_ENABLE_SBC = 112,                    // Exists only if medium for this router object is RF (PDT_FUNCTION)
    PID_IP_ENABLE_SBC = 120,                    // Exists only if medium for this router object is IP (PDT_FUNCTION)
};

enum LoadState
{
    LS_UNLOADED = 0,
    LS_LOADED = 1,
    LS_LOADING = 2,
    LS_ERROR = 3,
    LS_UNLOADING = 4,
    LS_LOADCOMPLETING = 5
};

enum LoadEvents
{
    LE_NOOP = 0,
    LE_START_LOADING = 1,
    LE_LOAD_COMPLETED = 2,
    LE_ADDITIONAL_LOAD_CONTROLS = 3,
    LE_UNLOAD = 4
};

// 20.011 DPT_ErrorClass_System 
enum ErrorCode
{
    E_NO_FAULT = 0,
    E_GENERAL_DEVICE_FAULT = 1,
    E_COMMUNICATION_FAULT = 2,
    E_CONFIGURATION_FAULT = 3,
    E_HARDWARE_FAULT = 4,
    E_SOFTWARE_FAULT = 5,
    E_INSUFFICIENT_NON_VOLATILE_MEMORY = 6,
    E_INSUFFICIENT_VOLATILE_MEMORY = 7,
    E_GOT_MEM_ALLOC_ZERO = 8,
    E_CRC_ERROR = 9,
    E_WATCHDOG_RESET = 10,
    E_INVALID_OPCODE = 11,
    E_GENERAL_PROTECTION_FAULT = 12,
    E_MAX_TABLE_LENGTH_EXEEDED = 13,
    E_GOT_UNDEF_LOAD_CMD = 14,
    E_GAT_NOT_SORTED = 15,
    E_INVALID_CONNECTION_NUMBER = 16,
    E_INVALID_GO_NUMBER = 17,
    E_GO_TYPE_TOO_BIG = 18
};

/** The access level necessary to read a property of an interface object. */
enum AccessLevel
{
    ReadLv0 = 0x00,
    ReadLv1 = 0x10,
    ReadLv2 = 0x20,
    ReadLv3 = 0x30,
    WriteLv0 = 0x00,
    WriteLv1 = 0x01,
    WriteLv2 = 0x02,
    WriteLv3 = 0x03,
};

struct PropertyDescription
{
    PropertyID Id;
    bool WriteEnable;
    PropertyDataType Type;
    uint16_t MaxElements;
    uint8_t Access;
};

class Property : public SaveRestore
{
  public:
    Property(PropertyID id, bool writeEnable, PropertyDataType type, uint16_t maxElements, uint8_t access);
    virtual ~Property();
    PropertyID Id() const;
    bool WriteEnable() const;
    PropertyDataType Type() const;
    uint16_t MaxElements() const;
    uint8_t Access() const;
    uint8_t ElementSize() const;
    virtual uint8_t read(uint16_t start, uint8_t count, uint8_t* data) const = 0;
    virtual uint8_t write(uint16_t start, uint8_t count, const uint8_t* data) = 0;
    virtual void command(uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength);
    virtual void state(uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength);
    uint8_t read(uint8_t& value) const;
    uint8_t read(uint16_t& value) const;
    uint8_t read(uint32_t& value) const;
    uint8_t read(uint8_t* value) const;
    uint8_t write(uint8_t value);
    uint8_t write(uint16_t value);
    uint8_t write(uint16_t position, uint16_t value);
    uint8_t write(uint32_t value);
    uint8_t write(const uint8_t* value);
  protected:
    PropertyID _id;
    bool _writeEnable;
    PropertyDataType _type;
    uint16_t _maxElements;
    uint8_t _access;
};
