#include "secure_application_layer.h"
#include "transport_layer.h"
#include "cemi_frame.h"
#include "association_table_object.h"
#include "apdu.h"
#include "bau.h"
#include "string.h"
#include "bits.h"
#include "aes.hpp"
#include <stdio.h>

const uint8_t SecureDataPdu = 0;
const uint8_t SecureServiceRequest = 2;
const uint8_t SecureServiceResponse = 3;

uint8_t lastValidSequenceNumberTool = 0;

// Our FDSK
uint8_t SecureApplicationLayer::_key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };

SecureApplicationLayer::SecureApplicationLayer(AssociationTableObject& assocTable, BusAccessUnit& bau):
    ApplicationLayer(assocTable, bau)
{
}

void SecureApplicationLayer::dataGroupIndication(HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu)
{
    ApplicationLayer::dataGroupIndication(hopType, priority, tsap, apdu);
}

void SecureApplicationLayer::dataGroupConfirm(AckType ack, HopCountType hopType, Priority priority,  uint16_t tsap, APDU& apdu, bool status)
{
    ApplicationLayer::dataGroupConfirm(ack, hopType, priority, tsap, apdu, status);
}

void SecureApplicationLayer::dataBroadcastIndication(HopCountType hopType, Priority priority, uint16_t source, APDU& apdu)
{
    ApplicationLayer::dataBroadcastIndication(hopType, priority, source, apdu);
}

void SecureApplicationLayer::dataBroadcastConfirm(AckType ack, HopCountType hopType, Priority priority, APDU& apdu, bool status)
{
    ApplicationLayer::dataBroadcastConfirm(ack, hopType, priority, apdu, status);
}

void SecureApplicationLayer::dataSystemBroadcastIndication(HopCountType hopType, Priority priority, uint16_t source, APDU& apdu)
{
    ApplicationLayer::dataSystemBroadcastIndication(hopType, priority, source, apdu);
}

void SecureApplicationLayer::dataSystemBroadcastConfirm(HopCountType hopType, Priority priority, APDU& apdu, bool status)
{
    ApplicationLayer::dataSystemBroadcastConfirm(hopType, priority, apdu, status);
}

void SecureApplicationLayer::dataIndividualIndication(HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu)
{
    ApplicationLayer::dataIndividualIndication(hopType, priority, tsap, apdu);
}

void SecureApplicationLayer::dataIndividualConfirm(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu, bool status)
{
    ApplicationLayer::dataIndividualConfirm(ack, hopType, priority, tsap, apdu, status);
}

void SecureApplicationLayer::connectIndication(uint16_t tsap)
{
    ApplicationLayer::connectIndication(tsap);
}

void SecureApplicationLayer::connectConfirm(uint16_t destination, uint16_t tsap, bool status)
{
    ApplicationLayer::connectConfirm(destination, tsap, status);
}

void SecureApplicationLayer::disconnectIndication(uint16_t tsap)
{
    ApplicationLayer::disconnectIndication(tsap);
}

void SecureApplicationLayer::disconnectConfirm(Priority priority, uint16_t tsap, bool status)
{
    ApplicationLayer::disconnectConfirm(priority, tsap, status);
}

void SecureApplicationLayer::dataConnectedIndication(Priority priority, uint16_t tsap, APDU& apdu)
{
    ApplicationLayer::dataConnectedIndication(priority, tsap, apdu);
}

void SecureApplicationLayer::dataConnectedConfirm(uint16_t tsap)
{
    ApplicationLayer::dataConnectedConfirm(tsap);
}

