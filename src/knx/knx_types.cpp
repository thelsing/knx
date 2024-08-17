#include "knx_types.h"

const string enum_name(const LCCONFIG enum_val)
{
    switch (enum_val)
    {
        case PHYS_FRAME:
            return "PHYS_FRAME(ROUT)";

        case PHYS_FRAME_UNLOCK:
            return "PHYS_FRAME_UNLOCK";

        case PHYS_FRAME_LOCK:
            return "PHYS_FRAME_LOCK";

        case PHYS_REPEAT:
            return "PHYS_REPEAT";

        case BROADCAST_LOCK:
            return "BROADCAST_LOCK";

        case BROADCAST_REPEAT:
            return "BROADCAST_REPEAT";

        case GROUP_IACK_ROUT:
            return "GROUP_IACK_ROUT";

        case PHYS_IACK:
            return "PHYS_IACK/NACK";

        case PHYS_IACK_NORMAL:
            return "PHYS_IACK_NORMAL";

        case PHYS_IACK_ALL:
            return "PHYS_IACK_ALL";
    }

    return to_string(enum_val);
}


const string enum_name(const LCGRPCONFIG enum_val)
{
    switch (enum_val)
    {
        case GROUP_6FFF:
            return "GROUP_6FFF(ROUTE)";

        case GROUP_7000:
            return "GROUP_7000(ROUTE)";

        case GROUP_REPEAT:
            return "GROUP_REPEAT";

        case GROUP_6FFFUNLOCK:
            return "GROUP_6FFFUNLOCK";

        case GROUP_6FFFLOCK:
            return "GROUP_6FFFLOCK";

        case GROUP_7000UNLOCK:
            return "GROUP_7000UNLOCK";

        case GROUP_7000LOCK:
            return "GROUP_7000LOCK";
    }

    return to_string(enum_val);
}


const string enum_name(const DptMedium enum_val)
{
    switch (enum_val)
    {
        case KNX_TP1:
            return "KNX_TP1";

        case KNX_PL110:
            return "KNX_PL110";

        case KNX_RF:
            return "KNX_RF";

        case KNX_IP:
            return "KNX_IP";
    }

    return to_string(enum_val);
}


const string enum_name(const EraseCode enum_val)
{
    switch (enum_val)
    {
        case Void:
            return "Void";

        case ConfirmedRestart:
            return "ConfirmedRestart";

        case FactoryReset:
            return "FactoryReset";

        case ResetIA:
            return "ResetIA";

        case ResetAP:
            return "ResetAP";

        case ResetParam:
            return "ResetParam";

        case ResetLinks:
            return "ResetLinks";

        case FactoryResetWithoutIA:
            return "FactoryResetWithoutIA";
    }

    return to_string(enum_val);
}


const string enum_name(const RestartType enum_val)
{
    switch (enum_val)
    {
        case BasicRestart:
            return "BasicRestart";

        case MasterReset:
            return "MasterReset";
    }

    return to_string(enum_val);
}


const string enum_name(const DataSecurity enum_val)
{
    switch (enum_val)
    {
        case None:
            return "None";

        case Auth:
            return "Auth";

        case AuthConf:
            return "AuthConf";
    }

    return to_string(enum_val);
}


