#include "property.h"

#include "../bits.h"

#include <cstring>

namespace Knx
{
    PropertyID Property::Id() const
    {
        return _id;
    }

    bool Property::WriteEnable() const
    {
        return _writeEnable;
    }

    PropertyDataType Property::Type() const
    {
        return _type;
    }

    uint16_t Property::MaxElements() const
    {
        return _maxElements;
    }

    uint8_t Property::Access() const
    {
        return _access;
    }

    uint8_t Property::ElementSize() const
    {
        switch (_type)
        {
            case PDT_CHAR:
            case PDT_CONTROL: // is actually 10 if written, but this is always handled with a callback
            case PDT_GENERIC_01:
            case PDT_UNSIGNED_CHAR:
            case PDT_BITSET8:
            case PDT_BINARY_INFORMATION: // only 1 bit really
            case PDT_ENUM8:
            case PDT_SCALING:
                return 1;

            case PDT_GENERIC_02:
            case PDT_INT:
            case PDT_KNX_FLOAT:
            case PDT_UNSIGNED_INT:
            case PDT_VERSION:
            case PDT_BITSET16:
                return 2;

            case PDT_DATE:
            case PDT_ESCAPE:
            case PDT_FUNCTION:
            case PDT_GENERIC_03:
            case PDT_NE_FL:
            case PDT_NE_VL:
            case PDT_POLL_GROUP_SETTING:
            case PDT_TIME:
            case PDT_UTF8:
                return 3;

            case PDT_FLOAT:
            case PDT_GENERIC_04:
            case PDT_LONG:
            case PDT_UNSIGNED_LONG:
                return 4;

            case PDT_GENERIC_05:
            case PDT_SHORT_CHAR_BLOCK:
                return 5;

            case PDT_GENERIC_06:
            case PDT_ALARM_INFO:
                return 6;

            case PDT_GENERIC_07:
                return 7;

            case PDT_DATE_TIME:
            case PDT_DOUBLE:
            case PDT_GENERIC_08:
                return 8;

            case PDT_GENERIC_09:
                return 9;

            case PDT_CHAR_BLOCK:
            case PDT_GENERIC_10:
                return 10;

            case PDT_GENERIC_11:
                return 11;

            case PDT_GENERIC_12:
                return 12;

            case PDT_GENERIC_13:
                return 13;

            case PDT_GENERIC_14:
                return 14;

            case PDT_GENERIC_15:
                return 15;

            case PDT_GENERIC_16:
                return 16;

            case PDT_GENERIC_17:
                return 17;

            case PDT_GENERIC_18:
                return 18;

            case PDT_GENERIC_19:
                return 19;

            case PDT_GENERIC_20:
                return 20;

            default:
                return 0;
        }
    }

    Property::Property(PropertyID id, bool writeEnable, PropertyDataType type,
                       uint16_t maxElements, uint8_t access)
        : _id(id), _writeEnable(writeEnable), _type(type), _maxElements(maxElements), _access(access)
    {}

    Property::~Property()
    {}

    uint8_t Property::read(uint8_t& value) const
    {
        if (ElementSize() != 1)
            return 0;

        return read(1, 1, &value);
    }

    uint8_t Property::read(uint16_t& value) const
    {
        if (ElementSize() != 2)
            return 0;

        uint8_t data[2];
        uint8_t count = read(1, 1, data);

        if (count > 0)
        {
            popWord(value, data);
        }

        return count;
    }

    uint8_t Property::read(uint32_t& value) const
    {
        if (ElementSize() != 4)
            return 0;

        uint8_t data[4];
        uint8_t count = read(1, 1, data);

        if (count > 0)
        {
            popInt(value, data);
        }

        return count;
    }

    uint8_t Property::read(uint8_t* value) const
    {
        return read(1, 1, value);
    }

    uint8_t Property::write(uint8_t value)
    {
        if (ElementSize() != 1)
            return 0;

        return write(1, 1, &value);
    }

    uint8_t Property::write(uint16_t value)
    {
        if (ElementSize() != 2)
            return 0;

        uint8_t data[2];
        pushWord(value, data);
        return write(1, 1, data);
    }