void SecureApplicationLayer::dataGroupRequest(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu)
{
    ApplicationLayer::dataGroupRequest(ack, hopType, priority, tsap, apdu);
}
void SecureApplicationLayer::dataBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, APDU& apdu)
{
    ApplicationLayer::dataBroadcastRequest(ack, hopType, SystemPriority, apdu);
}
void SecureApplicationLayer::dataSystemBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, APDU& apdu)
{
    ApplicationLayer::dataSystemBroadcastRequest(AckDontCare, hopType, SystemPriority, apdu);
}
void SecureApplicationLayer::dataIndividualRequest(AckType ack, HopCountType hopType, Priority priority, uint16_t destination, APDU& apdu)
{
    ApplicationLayer::dataIndividualRequest(ack, hopType, priority, destination, apdu);
}
void SecureApplicationLayer::dataConnectedRequest(uint16_t tsap, Priority priority, APDU& apdu)
{
    // apdu must be valid until it was confirmed
    ApplicationLayer::dataConnectedRequest(tsap, priority, apdu);
}

class TpTelegram
{
public:
    TpTelegram()
    {

    }

    ~TpTelegram()
    {
        if (_data)
            delete[] _data;
    }

    void parseByteArray(uint8_t *buf)
    {
        _ctrlField = buf[0];
        _ctrlFieldExt = buf[1];
        _srcAddr = buf[2] << 8 | buf[3];
        _dstAddr = buf[4] << 8 | buf[5];
        _dataLen = buf[6];

        // Copy starting from TPCI octet
        _dataLen += 1;
        _data = new uint8_t (_dataLen);
        memcpy(_data, &buf[7], _dataLen);
    }

    uint16_t SrcAddr()
    {
        return _srcAddr;
    }

    uint16_t DstAddr()
    {
        return _dstAddr;
    }

    uint8_t Tpci()
    {
        uint8_t tpci;

        tpci = (_data[0] & 0xFC) >> 2;

        return tpci;
    }

    uint16_t Apci()
    {
        uint16_t apci;

        if (_dataLen > 1)
        {
            apci = (_data[0] & 0x03) << 8 | _data[1];
        }
        else
        {
            apci = (_data[0] & 0x03);
        }

        return apci;
    }

    uint8_t* Apdu()
    {
        return _data;
    }

    uint16_t ApduLen()
    {
        return _dataLen;
    }

    uint8_t* Asdu()
    {
        return _data + 2;
    }

    uint16_t AsduLen()
    {
        return _dataLen - 2;
    }

    bool isSecureTelegram()
    {
        return Apci() == SecureService;
    }

private:
    uint8_t _ctrlField;
    uint8_t _ctrlFieldExt;
    uint16_t _srcAddr;
    uint16_t _dstAddr;
    uint8_t _dataLen;
    uint8_t* _data;

};

uint32_t SecureApplicationLayer::calcAuthOnlyMac(uint8_t* apdu, uint8_t apduLength, uint8_t* key, uint8_t* iv, uint8_t* ctr0)
{
    uint16_t bufLen = 2 + apduLength; // 2 bytes for the length field (uint16_t)
    // AES-128 operates on blocks of 16 bytes, add padding
    uint16_t bufLenPadded = (bufLen + 15) / 16 * 16;
    uint8_t buffer[bufLenPadded];
    // Make sure to have zeroes everywhere, because of the padding
    memset(buffer, 0x00, bufLenPadded);

    uint8_t* pBuf = buffer;

    pBuf = pushWord(apduLength, pBuf);
    pBuf = pushByteArray(apdu, apduLength, pBuf);

    // Use zeroes as IV for first round
    uint8_t zeroIv[16] = {0x00};

    struct AES_ctx ctx1;
    AES_init_ctx_iv(&ctx1, key, zeroIv);
    // Now encrypt first block B0
    AES_CBC_encrypt_buffer(&ctx1, iv, 16);
    // Encrypt remaining buffer
    AES_CBC_encrypt_buffer(&ctx1, buffer, bufLenPadded);

    struct AES_ctx ctx2;
    AES_init_ctx_iv(&ctx2, key, ctr0);
    AES_CTR_xcrypt_buffer(&ctx2, buffer, 4); // 4 bytes only for the MAC

    uint32_t mac;
    popInt(mac, &buffer[0]);

    return mac;
}