const string enum_name(const ApduType enum_val)
{
    switch (enum_val)
    {
        case GroupValueRead:
            return "GroupValueRead";

        case GroupValueResponse:
            return "GroupValueResponse";

        case GroupValueWrite:
            return "GroupValueWrite";

        case IndividualAddressWrite:
            return "IndividualAddressWrite";

        case IndividualAddressRead:
            return "IndividualAddressRead";

        case IndividualAddressResponse:
            return "IndividualAddressResponse";

        case IndividualAddressSerialNumberRead:
            return "IndividualAddressSerialNumberRead";

        case IndividualAddressSerialNumberResponse:
            return "IndividualAddressSerialNumberResponse";

        case IndividualAddressSerialNumberWrite:
            return "IndividualAddressSerialNumberWrite";

        case SystemNetworkParameterRead:
            return "SystemNetworkParameterRead";

        case SystemNetworkParameterResponse:
            return "SystemNetworkParameterResponse";

        case SystemNetworkParameterWrite:
            return "SystemNetworkParameterWrite";

        case DomainAddressWrite:
            return "DomainAddressWrite";

        case DomainAddressRead:
            return "DomainAddressRead";

        case DomainAddressResponse:
            return "DomainAddressResponse";

        case DomainAddressSelectiveRead:
            return "DomainAddressSelectiveRead";

        case DomainAddressSerialNumberRead:
            return "DomainAddressSerialNumberRead";

        case DomainAddressSerialNumberResponse:
            return "DomainAddressSerialNumberResponse";

        case DomainAddressSerialNumberWrite:
            return "DomainAddressSerialNumberWrite";

        case ADCRead:
            return "ADCRead";

        case ADCResponse:
            return "ADCResponse";

        case PropertyValueExtRead:
            return "PropertyValueExtRead";

        case PropertyValueExtResponse:
            return "PropertyValueExtResponse";

        case PropertyValueExtWriteCon:
            return "PropertyValueExtWriteCon";

        case PropertyValueExtWriteConResponse:
            return "PropertyValueExtWriteConResponse";

        case PropertyValueExtWriteUnCon:
            return "PropertyValueExtWriteUnCon";

        case PropertyExtDescriptionRead:
            return "PropertyExtDescriptionRead";

        case PropertyExtDescriptionResponse:
            return "PropertyExtDescriptionResponse";

        case FunctionPropertyExtCommand:
            return "FunctionPropertyExtCommand";

        case FunctionPropertyExtState:
            return "FunctionPropertyExtState";

        case FunctionPropertyExtStateResponse:
            return "FunctionPropertyExtStateResponse";

        case MemoryExtWrite:
            return "MemoryExtWrite";

        case MemoryExtWriteResponse:
            return "MemoryExtWriteResponse";

        case MemoryExtRead:
            return "MemoryExtRead";

        case MemoryExtReadResponse:
            return "MemoryExtReadResponse";

        case MemoryRead:
            return "MemoryRead";

        case MemoryResponse:
            return "MemoryResponse";

        case MemoryWrite:
            return "MemoryWrite";

        case UserMemoryRead:
            return "UserMemoryRead";

        case UserMemoryResponse:
            return "UserMemoryResponse";

        case UserMemoryWrite:
            return "UserMemoryWrite";

        case UserManufacturerInfoRead:
            return "UserManufacturerInfoRead";

        case UserManufacturerInfoResponse:
            return "UserManufacturerInfoResponse";

        case FunctionPropertyCommand:
            return "FunctionPropertyCommand";

        case FunctionPropertyState:
            return "FunctionPropertyState";

        case FunctionPropertyStateResponse:
            return "FunctionPropertyStateResponse";

        case DeviceDescriptorRead:
            return "DeviceDescriptorRead";

        case DeviceDescriptorResponse:
            return "DeviceDescriptorResponse";

        case Restart:
            return "Restart";

        case RestartMasterReset:
            return "RestartMasterReset";

        case RoutingTableOpen:
            return "RoutingTableOpen";

        case RoutingTableRead:
            return "RoutingTableRead";

        case RoutingTableReadResponse:
            return "RoutingTableReadResponse";

        case RoutingTableWrite:
            return "RoutingTableWrite";

        case MemoryRouterWrite:
            return "MemoryRouterWrite";

        case MemoryRouterReadResponse:
            return "MemoryRouterReadResponse";

        case AuthorizeRequest:
            return "AuthorizeRequest";

        case AuthorizeResponse:
            return "AuthorizeResponse";

        case KeyWrite:
            return "KeyWrite";

        case KeyResponse:
            return "KeyResponse";

        case PropertyValueRead:
            return "PropertyValueRead";

        case PropertyValueResponse:
            return "PropertyValueResponse";

        case PropertyValueWrite:
            return "PropertyValueWrite";

        case PropertyDescriptionRead:
            return "PropertyDescriptionRead";

        case PropertyDescriptionResponse:
            return "PropertyDescriptionResponse";

        case SecureService:
            return "SecureService";
    }

    return to_string(enum_val);
}


const string enum_name(const TpduType enum_val)
{
    switch (enum_val)
    {
        case DataBroadcast:
            return "DataBroadcast";

        case DataGroup:
            return "DataGroup";

        case DataInduvidual:
            return "DataInduvidual";

        case DataConnected:
            return "DataConnected";

        case Connect:
            return "Connect";

        case Disconnect:
            return "Disconnect";

        case Ack:
            return "Ack";

        case Nack:
            return "Nack";
    }

    return to_string(enum_val);
}


const string enum_name(const HopCountType enum_val)
{
    switch (enum_val)
    {
        case UnlimitedRouting:
            return "UnlimitedRouting";

        case NetworkLayerParameter:
            return "NetworkLayerParameter";
    }

    return to_string(enum_val);
}


const string enum_name(const Confirm enum_val)
{
    switch (enum_val)
    {
        case ConfirmNoError:
            return "ConfirmNoError";

        case ConfirmError:
            return "ConfirmError";
    }

    return to_string(enum_val);
}


const string enum_name(const SystemBroadcast enum_val)
{
    switch (enum_val)
    {
        case SysBroadcast:
            return "SysBroadcast";

        case Broadcast:
            return "Broadcast";
    }

    return to_string(enum_val);
}


