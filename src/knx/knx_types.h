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

enum AddressType
{
    InduvidualAddress = 0,
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

enum Repetition
{
    NoRepitiion = 0,
    WasRepeated = 0,
    RepititionAllowed = 0x20,
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
    DomainAddressSerialNumberRead = 0x3ec,
    DomainAddressSerialNumberResponse = 0x3ed,
    DomainAddressSerialNumberWrite = 0x3ee,
    
    // Application Layer Services on Point-to-point Connection-Oriented Communication Mode (mandatory)
    // Application Layer Services on Point-to-point Connectionless Communication Mode (either optional or mandatory)
    MemoryRead = 0x200,
    MemoryResponse = 0x240,
    MemoryWrite = 0x280,
    UserMemoryRead = 0x2C0,
    UserMemoryResponse = 0x2C1,
    UserMemoryWrite = 0x2C2,
    UserManufacturerInfoRead = 0x2C5,
    UserManufacturerInfoResponse = 0x2C6,
    DeviceDescriptorRead = 0x300,
    DeviceDescriptorResponse = 0x340,
    Restart = 0x380,
    AuthorizeRequest = 0x3d1,
    AuthorizeResponse = 0x3d2,
    KeyWrite = 0x3d3,
    KeyResponse = 0x3d4,
    PropertyValueRead = 0x3d5,
    PropertyValueResponse = 0x3d6,
    PropertyValueWrite = 0x3d7,
    PropertyDescriptionRead = 0x3d8,
    PropertyDescriptionResponse = 0x3d9,
};