uint32_t SecureApplicationLayer::calcConfAuthMac(uint8_t* associatedData, uint16_t associatedDataLength,
                            uint8_t* apdu, uint8_t apduLength,
                            uint8_t* key, uint8_t* iv)
{
    uint16_t bufLen = 2 + associatedDataLength + apduLength; // 2 bytes for the length field (uint16_t)
    // AES-128 operates on blocks of 16 bytes, add padding
    uint16_t bufLenPadded = (bufLen + 15) / 16 * 16;
    uint8_t buffer[bufLenPadded];
    // Make sure to have zeroes everywhere, because of the padding
    memset(buffer, 0x00, bufLenPadded);

    uint8_t* pBuf = buffer;

    pBuf = pushWord(associatedDataLength, pBuf);
    pBuf = pushByteArray(associatedData, associatedDataLength, pBuf);
    pBuf = pushByteArray(apdu, apduLength, pBuf);

    // Use zeroes as IV for first round
    uint8_t zeroIv[16] = {0x00};

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, zeroIv);
    // Now encrypt first block B0
    AES_CBC_encrypt_buffer(&ctx, iv, 16);
    // Encrypt remaining buffer
    AES_CBC_encrypt_buffer(&ctx, buffer, bufLenPadded);

    uint32_t mac;
    popInt(mac, &buffer[bufLenPadded - 16]); // bufLenPadded has a guaranteed minimum size of 16 bytes

    return mac;
}

void SecureApplicationLayer::block0(uint8_t* buffer, uint8_t* seqNum, uint16_t indSrcAddr, uint16_t dstAddr, bool dstAddrIsGroupAddr, uint8_t extFrameFormat, uint8_t tpci, uint8_t apci, uint8_t payloadLength)
{
    uint8_t* pBuf = buffer;
    pBuf = pushByteArray(seqNum, 6, pBuf);
    pBuf = pushWord(indSrcAddr, pBuf);
    pBuf = pushWord(dstAddr, pBuf);
    pBuf = pushByte(0x00, pBuf); // FT: frametype
    pBuf = pushByte( (dstAddrIsGroupAddr ? 0x80 : 0x00) | (extFrameFormat & 0xf), pBuf); // AT: address type
    pBuf = pushByte(tpci, pBuf); // TPCI
    pBuf = pushByte(apci, pBuf); // APCI // draft spec shows something different!
    pBuf = pushByte(0x00, pBuf); // Reserved: fixed 0x00 (really?)
    pBuf = pushByte(payloadLength, pBuf); // Payload length
}

void SecureApplicationLayer::blockCtr0(uint8_t* buffer, uint8_t* seqNum, uint16_t indSrcAddr, uint16_t dstAddr, bool dstAddrIsGroupAddr)
{
    uint8_t* pBuf = buffer;
    pBuf = pushByteArray(seqNum, 6, pBuf);
    pBuf = pushWord(indSrcAddr, pBuf);
    pBuf = pushWord(dstAddr, pBuf);
    pBuf = pushInt(0x00000000, pBuf);
    pBuf = pushByte(0x01, pBuf);
}

uint64_t SecureApplicationLayer::lastValidSequenceNumber(bool toolAcces, uint16_t srcAddr)
{
    if (toolAcces)
    {
        return lastValidSequenceNumberTool;
    }

    // TODO
    return 0;
}

