#pragma once

enum FrameFormat
{
    ExtendedFrame = 0,
    StandardFrame = 0x80
};

enum Priority
{
    LowPriority = 0xC,    //!< Normal priority of group communication.
    NormalPriority = 0x4, //!< More important telegrams like central functions
    UrgentPriority = 0x8, //!< Used for alarms.
    SystemPriority = 0x0  //!< Mainly used by ETS for device programming.
};

enum AckType
{
    AckDontCare = 0,    //!< We don't care about DataLinkLayer acknowledgement.
    AckRequested = 0x2, //!< We want a DataLinkLayer acknowledgement.
};

enum TPAckType
{
    // see U_ACK_REQ defines in tpuart_data_link_layer.cpp
    AckReqNack = 0x04,
    AckReqBusy = 0x02,
    AckReqAck = 0x01,
    AckReqNone = 0x0,
};

enum AddressType
{
    IndividualAddress = 0,
    GroupAddress = 0x80,
};

enum MessageCode
{
    // L_Data services
    L_data_req = 0x11,
    L_data_con = 0x2E,
    L_data_ind = 0x29,

    // Data Properties
    M_PropRead_req = 0xFC,
    M_PropRead_con = 0xFB,
    M_PropWrite_req = 0xF6,
    M_PropWrite_con = 0xF5,
    M_PropInfo_ind = 0xF7,

    // Function Properties
     M_FuncPropCommand_req = 0xF8,
     M_FuncPropCommand_con = 0xFA,
     M_FuncPropStateRead_req = 0xF9,
     M_FuncPropStateRead_con = 0xFA, // same as M_FuncPropStateRead_con (see 3/6/3 p.105)

     // Further cEMI servies
     M_Reset_req = 0xF1,
     M_Reset_ind = 0xF0,
};

enum cEmiErrorCode
{
    Unspecified_Error = 0x00, // unknown error (R/W)
    Out_Of_Range = 0x01,      // write value not allowed (general, if not error 2 or 3) (W)
    Out_Of_Max_Range = 0x02,  // write value to high (W)
    Out_Of_Min_Range = 0x03,  // write value to low (W)
    Memory_Error = 0x04,      // memory can not be written or only with fault(s) (W)
    Read_Only = 0x05,         // write access to a ‘read only’ or a write protected Property (W)
    Illegal_Command = 0x06,   // COMMAND not valid or not supported (W)
    Void_DP = 0x07,           // read or write access to an non existing Property (R/W)
    Type_Conflict = 0x08,     // write access with a wrong data type (Datapoint length) (W)
    Prop_Index_Range_Error = 0x09,   //  read or write access to a non existing Property array index  (R/W)
    Value_temp_not_writeable = 0x0A, // The Property exists but can at this moment not be written with a new value (W)
};

// Unified return codes for KNX services and functions
// Note, that several older KNX services and functions do not use these return codes.
enum ReturnCodes
{
    // Generic positive return codes
    Success = 0x00,                 // service, function or command executed sucessfully
    SuccessWithCrc = 0x01,          // positive message confirmation, CRC over original data
    // Generic negative return codes
    MemoryError = 0xF1,             // memory cannot be accessed or only with fault(s)
    InvalidCommand = 0xF2,          // server does not support the requested command. ets: also non-existing or protected resource
    ImpossibleCommand = 0xF3,       // command cannot be executed because a dependency is not fulfilled
    ExceedsMaxApduLength  = 0xF4,   // data will not fit into a frame supported by this server
    DataOverflow  = 0xF5,           // attempt to write data beyond what is reserved for the addressed resource
    OutOfMinRange = 0xF6,           // write value below minimum supported value
    OutOfMaxRange = 0xF7,           // write value exceeds maximum supported value
    DataVoid = 0xF8,                // request contains invalid data
    TemporarilyNotAvailable = 0xF9, // data access not possible at this time
    AccessWriteOnly = 0xFA,         // read access to write-only resource
    AccessReadOnly = 0xFB,          // write access to read-only resource
    AccessDenied = 0xFC,            // access to recource is not allowed because of authorization/security
    AddressVoid = 0xFD,             // resource is not present, address does not exist
    DataTypeConflict = 0xFE,        // write access with wrong datatype (datapoint length)
    GenericError = 0xFF             // service, function or command failed
};

enum Repetition
{
    NoRepitiion = 0,
    WasRepeated = 0,
    RepetitionAllowed = 0x20,
    WasNotRepeated = 0x20,
};

enum SystemBroadcast
{
    SysBroadcast = 0,
    Broadcast = 0x10,
};

enum Confirm
{
    ConfirmNoError = 0,
    ConfirmError = 1,
};

enum HopCountType
{
    UnlimitedRouting,     //!< NPDU::hopCount is set to 7. This means that the frame never expires. This could be a problem if your bus contains a circle.
    NetworkLayerParameter //!< use NetworkLayer::hopCount as NPDU::hopCount
};

enum TpduType
{
    DataBroadcast,
    DataGroup,
    DataInduvidual,
    DataConnected,
    Connect,
    Disconnect,
    Ack,
    Nack,
};

enum ApduType
{
    // Application Layer services on Multicast Communication Mode 
    GroupValueRead = 0x000,
    GroupValueResponse = 0x040,
    GroupValueWrite = 0x080,