    uint8_t Property::write(uint32_t value)
    {
        if (ElementSize() != 4)
            return 0;

        uint8_t data[4];
        pushInt(value, data);
        return write(1, 1, data);
    }

    uint8_t Property::write(const uint8_t* value)
    {
        return write(1, 1, value);
    }

    uint8_t Property::write(uint16_t position, uint16_t value)
    {
        if (ElementSize() != 2)
            return 0;

        uint8_t data[2];
        pushWord(value, data);
        return write(position, 1, data);
    }

    void Property::command(uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength)
    {
        (void)data;
        (void)length;
        (void)resultData;
        resultLength = 0;
    }

    void Property::state(uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength)
    {
        (void)data;
        (void)length;
        (void)resultData;
        resultLength = 0;
    }
#ifndef KNX_NO_PRINT
    const char* enum_name(const PropertyDataType enum_val)
    {
        switch (enum_val)
        {
            case PDT_CONTROL:
                return "PDT_CONTROL";

            case PDT_CHAR:
                return "PDT_CHAR";

            case PDT_UNSIGNED_CHAR:
                return "PDT_UNSIGNED_CHAR";

            case PDT_INT:
                return "PDT_INT";

            case PDT_UNSIGNED_INT:
                return "PDT_UNSIGNED_INT";

            case PDT_KNX_FLOAT:
                return "PDT_KNX_FLOAT";

            case PDT_DATE:
                return "PDT_DATE";

            case PDT_TIME:
                return "PDT_TIME";

            case PDT_LONG:
                return "PDT_LONG";

            case PDT_UNSIGNED_LONG:
                return "PDT_UNSIGNED_LONG";

            case PDT_FLOAT:
                return "PDT_FLOAT";

            case PDT_DOUBLE:
                return "PDT_DOUBLE";

            case PDT_CHAR_BLOCK:
                return "PDT_CHAR_BLOCK";

            case PDT_POLL_GROUP_SETTING:
                return "PDT_POLL_GROUP_SETTING";

            case PDT_SHORT_CHAR_BLOCK:
                return "PDT_SHORT_CHAR_BLOCK";

            case PDT_DATE_TIME:
                return "PDT_DATE_TIME";

            case PDT_VARIABLE_LENGTH:
                return "PDT_VARIABLE_LENGTH";

            case PDT_GENERIC_01:
                return "PDT_GENERIC_01";

            case PDT_GENERIC_02:
                return "PDT_GENERIC_02";

            case PDT_GENERIC_03:
                return "PDT_GENERIC_03";

            case PDT_GENERIC_04:
                return "PDT_GENERIC_04";

            case PDT_GENERIC_05:
                return "PDT_GENERIC_05";

            case PDT_GENERIC_06:
                return "PDT_GENERIC_06";

            case PDT_GENERIC_07:
                return "PDT_GENERIC_07";

            case PDT_GENERIC_08:
                return "PDT_GENERIC_08";

            case PDT_GENERIC_09:
                return "PDT_GENERIC_09";

            case PDT_GENERIC_10:
                return "PDT_GENERIC_10";

            case PDT_GENERIC_11:
                return "PDT_GENERIC_11";

            case PDT_GENERIC_12:
                return "PDT_GENERIC_12";

            case PDT_GENERIC_13:
                return "PDT_GENERIC_13";

            case PDT_GENERIC_14:
                return "PDT_GENERIC_14";

            case PDT_GENERIC_15:
                return "PDT_GENERIC_15";

            case PDT_GENERIC_16:
                return "PDT_GENERIC_16";

            case PDT_GENERIC_17:
                return "PDT_GENERIC_17";

            case PDT_GENERIC_18:
                return "PDT_GENERIC_18";

            case PDT_GENERIC_19:
                return "PDT_GENERIC_19";

            case PDT_GENERIC_20:
                return "PDT_GENERIC_20";

            case PDT_UTF8:
                return "PDT_UTF8";

            case PDT_VERSION:
                return "PDT_VERSION";

            case PDT_ALARM_INFO:
                return "PDT_ALARM_INFO";

            case PDT_BINARY_INFORMATION:
                return "PDT_BINARY_INFORMATION";

            case PDT_BITSET8:
                return "PDT_BITSET8";

            case PDT_BITSET16:
                return "PDT_BITSET16";

            case PDT_ENUM8:
                return "PDT_ENUM8";

            case PDT_SCALING:
                return "PDT_SCALING";

            case PDT_NE_VL:
                return "PDT_NE_VL";

            case PDT_NE_FL:
                return "PDT_NE_FL";

            case PDT_FUNCTION:
                return "PDT_FUNCTION";

            case PDT_ESCAPE:
                return "PDT_ESCAPE";
        }

        return "";
    }