bool SecureApplicationLayer::decrypt(uint8_t* plainApdu, uint16_t srcAddr, uint16_t dstAddr, uint8_t tpci, uint8_t* secureAsdu, uint16_t secureAdsuLength)
{
    uint8_t extendedFrameFormat = 0;

    const uint8_t* pBuf;
    uint8_t scf;

    pBuf = popByte(scf, secureAsdu);

        bool toolAccess = ((scf & 0x80) == 0x80);
        bool systemBroadcast = ((scf & 0x08) == 0x08);
        uint8_t sai = (scf >> 4) & 0x07; // sai can only be 0x0 (CCM auth only) or 0x1 (CCM with auth+conf), other values are reserved
        bool authOnly = ( sai == 0);
        uint8_t service = (scf & 0x07); // only 0x0 (S-A_Data-PDU), 0x2 (S-A_Sync_Req-PDU) or 0x3 (S-A_Sync_Rsp-PDU) are valid values

    uint8_t seqNum[6];
    pBuf = popByteArray(seqNum, 6, pBuf);

    if (service == SecureDataPdu)
    {
        uint64_t receivedSeqNumber = ((uint64_t)seqNum[0] << 40) | ((uint64_t)seqNum[1] << 32) | ((uint64_t)seqNum[2] << 24) |
                                     ((uint64_t)seqNum[3] << 16) | ((uint64_t)seqNum[4] << 8) | (uint64_t)seqNum[5];
        uint64_t expectedSeqNumber = lastValidSequenceNumber(toolAccess, srcAddr) + 1;

        if (receivedSeqNumber < expectedSeqNumber)
        {
            // security failure
            print("security failure: received seqNum: ");
            print(receivedSeqNumber, HEX);
            print(" < expected seqNum: ");
            print(expectedSeqNumber, HEX);
            return false;
        }
    }

    uint16_t apduLength = secureAdsuLength - 1 - 6 - 4; // secureAdsuLength - sizeof(scf) - sizeof(seqNum) - sizeof(mac)
    pBuf = popByteArray(plainApdu, apduLength, pBuf);

    // Clear IV buffer
    uint8_t iv[16] = {0x00};
    // Create first block B0 for AES CBC MAC calculation, used as IV later
    block0(iv, seqNum, srcAddr, dstAddr, false, extendedFrameFormat, tpci | (SecureService >> 8), SecureService & 0x00FF, apduLength);

    // Clear block counter0 buffer
    uint8_t ctr0[16] = {0x00};
    // Create first block for block counter 0
    blockCtr0(ctr0, seqNum, srcAddr, dstAddr, false);

    uint32_t mac;
    pBuf = popInt(mac, pBuf);

    if (authOnly)
    {
        // APDU is already plain, no decryption needed

        // Only check the MAC
        uint32_t calculatedMac = calcAuthOnlyMac(plainApdu, apduLength, _key, iv, ctr0);
        if (calculatedMac != mac)
        {
            // security failure
            print("security failure: calculated MAC: ");
            print(calculatedMac, HEX);
            print(" != received MAC: ");
            print(mac, HEX);

            return false;
        }
    }
    else
    {
        // APDU is encrypted and needs decryption

        uint16_t bufLen = 4 + apduLength;
        // AES-128 operates on blocks of 16 bytes, add padding
        //uint16_t bufLenPadded = (bufLen + 15) / 16 * 16;
        //uint8_t buffer[bufLenPadded];
        uint8_t buffer[bufLen];
        // Make sure to have zeroes everywhere, because of the padding
        //memset(buffer, 0x00, bufLenPadded);

        pushInt(mac, &buffer[0]);
        pushByteArray(plainApdu, apduLength, &buffer[4]); // apdu is still encrypted

        struct AES_ctx ctx;
        AES_init_ctx_iv(&ctx, _key, ctr0);
        //AES_CTR_xcrypt_buffer(&ctx, buffer, bufLenPadded);
        AES_CTR_xcrypt_buffer(&ctx, buffer, bufLen);

        uint32_t decryptedMac;
        popInt(decryptedMac, &buffer[0]);
        popByteArray(plainApdu, apduLength, &buffer[4]); // apdu is now decrypted (overwritten)

        // Do calculations for Auth+Conf
        uint8_t associatedData[1] = {scf};
        uint32_t calculatedMac = calcConfAuthMac(associatedData, sizeof(associatedData), plainApdu, apduLength, _key, iv);
        if (calculatedMac != decryptedMac)
        {
            // security failure
            print("security failure: calculated MAC: ");
            print(calculatedMac, HEX);
            print(" != decrypted MAC: ");
            print(decryptedMac, HEX);
            return false;
        }
    }

    return true;
}