    // Application Layer services on Broadcast Communication Mode
    IndividualAddressWrite = 0x0c0,
    IndividualAddressRead = 0x100,
    IndividualAddressResponse = 0x140,
    IndividualAddressSerialNumberRead = 0x3dc,
    IndividualAddressSerialNumberResponse = 0x3dd,
    IndividualAddressSerialNumberWrite = 0x3de,

    // Application Layer Services on System Broadcast communication mode
    SystemNetworkParameterRead = 0x1c8,
    SystemNetworkParameterResponse = 0x1c9,
    SystemNetworkParameterWrite = 0x1ca,
    // Open media specific Application Layer Services on System Broadcast communication mode
    DomainAddressWrite = 0x3e0,
    DomainAddressRead = 0x3e1,
    DomainAddressResponse = 0x3e2,
    DomainAddressSelectiveRead = 0x3e3,
    DomainAddressSerialNumberRead = 0x3ec,
    DomainAddressSerialNumberResponse = 0x3ed,
    DomainAddressSerialNumberWrite = 0x3ee,
    
    // Application Layer Services on Point-to-point Connection-Oriented Communication Mode (mandatory)
    // Application Layer Services on Point-to-point Connectionless Communication Mode (either optional or mandatory)
    ADCRead = 0x0180,
    ADCResponse = 0x01C0,
    PropertyValueExtRead = 0x1CC,
    PropertyValueExtResponse = 0x1CD,
    PropertyValueExtWriteCon = 0x1CE,
    PropertyValueExtWriteConResponse = 0x1CF,
    PropertyValueExtWriteUnCon = 0x1D0,
    PropertyExtDescriptionRead = 0x1D2,
    PropertyExtDescriptionResponse = 0x1D3,
    FunctionPropertyExtCommand = 0x1D4,
    FunctionPropertyExtState = 0x1D5,
    FunctionPropertyExtStateResponse = 0x1D6,
    MemoryExtWrite = 0x1FB,
    MemoryExtWriteResponse = 0x1FC,
    MemoryExtRead = 0x1FD,
    MemoryExtReadResponse = 0x1FE,
    MemoryRead = 0x200,
    MemoryResponse = 0x240,
    MemoryWrite = 0x280,
    UserMemoryRead = 0x2C0,
    UserMemoryResponse = 0x2C1,
    UserMemoryWrite = 0x2C2,
    UserManufacturerInfoRead = 0x2C5,
    UserManufacturerInfoResponse = 0x2C6,
    FunctionPropertyCommand = 0x2C7,
    FunctionPropertyState = 0x2C8,
    FunctionPropertyStateResponse = 0x2C9,
    DeviceDescriptorRead = 0x300,
    DeviceDescriptorResponse = 0x340,
    Restart = 0x380,
    RestartMasterReset = 0x381,
    RoutingTableOpen = 0x3C0,
    RoutingTableRead = 0x3C1,
    RoutingTableReadResponse = 0x3C2,
    RoutingTableWrite = 0x3C3,
    MemoryRouterWrite = 0x3CA,
    MemoryRouterReadResponse = 0x3C9,
    AuthorizeRequest = 0x3d1,
    AuthorizeResponse = 0x3d2,
    KeyWrite = 0x3d3,
    KeyResponse = 0x3d4,
    PropertyValueRead = 0x3d5,
    PropertyValueResponse = 0x3d6,
    PropertyValueWrite = 0x3d7,
    PropertyDescriptionRead = 0x3d8,
    PropertyDescriptionResponse = 0x3d9,

    // Secure Service
    SecureService = 0x3F1
};

enum DataSecurity
{
    None,
    Auth,
    AuthConf
};

struct SecurityControl
{
    bool toolAccess;
    DataSecurity dataSecurity;
};

enum RestartType
{
    BasicRestart = 0x0,
    MasterReset = 0x1
};

enum EraseCode
{
    Void = 0x00,
    ConfirmedRestart = 0x01,
    FactoryReset = 0x02,
    ResetIA = 0x03,
    ResetAP = 0x04,
    ResetParam = 0x05,
    ResetLinks = 0x06,
    FactoryResetWithoutIA = 0x07
};

enum DptMedium
{
    // DPT_Medium (20.1004), range 0-255
    // All other values are reserved.
    KNX_TP1 = 0x00,
    KNX_PL110 = 0x01,
    KNX_RF = 0x02,
    KNX_IP = 0x05
};

enum LCGRPCONFIG
{
    GROUP_6FFF =        0b00000011,
    GROUP_7000 =        0b00001100,
    GROUP_REPEAT =      0b00010000,
    GROUP_6FFFUNLOCK =  0b00000001,
    GROUP_6FFFLOCK =    0b00000010,
    GROUP_6FFFROUTE =   0b00000011,
    GROUP_7000UNLOCK =  0b00000100,
    GROUP_7000LOCK =    0b00001000,
    GROUP_7000ROUTE =   0b00001100
};

enum LCCONFIG
{
    PHYS_FRAME =        0b00000011,
    PHYS_FRAME_UNLOCK = 0b00000001,
    PHYS_FRAME_LOCK =   0b00000010,
    PHYS_FRAME_ROUT =   0b00000011,
    PHYS_REPEAT =       0b00000100,
    BROADCAST_LOCK =    0b00001000,
    BROADCAST_REPEAT =  0b00010000,
    GROUP_IACK_ROUT =   0b00100000,
    PHYS_IACK =         0b11000000,
    PHYS_IACK_NORMAL =  0b01000000,
    PHYS_IACK_ALL =     0b10000000,
    PHYS_IACK_NACK =    0b11000000
};