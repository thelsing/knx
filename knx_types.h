#pragma once

enum FrameFormat
{
    ExtendedFrame = 0,
    StandardFrame = 0x80
};

enum Priority
{
    LowPriority = 0xC,
    NormalPriority = 0x4,
    UrgentPriority = 0x8,
    SystemPriority = 0x0
};

enum AckType
{
    AckDontCare = 0,
    AckRequested = 0x2,
};

enum AddressType
{
    InduvidualAddress = 0,
    GroupAddress = 0x80,
};

enum MessageCode
{
    L_data_ind = 0x29,
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
    UnlimitedRouting,
    NetworkLayerParameter
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
    GroupValueRead = 0x000,
    GroupValueResponse = 0x040,
    GroupValueWrite = 0x080,
    IndividualAddressWrite = 0x0c0,
    IndividualAddressRead = 0x100,
    IndividualAddressResponse = 0x140,
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
    IndividualAddressSerialNumberRead = 0x3dc,
    IndividualAddressSerialNumberResponse = 0x3dd,
    IndividualAddressSerialNumberWrite = 0x3de,
};