/*
void SecureApplicationLayer::test_datasecure_decrypt()
{
    TpTelegram t;
    t.parseByteArray(secureTelegram);

    if (t.isSecureTelegram())
    {
        uint16_t apduLength = t.AsduLen() - 1 - 6 - 4; // secureAdsuLength - sizeof(scf) - sizeof(seqNum) - sizeof(mac)
        uint8_t apdu[apduLength];

        if (decrypt(apdu, t.SrcAddr(), t.DstAddr(), t.Tpci(), t.Asdu(), t.AsduLen()))
        {
            std::cout << "Plain APDU: ";
            for (uint8_t i = 0; i< apduLength; i++)
            {
                std::cout << std::hex << static_cast<unsigned int>(apdu[i]) << " ";
            }
            std::cout << std::endl;
        }
    }
    else
    {
        std::cout << "Telegram is not secured!" << std::endl;
    }
}
*/
void SecureApplicationLayer::encrypt(uint8_t* buffer, uint16_t srcAddr, uint16_t dstAddr, uint8_t tpci, uint8_t* apdu, uint16_t apduLength)
{
    uint8_t scf = 0x90;
    uint8_t seqNum[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x04};
    bool authOnly = false;
    uint8_t extendedFrameFormat = 0;

    // Clear IV buffer
    uint8_t iv[16] = {0x00};
    // Create first block B0 for AES CBC MAC calculation, used as IV later
    block0(iv, seqNum, srcAddr, dstAddr, false, extendedFrameFormat, tpci | (SecureService >> 8), SecureService & 0x00FF, apduLength);

    // Clear block counter0 buffer
    uint8_t ctr0[16] = {0x00};
    // Create first block for block counter 0
    blockCtr0(ctr0, seqNum, srcAddr, dstAddr, false);

    if (authOnly)
    {
        // Do calculations for AuthOnly
        uint32_t tmpMac = calcAuthOnlyMac(apdu, apduLength, _key, iv, ctr0);
    }
    else
    {
        // Do calculations for Auth+Conf
        uint8_t associatedData[1] = {scf};
        uint32_t mac = calcConfAuthMac(associatedData, sizeof(associatedData), apdu, apduLength, _key, iv);

        pushInt(mac, buffer);
        pushByteArray(apdu, apduLength, &buffer[4]);

        struct AES_ctx ctx;
        AES_init_ctx_iv(&ctx, _key, ctr0);
        AES_CTR_xcrypt_buffer(&ctx, buffer, apduLength + 4); // APDU + MAC (4 bytes)
    }
}

/*
void SecureApplicationLayer::test_datasecure_encrypt()
{
    TpTelegram t;
    t.parseByteArray(plainTelegram);

    if (!t.isSecureTelegram())
    {
        uint16_t bufLen = 4 + t.ApduLen();
        // AES-128 operates on blocks of 16 bytes, add padding
        //uint16_t bufLenPadded = (bufLen + 15) / 16 * 16;
        //uint8_t buffer[bufLenPadded];
        uint8_t buffer[bufLen];
        // Make sure to have zeroes everywhere, because of the padding
        //memset(buffer, 0x00, bufLenPadded);

        encrypt(buffer, t.SrcAddr(), t.DstAddr(), t.Tpci(), t.Apdu(), t.ApduLen());

        std::cout << "Secure Data: ";
        for (uint8_t i = 0; i< t.ApduLen(); i++)
        {
            std::cout << std::hex << static_cast<unsigned int>(buffer[4+i]) << " ";
        }
        std::cout << std::endl;

        uint32_t mac;
        popInt(mac, &buffer[0]);

        std::cout << std::hex << "MAC: " << mac << std::endl;
    }
    else
    {
        std::cout << "Telegram is secured!" << std::endl;
    }
}
*/