    const char* enum_name(const PropertyID enum_val)
    {
        switch (enum_val)
        {
            case PID_OBJECT_TYPE:
                return "PID_OBJECT_TYPE";

            case PID_LOAD_STATE_CONTROL:
                return "PID_LOAD_STATE_CONTROL";

            case PID_RUN_STATE_CONTROL:
                return "PID_RUN_STATE_CONTROL";

            case PID_TABLE_REFERENCE:
                return "PID_TABLE_REFERENCE";

            case PID_SERVICE_CONTROL:
                return "PID_SERVICE_CONTROL";

            case PID_FIRMWARE_REVISION:
                return "PID_FIRMWARE_REVISION";

            case PID_SERIAL_NUMBER:
                return "PID_SERIAL_NUMBER";

            case PID_MANUFACTURER_ID:
                return "PID_MANUFACTURER_ID";

            case PID_PROG_VERSION:
                return "PID_PROG_VERSION";

            case PID_DEVICE_CONTROL:
                return "PID_DEVICE_CONTROL";

            case PID_ORDER_INFO:
                return "PID_ORDER_INFO";

            case PID_PEI_TYPE:
                return "PID_PEI_TYPE";

            case PID_PORT_CONFIGURATION:
                return "PID_PORT_CONFIGURATION";

            case PID_TABLE:
                return "PID_TABLE";

            case PID_VERSION:
                return "PID_VERSION";

            case PID_MCB_TABLE:
                return "PID_MCB_TABLE";

            case PID_ERROR_CODE:
                return "PID_ERROR_CODE";

            case PID_OBJECT_INDEX:
                return "PID_OBJECT_INDEX";

            case PID_DOWNLOAD_COUNTER:
                return "PID_DOWNLOAD_COUNTER";

            case PID_ROUTING_COUNT:
                return "PID_ROUTING_COUNT/RF_MULTI_TYPE/PROJECT_INSTALLATION_ID";

            case PID_PROG_MODE:
                return "PID_PROG_MODE/CURRENT_IP_ASSIGNMENT_METHOD";

            case PID_MAX_APDU_LENGTH:
                return "PID_MAX_APDU_LENGTH/RF_DOMAIN_ADDRESS/IP_CAPABILITIES";

            case PID_SUBNET_ADDR:
                return "PID_SUBNET_ADDR/RF_RETRANSMITTER";

            case PID_DEVICE_ADDR:
                return "PID_DEVICE_ADDR/RF_FILTERING_MODE_SUPPORT";

            case PID_IO_LIST:
                return "PID_IO_LIST/PRIORITY_FIFO_ENABLED";

            case PID_HARDWARE_TYPE:
                return "PID_HARDWARE_TYPE";

            case PID_RF_DOMAIN_ADDRESS_CEMI_SERVER:
                return "PID_RF_DOMAIN_ADDRESS_CEMI_SERVER";

            case PID_DEVICE_DESCRIPTOR:
                return "PID_DEVICE_DESCRIPTOR";

            case PID_RF_FILTERING_MODE_SELECT:
                return "PID_RF_FILTERING_MODE_SELECT";

            case PID_RF_BIDIR_TIMEOUT:
                return "PID_RF_BIDIR_TIMEOUT";

            case PID_RF_DIAG_SA_FILTER_TABLE:
                return "PID_RF_DIAG_SA_FILTER_TABLE";

            case PID_RF_DIAG_BUDGET_TABLE:
                return "PID_RF_DIAG_BUDGET_TABLE";

            case PID_RF_DIAG_PROBE:
                return "PID_RF_DIAG_PROBE";

            case PID_KNX_INDIVIDUAL_ADDRESS:
                return "PID_KNX_INDIVIDUAL_ADDRESS";

            case PID_ADDITIONAL_INDIVIDUAL_ADDRESSES:
                return "PID_ADDITIONAL_INDIVIDUAL_ADDRESSES";

            case PID_IP_ASSIGNMENT_METHOD:
                return "PID_IP_ASSIGNMENT_METHOD";

            /*
                    case PID_CURRENT_IP_ADDRESS:
                        return "PID_CURRENT_IP_ADDRESS";

                    case PID_CURRENT_SUBNET_MASK:
                        return "PID_CURRENT_SUBNET_MASK";

                    case PID_CURRENT_DEFAULT_GATEWAY:
                        return "PID_CURRENT_DEFAULT_GATEWAY";

                    case PID_IP_ADDRESS:
                        return "PID_IP_ADDRESS";

                    case PID_SUBNET_MASK:
                        return "PID_SUBNET_MASK";

                    case PID_DEFAULT_GATEWAY:
                        return "PID_DEFAULT_GATEWAY";

                    case PID_DHCP_BOOTP_SERVER:
                        return "PID_DHCP_BOOTP_SERVER";
            */
            case PID_MAC_ADDRESS:
                return "PID_MAC_ADDRESS";

            case PID_SYSTEM_SETUP_MULTICAST_ADDRESS:
                return "PID_SYSTEM_SETUP_MULTICAST_ADDRESS";

            case PID_ROUTING_MULTICAST_ADDRESS:
                return "PID_ROUTING_MULTICAST_ADDRESS";

            case PID_TTL:
                return "PID_TTL";

            case PID_KNXNETIP_DEVICE_CAPABILITIES:
                return "PID_KNXNETIP_DEVICE_CAPABILITIES";

            case PID_KNXNETIP_DEVICE_STATE:
                return "PID_KNXNETIP_DEVICE_STATE";

            case PID_KNXNETIP_ROUTING_CAPABILITIES:
                return "PID_KNXNETIP_ROUTING_CAPABILITIES";

            /*
                    case PID_PRIORITY_FIFO_ENABLED:
                        return "PID_PRIORITY_FIFO_ENABLED";
            */
            case PID_QUEUE_OVERFLOW_TO_IP:
                return "PID_QUEUE_OVERFLOW_TO_IP";

            case PID_QUEUE_OVERFLOW_TO_KNX:
                return "PID_QUEUE_OVERFLOW_TO_KNX";

            case PID_MSG_TRANSMIT_TO_IP:
                return "PID_MSG_TRANSMIT_TO_IP";

            case PID_MSG_TRANSMIT_TO_KNX:
                return "PID_MSG_TRANSMIT_TO_KNX";

            case PID_FRIENDLY_NAME:
                return "PID_FRIENDLY_NAME";

            /*
                    case PID_ROUTING_BUSY_WAIT_TIME:
                        return "PID_ROUTING_BUSY_WAIT_TIME";
            */
            case PID_CUSTOM_RESERVED_TUNNELS_CTRL:
                return "PID_CUSTOM_RESERVED_TUNNELS_CTRL";

            case PID_CUSTOM_RESERVED_TUNNELS_IP:
                return "PID_CUSTOM_RESERVED_TUNNELS_IP";

            /*
                    case PID_MEDIUM_TYPE:
                        return "PID_MEDIUM_TYPE";

                    case PID_COMM_MODE:
                        return "PID_COMM_MODE";

                    case PID_MEDIUM_AVAILABILITY:
                        return "PID_MEDIUM_AVAILABILITY";

                    case PID_ADD_INFO_TYPES:
                        return "PID_ADD_INFO_TYPES";

                    case PID_TIME_BASE:
                        return "PID_TIME_BASE";

                    case PID_TRANSP_ENABLE:
                        return "PID_TRANSP_ENABLE";

                    case PID_CLIENT_SNA:
                        return "PID_CLIENT_SNA";

                    case PID_CLIENT_DEVICE_ADDRESS:
                        return "PID_CLIENT_DEVICE_ADDRESS";

                    case PID_BIBAT_NEXTBLOCK:
                        return "PID_BIBAT_NEXTBLOCK";

                    case PID_RF_MODE_SELECT:
                        return "PID_RF_MODE_SELECT";

                    case PID_RF_MODE_SUPPORT:
                        return "PID_RF_MODE_SUPPORT";

                    case PID_RF_FILTERING_MODE_SELECT_CEMI_SERVER:
                        return "PID_RF_FILTERING_MODE_SELECT_CEMI_SERVER";

                    case PID_RF_FILTERING_MODE_SUPPORT_CEMI_SERVER:
                        return "PID_RF_FILTERING_MODE_SUPPORT_CEMI_SERVER";

                    case PID_COMM_MODES_SUPPORTED:
                        return "PID_COMM_MODES_SUPPORTED";

                    case PID_FILTERING_MODE_SUPPORT:
                        return "PID_FILTERING_MODE_SUPPORT";

                    case PID_FILTERING_MODE_SELECT:
                        return "PID_FILTERING_MODE_SELECT";

                    case PID_MAX_INTERFACE_APDU_LENGTH:
                        return "PID_MAX_INTERFACE_APDU_LENGTH";

                    case PID_MAX_LOCAL_APDU_LENGTH:
                        return "PID_MAX_LOCAL_APDU_LENGTH";

                    case PID_SECURITY_MODE:
                        return "PID_SECURITY_MODE";

                    case PID_P2P_KEY_TABLE:
                        return "PID_P2P_KEY_TABLE";

                    case PID_GRP_KEY_TABLE:
                        return "PID_GRP_KEY_TABLE";

                    case PID_SECURITY_INDIVIDUAL_ADDRESS_TABLE:
                        return "PID_SECURITY_INDIVIDUAL_ADDRESS_TABLE";

                    case PID_SECURITY_FAILURES_LOG:
                        return "PID_SECURITY_FAILURES_LOG";

                    case PID_TOOL_KEY:
                        return "PID_TOOL_KEY";

                    case PID_SECURITY_REPORT:
                        return "PID_SECURITY_REPORT";

                    case PID_SECURITY_REPORT_CONTROL:
                        return "PID_SECURITY_REPORT_CONTROL";

                    case PID_SEQUENCE_NUMBER_SENDING:
                        return "PID_SEQUENCE_NUMBER_SENDING";

                    case PID_ZONE_KEY_TABLE:
                        return "PID_ZONE_KEY_TABLE";

                    case PID_GO_SECURITY_FLAGS:
                        return "PID_GO_SECURITY_FLAGS";

                    case PID_ROLE_TABLE:
                        return "PID_ROLE_TABLE";
            */
            case PID_TOOL_SEQUENCE_NUMBER_SENDING:
                return "PID_TOOL_SEQUENCE_NUMBER_SENDING";

            /*
                    case PID_MEDIUM_STATUS:
                        return "PID_MEDIUM_STATUS";

                    case PID_MAIN_LCCONFIG:
                        return "PID_MAIN_LCCONFIG";

                    case PID_SUB_LCCONFIG:
                        return "PID_SUB_LCCONFIG";

                    case PID_MAIN_LCGRPCONFIG:
                        return "PID_MAIN_LCGRPCONFIG";

                    case PID_SUB_LCGRPCONFIG:
                        return "PID_SUB_LCGRPCONFIG";

                    case PID_ROUTETABLE_CONTROL:
                        return "PID_ROUTETABLE_CONTROL";

                    case PID_COUPLER_SERVICES_CONTROL:
                        return "PID_COUPLER_SERVICES_CONTROL";

                    case PID_MAX_APDU_LENGTH_ROUTER:
                        return "PID_MAX_APDU_LENGTH_ROUTER";

                    case PID_L2_COUPLER_TYPE:
                        return "PID_L2_COUPLER_TYPE";

                    case PID_HOP_COUNT:
                        return "PID_HOP_COUNT";

                    case PID_MEDIUM:
                        return "PID_MEDIUM";

                    case PID_FILTER_TABLE_USE:
                        return "PID_FILTER_TABLE_USE";

            */
            case PID_RF_ENABLE_SBC:
                return "PID_RF_ENABLE_SBC";

            case PID_IP_ENABLE_SBC:
                return "PID_IP_ENABLE_SBC";
        }

        return "";
    }