const string enum_name_in(Repetition enum_val)
{
    switch (enum_val)
    {
        case WasRepeated:
            return "WasRepeated";

        case WasNotRepeated:
            return "WasNotRepeated";
    }

    return to_string(enum_val);
}

const string enum_name_out(Repetition enum_val)
{
    switch (enum_val)
    {
        case NoRepetiion:
            return "NoRepetiion";

        case RepetitionAllowed:
            return "RepetitionAllowed";
    }

    return to_string(enum_val);
}

const string enum_name(const ReturnCodes enum_val)
{
    switch (enum_val)
    {
        case Success:
            return "Success";

        case SuccessWithCrc:
            return "SuccessWithCrc";

        case MemoryError:
            return "MemoryError";

        case InvalidCommand:
            return "InvalidCommand";

        case ImpossibleCommand:
            return "ImpossibleCommand";

        case ExceedsMaxApduLength:
            return "ExceedsMaxApduLength";

        case DataOverflow:
            return "DataOverflow";

        case OutOfMinRange:
            return "OutOfMinRange";

        case OutOfMaxRange:
            return "OutOfMaxRange";

        case DataVoid:
            return "DataVoid";

        case TemporarilyNotAvailable:
            return "TemporarilyNotAvailable";

        case AccessWriteOnly:
            return "AccessWriteOnly";

        case AccessReadOnly:
            return "AccessReadOnly";

        case AccessDenied:
            return "AccessDenied";

        case AddressVoid:
            return "AddressVoid";

        case DataTypeConflict:
            return "DataTypeConflict";

        case GenericError:
            return "GenericError";
    }

    return to_string(enum_val);
}


const string enum_name(const cEmiErrorCode enum_val)
{
    switch (enum_val)
    {
        case Unspecified_Error:
            return "Unspecified_Error";

        case Out_Of_Range:
            return "Out_Of_Range";

        case Out_Of_Max_Range:
            return "Out_Of_Max_Range";

        case Out_Of_Min_Range:
            return "Out_Of_Min_Range";

        case Memory_Error:
            return "Memory_Error";

        case Read_Only:
            return "Read_Only";

        case Illegal_Command:
            return "Illegal_Command";

        case Void_DP:
            return "Void_DP";

        case Type_Conflict:
            return "Type_Conflict";

        case Prop_Index_Range_Error:
            return "Prop_Index_Range_Error";

        case Value_temp_not_writeable:
            return "Value_temp_not_writeable";
    }

    return to_string(enum_val);
}


const string enum_name(const MessageCode enum_val)
{
    switch (enum_val)
    {
        case L_data_req:
            return "L_data_req";

        case L_data_con:
            return "L_data_con";

        case L_data_ind:
            return "L_data_ind";

        case M_PropRead_req:
            return "M_PropRead_req";

        case M_PropRead_con:
            return "M_PropRead_con";

        case M_PropWrite_req:
            return "M_PropWrite_req";

        case M_PropWrite_con:
            return "M_PropWrite_con";

        case M_PropInfo_ind:
            return "M_PropInfo_ind";

        case M_FuncPropCommand_req:
            return "M_FuncPropCommand_req";

        case M_FuncPropCommand_con:
            return "M_FuncPropCommand/StateRead_con";

        case M_FuncPropStateRead_req:
            return "M_FuncPropStateRead_req";

        case M_Reset_req:
            return "M_Reset_req";

        case M_Reset_ind:
            return "M_Reset_ind";
    }

    return to_string(enum_val);
}


const string enum_name(const AddressType enum_val)
{
    switch (enum_val)
    {
        case IndividualAddress:
            return "IndividualAddress";

        case GroupAddress:
            return "GroupAddress";
    }

    return to_string(enum_val);
}


const string enum_name(const TPAckType enum_val)
{
    switch (enum_val)
    {
        case AckReqNack:
            return "AckReqNack";

        case AckReqBusy:
            return "AckReqBusy";

        case AckReqAck:
            return "AckReqAck";

        case AckReqNone:
            return "AckReqNone";
    }

    return to_string(enum_val);
}


const string enum_name(const AckType enum_val)
{
    switch (enum_val)
    {
        case AckDontCare:
            return "AckDontCare";

        case AckRequested:
            return "AckRequested";
    }

    return to_string(enum_val);
}


const string enum_name(const Priority enum_val)
{
    switch (enum_val)
    {
        case LowPriority:
            return "LowPriority";

        case NormalPriority:
            return "NormalPriority";

        case UrgentPriority:
            return "UrgentPriority";

        case SystemPriority:
            return "SystemPriority";
    }

    return to_string(enum_val);
}


const string enum_name(const FrameFormat enum_val)
{
    switch (enum_val)
    {
        case ExtendedFrame:
            return "ExtendedFrame";

        case StandardFrame:
            return "StandardFrame";
    }

    return to_string(enum_val);
}