    const char* enum_name(const LoadState enum_val)
    {
        switch (enum_val)
        {
            case LS_UNLOADED:
                return "LS_UNLOADED";

            case LS_LOADED:
                return "LS_LOADED";

            case LS_LOADING:
                return "LS_LOADING";

            case LS_ERROR:
                return "LS_ERROR";

            case LS_UNLOADING:
                return "LS_UNLOADING";

            case LS_LOADCOMPLETING:
                return "LS_LOADCOMPLETING";
        }

        return "";
    }

    const char* enum_name(const LoadEvents enum_val)
    {
        switch (enum_val)
        {
            case LE_NOOP:
                return "LE_NOOP";

            case LE_START_LOADING:
                return "LE_START_LOADING";

            case LE_LOAD_COMPLETED:
                return "LE_LOAD_COMPLETED";

            case LE_ADDITIONAL_LOAD_CONTROLS:
                return "LE_ADDITIONAL_LOAD_CONTROLS";

            case LE_UNLOAD:
                return "LE_UNLOAD";
        }

        return "";
    }

    const char* enum_name(const ErrorCode enum_val)
    {
        switch (enum_val)
        {
            case E_NO_FAULT:
                return "E_NO_FAULT";

            case E_GENERAL_DEVICE_FAULT:
                return "E_GENERAL_DEVICE_FAULT";

            case E_COMMUNICATION_FAULT:
                return "E_COMMUNICATION_FAULT";

            case E_CONFIGURATION_FAULT:
                return "E_CONFIGURATION_FAULT";

            case E_HARDWARE_FAULT:
                return "E_HARDWARE_FAULT";

            case E_SOFTWARE_FAULT:
                return "E_SOFTWARE_FAULT";

            case E_INSUFFICIENT_NON_VOLATILE_MEMORY:
                return "E_INSUFFICIENT_NON_VOLATILE_MEMORY";

            case E_INSUFFICIENT_VOLATILE_MEMORY:
                return "E_INSUFFICIENT_VOLATILE_MEMORY";

            case E_GOT_MEM_ALLOC_ZERO:
                return "E_GOT_MEM_ALLOC_ZERO";

            case E_CRC_ERROR:
                return "E_CRC_ERROR";

            case E_WATCHDOG_RESET:
                return "E_WATCHDOG_RESET";

            case E_INVALID_OPCODE:
                return "E_INVALID_OPCODE";

            case E_GENERAL_PROTECTION_FAULT:
                return "E_GENERAL_PROTECTION_FAULT";

            case E_MAX_TABLE_LENGTH_EXEEDED:
                return "E_MAX_TABLE_LENGTH_EXEEDED";

            case E_GOT_UNDEF_LOAD_CMD:
                return "E_GOT_UNDEF_LOAD_CMD";

            case E_GAT_NOT_SORTED:
                return "E_GAT_NOT_SORTED";

            case E_INVALID_CONNECTION_NUMBER:
                return "E_INVALID_CONNECTION_NUMBER";

            case E_INVALID_GO_NUMBER:
                return "E_INVALID_GO_NUMBER";

            case E_GO_TYPE_TOO_BIG:
                return "E_GO_TYPE_TOO_BIG";
        }

        return "";
    }

    const char* enum_name(const AccessLevel enum_val)
    {
        switch (enum_val)
        {
            case ReadLv0:
                return "ReadLv0/WriteLv0";

            case ReadLv1:
                return "ReadLv1";

            case ReadLv2:
                return "ReadLv2";

            case ReadLv3:
                return "ReadLv3";

            case WriteLv1:
                return "WriteLv1";

            case WriteLv2:
                return "WriteLv2";

            case WriteLv3:
                return "WriteLv3";
        }

        return "";
    }
#endif
} // namespace Knx