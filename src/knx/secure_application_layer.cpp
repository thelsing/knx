#include "secure_application_layer.h"
#include "transport_layer.h"
#include "cemi_frame.h"
#include "association_table_object.h"
#include "security_interface_object.h"
#include "device_object.h"
#include "apdu.h"
#include "bau.h"
#include "string.h"
#include "bits.h"
#include "aes.hpp"
#include <stdio.h>

const uint8_t SecureDataPdu = 0;
const uint8_t SecureSyncRequest = 2;
const uint8_t SecureSyncResponse = 3;

uint64_t sequenceNumberToolAccess = 0;
uint64_t sequenceNumber = 0;

uint64_t lastValidSequenceNumberTool = 0;
uint64_t lastValidSequenceNumber = 0;


SecureApplicationLayer::SecureApplicationLayer(DeviceObject &deviceObj, SecurityInterfaceObject &secIfObj, AssociationTableObject& assocTable, BusAccessUnit& bau):
    ApplicationLayer(assocTable, bau),
    _secIfObj(secIfObj),
    _deviceObj(deviceObj)
{

}

/* from transport layer */

void SecureApplicationLayer::dataGroupIndication(HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu)
{
    if (apdu.type() == SecureService)
    {
        // Decrypt secure APDU

        // Process decrypted inner APDU
        ApplicationLayer::dataGroupIndication(hopType, priority, tsap, apdu);
    }
    else
    {
        ApplicationLayer::dataGroupIndication(hopType, priority, tsap, apdu);
    }
}

void SecureApplicationLayer::dataGroupConfirm(AckType ack, HopCountType hopType, Priority priority,  uint16_t tsap, APDU& apdu, bool status)
{
    if (apdu.type() == SecureService)
    {
        // Decrypt secure APDU

        // Process decrypted inner APDU
        ApplicationLayer::dataGroupConfirm(ack, hopType, priority, tsap, apdu, status);
    }
    else
    {
        ApplicationLayer::dataGroupConfirm(ack, hopType, priority, tsap, apdu, status);
    }
}

void SecureApplicationLayer::dataBroadcastIndication(HopCountType hopType, Priority priority, uint16_t source, APDU& apdu)
{
    if (apdu.type() == SecureService)
    {
        // Secure APDU is not allowed in Broadcast
        println("Secure APDU in Broadcast not allowed!");
    }
    else
    {
        ApplicationLayer::dataBroadcastIndication(hopType, priority, source, apdu);
    }
}

void SecureApplicationLayer::dataBroadcastConfirm(AckType ack, HopCountType hopType, Priority priority, APDU& apdu, bool status)
{
    ApplicationLayer::dataBroadcastConfirm(ack, hopType, priority, apdu, status);
}

void SecureApplicationLayer::dataSystemBroadcastIndication(HopCountType hopType, Priority priority, uint16_t source, APDU& apdu)
{
    if (apdu.type() == SecureService)
    {
        // Secure APDU is not allowed in SystemBroadcast
        println("Secure APDU in SystemBroadcast not allowed!");
    }
    else
    {
        ApplicationLayer::dataSystemBroadcastIndication(hopType, priority, source, apdu);
    }
}

void SecureApplicationLayer::dataSystemBroadcastConfirm(HopCountType hopType, Priority priority, APDU& apdu, bool status)
{
    ApplicationLayer::dataSystemBroadcastConfirm(hopType, priority, apdu, status);
}

void SecureApplicationLayer::dataIndividualIndication(HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu)
{
    if (apdu.type() == SecureService)
    {
        // Decrypt secure APDU

        // Process decrypted inner APDU
        ApplicationLayer::dataIndividualIndication(hopType, priority, tsap, apdu);
    }
    else
    {
        ApplicationLayer::dataIndividualIndication(hopType, priority, tsap, apdu);
    }
}

void SecureApplicationLayer::dataIndividualConfirm(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu, bool status)
{
    if (apdu.type() == SecureService)
    {
        // Decrypt secure APDU

        // Process decrypted inner APDU
        ApplicationLayer::dataIndividualConfirm(ack, hopType, priority, tsap, apdu, status);
    }
    else
    {
        ApplicationLayer::dataIndividualConfirm(ack, hopType, priority, tsap, apdu, status);
    }
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
    if (apdu.type() == SecureService)
    {
        // Decrypt secure APDU

        println("Secure APDU: ");
        apdu.printPDU();

        // Somehow ugly that we need to know the size in advance here at this point
        // Same length calculation is also in the decrypt() function
        uint16_t plainApduLength = apdu.length() - 1 - 6 - 4; // secureAdsuLength - sizeof(scf) - sizeof(seqNum) - sizeof(mac)
        CemiFrame plainFrame(plainApduLength);

        uint16_t srcAddress = apdu.frame().sourceAddress();
        uint16_t dstAddress = apdu.frame().destinationAddress();
        uint8_t tpci = apdu.frame().data()[TPDU_LPDU_DIFF]; // FIXME: when cEMI class is refactored, there might be additional info fields in cEMI [fixed TPDU_LPDU_DIFF]
        print("Secure Debug: TPCI: ");
        println(tpci, HEX);
        // Note:
        // The TPCI is also included in the MAC calculation to provide authenticity for this field.
        // However, a secure APDU (with a MAC) is only included in transport layer PDUs T_DATA_GROUP, T_DATA_TAG_GROUP, T_DATA_INDIVIDUAL, T_DATA_CONNECTED
        // and not in T_CONNECT, T_DISCONNECT, T_ACK, T_NACK.
        // This means the DATA/CONTROL flag is always 0(=DATA). The flag "NUMBERED" differentiates between T_DATA_INDIVIDUAL and T_DATA_CONNECTED.
        // The seqNumber is only in T_DATA_CONNECTED and 0 in case of T_DATA_GROUP and T_DATA_GROUP (leaving out T_DATA_TAG_GROUP).
        // Summary: effectively only the "NUMBERED" flag (bit6) and the SeqNumber (bit5-2) are used from transport layer.
        //          In T_DATA_* services the bits1-0 of TPCI octet are used as bits9-8 for APCI type which is fixed to 0x03. SecureService APCI is 0x03F1.

        // FIXME: when cEMI class is refactored, there might be additional info fields in cEMI (fixed APDU_LPDU_DIFF)
        if (decrypt(plainFrame.data()+APDU_LPDU_DIFF, srcAddress, dstAddress, false, tpci, apdu.data(), apdu.length()))
        {
            println("Plain APDU: ");
            plainFrame.apdu().printPDU();

            // Process decrypted inner APDU
            ApplicationLayer::dataConnectedIndication(priority, tsap, plainFrame.apdu());
        }
    }
    else
    {
        ApplicationLayer::dataConnectedIndication(priority, tsap, apdu);
    }
}

void SecureApplicationLayer::dataConnectedConfirm(uint16_t tsap)
{
    ApplicationLayer::dataConnectedConfirm(tsap);
}

/* to transport layer */

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
    ApplicationLayer::dataSystemBroadcastRequest(ack, hopType, SystemPriority, apdu);
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

uint32_t SecureApplicationLayer::calcAuthOnlyMac(uint8_t* apdu, uint8_t apduLength, const uint8_t* key, uint8_t* iv, uint8_t* ctr0)
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
                            const uint8_t* key, uint8_t* iv)
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

void SecureApplicationLayer::blockCtr0(uint8_t* buffer, uint8_t* seqNum, uint16_t indSrcAddr, uint16_t dstAddr)
{
    uint8_t* pBuf = buffer;
    pBuf = pushByteArray(seqNum, 6, pBuf);
    pBuf = pushWord(indSrcAddr, pBuf);
    pBuf = pushWord(dstAddr, pBuf);
    pBuf = pushInt(0x00000000, pBuf);
    pBuf = pushByte(0x01, pBuf);
}

void SecureApplicationLayer::sixBytes(uint64_t num, uint8_t* toByteArray)
{
    *(toByteArray + 0) = (uint16_t)(num >> 32);
    *(toByteArray + 2) = (uint32_t)(num);
}

uint64_t SecureApplicationLayer::toUInt64(uint8_t* data, uint8_t dataLen)
{
        uint64_t l = 0;

        for (uint8_t i = 0; i< dataLen; i++)
        {
            l = (l << 8) + data[i];
        }
        return l;
}

const uint8_t* SecureApplicationLayer::toolKey(uint16_t devAddr)
{
    const uint8_t* toolKey = _secIfObj.propertyData(PID_TOOL_KEY);
    return toolKey;
}

const uint8_t* SecureApplicationLayer::p2pKey(uint16_t addressIndex)
{
    if (!_secIfObj.isLoaded())
        return nullptr;

    // TODO
    return _secIfObj.propertyData(PID_P2P_KEY_TABLE);
}

const uint8_t* SecureApplicationLayer::groupKey(uint16_t addressIndex)
{
    if (!_secIfObj.isLoaded())
        return nullptr;

    // TODO
    return _secIfObj.propertyData(PID_GRP_KEY_TABLE);
}

uint16_t SecureApplicationLayer::groupAddressIndex(uint16_t groupAddr)
{
    // TODO
    return 0;
}

uint16_t SecureApplicationLayer::indAddressIndex(uint16_t indAddr)
{
    // TODO
    return 0;
}

const uint8_t* SecureApplicationLayer::securityKey(uint16_t addr, bool isGroupAddress)
{
    if (isGroupAddress)
    {
        uint16_t gaIndex = groupAddressIndex(addr);
        if (gaIndex > 0)
            return groupKey(gaIndex);
    }
    else
    {
        uint16_t iaIndex = indAddressIndex(addr);
        if (iaIndex > 0)
            return p2pKey(iaIndex);
    }

    return nullptr;
}

// returns next outgoing sequence number for secure communication
uint64_t SecureApplicationLayer::nextSequenceNumber(bool toolAccess)
{
    return toolAccess ? sequenceNumberToolAccess : sequenceNumber;
}

// stores next outgoing sequence number for secure communication
void SecureApplicationLayer::updateSequenceNumber(bool toolAccess, uint64_t seqNum)
{
    if (toolAccess)
        sequenceNumberToolAccess = seqNum;
    else
        sequenceNumber = seqNum;
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

void SecureApplicationLayer::updateLastValidSequence(bool toolAccess, uint16_t remoteAddr, uint64_t seqNo)
{
    if (toolAccess)
        // TODO
        //lastValidSequenceToolAccess.put(remoteAddr, seqNo);
        lastValidSequenceNumberTool = seqNo;
    //else
        // TODO
        //lastValidSequence.put(remoteAddr, seqNo);
}

void SecureApplicationLayer::sendSyncResponse(uint16_t dstAddr, bool dstAddrIsGroupAddr, bool toolAccess, uint16_t remoteNextSeqNum)
{
    uint64_t ourNextSeqNum = nextSequenceNumber(toolAccess);
    // asdu = ByteBuffer.allocate(12).put(sixBytes(ourNextSeq)).put(sixBytes(remoteNextSeq));
    // response = secure(SecureSyncResponse, address(), dst, asdu.array(), toolAccess, true).get();

    _lastSyncRes = millis();

    // Send encrypted SyncResponse message using T_DATA_INDIVIDUAL
    //dataIndividualRequest(AckType::AckDontCare, NetworkLayerParameter, SystemPriority, dstAddr, apdu);
}

void SecureApplicationLayer::receivedSyncRequest(uint16_t srcAddr, uint16_t dstAddr, bool dstAddrIsGroupAddr, bool toolAccess, uint8_t* seqNum, long challenge)
{
    uint64_t nextRemoteSeqNum = toUInt64(seqNum, 6);
    uint64_t nextSeqNum = 1 + lastValidSequenceNumber(toolAccess, srcAddr);

    if (nextRemoteSeqNum > nextSeqNum)
    {
        updateLastValidSequence(toolAccess, srcAddr, nextRemoteSeqNum - 1);
        nextSeqNum = nextRemoteSeqNum;
    }

    _syncReqBroadcast = (dstAddr == 0x0000) && dstAddrIsGroupAddr;

    uint16_t toAddr = _syncReqBroadcast ? dstAddr : srcAddr;
    bool toIsGroupAddress = _syncReqBroadcast;
    sendSyncResponse(toAddr, toIsGroupAddress, toolAccess, nextSeqNum);
}

void SecureApplicationLayer::receivedSyncResponse(uint16_t remoteAddr, bool toolAccess, uint8_t* plainApdu)
{
//    final var request = pendingSyncRequests.get(remote);
//    if (request == null)
//        return;

    // Bytes 0-5 in the "APDU" buffer contain the remote sequence number
    // Bytes 6-11 in the "APDU" buffer contain the local sequence number
    uint64_t remoteSeq = toUInt64(plainApdu + 0, 6);
    uint64_t localSeq = toUInt64(plainApdu + 6, 6 + 6);

    uint64_t last = lastValidSequenceNumber(toolAccess, remoteAddr);
    if (remoteSeq - 1 > last)
    {
        //logger.debug("sync.res update {} last valid {} seq -> {}", remote, toolAccess ? "tool access" : "p2p", remoteSeq -1);
        updateLastValidSequence(toolAccess, remoteAddr, remoteSeq - 1);
    }

    uint64_t next = nextSequenceNumber(toolAccess);
    if (localSeq > next) {
        //logger.debug("sync.res update local next {} seq -> {}", toolAccess ? "tool access" : "p2p", localSeq);
        updateSequenceNumber(toolAccess, localSeq);
    }

    //syncRequestCompleted(request);
}

bool SecureApplicationLayer::decrypt(uint8_t* plainApdu, uint16_t srcAddr, uint16_t dstAddr, bool dstAddrIsGroupAddr, uint8_t tpci, uint8_t* secureAsdu, uint16_t secureAdsuLength)
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

    bool syncReq = service == SecureSyncRequest;
    bool syncRes = service == SecureSyncResponse;

    const uint8_t* key = dstAddrIsGroupAddr ? securityKey(dstAddr, dstAddrIsGroupAddr) : toolAccess ? toolKey(srcAddr == _deviceObj.induvidualAddress() ? dstAddr : srcAddr) : securityKey(srcAddr, false);

    uint8_t seqNum[6];
    pBuf = popByteArray(seqNum, 6, pBuf);
    uint64_t receivedSeqNumber = toUInt64(seqNum, 6);

    // Provide array for KNX serial number if it is a SyncRequest
    // DataService and SyncResponse do not use this variable.
    uint8_t knxSerialNumber[6];

    if (service == SecureDataPdu)
    {
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
    else if(syncReq)
    {
        pBuf = popByteArray(knxSerialNumber, 6, pBuf);

        // ignore sync.reqs not addressed to us
        if (!memcmp(knxSerialNumber, _deviceObj.propertyData(PID_SERIAL_NUMBER), 6))
        {
            uint8_t emptySerialNumber[6] = {0};
            if (systemBroadcast || dstAddr != _deviceObj.induvidualAddress() || !memcmp(knxSerialNumber, emptySerialNumber, 6))
                return false;
        }

        // if responded to another request within the last 1 second, ignore
        if ((millis() - _lastSyncRes) < 1000)
        {
            return false;
        }
    }
    else if (syncRes)
    {
        // TODO
        //final var request = pendingSyncRequests.get(src);
        //if (request == null)
        //    return null;

        // in a sync.res, seq actually contains our challenge from sync.req xored with a random value
        // extract the random value and store it in seq to use it for block0 and ctr0
        //final var challengeXorRandom = BitSet.valueOf(seq);
        //final var challenge = BitSet.valueOf(sixBytes((long) request[0]));
        //challengeXorRandom.xor(challenge);
        //seq = challengeXorRandom.toByteArray();
    }

    uint16_t apduLength = secureAdsuLength - 1 - 6 - 4; // secureAdsuLength - sizeof(scf) - sizeof(seqNum) - sizeof(mac)
    pBuf = popByteArray(plainApdu, apduLength, pBuf);

    // Clear IV buffer
    uint8_t iv[16] = {0x00};
    // Create first block B0 for AES CBC MAC calculation, used as IV later
    block0(iv, seqNum, srcAddr, dstAddr, dstAddrIsGroupAddr, extendedFrameFormat, tpci | (SecureService >> 8), SecureService & 0x00FF, apduLength);

    // Clear block counter0 buffer
    uint8_t ctr0[16] = {0x00};
    // Create first block for block counter 0
    blockCtr0(ctr0, seqNum, srcAddr, dstAddr);

    uint32_t mac;
    pBuf = popInt(mac, pBuf);

    if (authOnly)
    {
        // APDU is already plain, no decryption needed

        // Only check the MAC
        uint32_t calculatedMac = calcAuthOnlyMac(plainApdu, apduLength, key, iv, ctr0);
        if (calculatedMac != mac)
        {
            // security failure
            print("security failure: calculated MAC: ");
            print(calculatedMac, HEX);
            print(" != received MAC: ");
            print(mac, HEX);

            return false;
        }

        memcpy(plainApdu, secureAsdu, apduLength);
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
        AES_init_ctx_iv(&ctx, key, ctr0);
        //AES_CTR_xcrypt_buffer(&ctx, buffer, bufLenPadded);
        AES_CTR_xcrypt_buffer(&ctx, buffer, bufLen);

        uint32_t decryptedMac;
        popInt(decryptedMac, &buffer[0]);
        popByteArray(plainApdu, apduLength, &buffer[4]); // apdu is now decrypted (overwritten)

        // Do calculations for Auth+Conf
        uint8_t associatedData[syncReq ? 7 : 1];
        associatedData[0] = scf;
        if (syncReq)
        {
            memcpy(&associatedData[1], knxSerialNumber, 6);
        }
        uint32_t calculatedMac = calcConfAuthMac(associatedData, sizeof(associatedData), plainApdu, apduLength, key, iv);
        if (calculatedMac != decryptedMac)
        {
            // security failure
            print("security failure: calculated MAC: ");
            print(calculatedMac, HEX);
            print(" != decrypted MAC: ");
            print(decryptedMac, HEX);
            return false;
        }

        // prevent a sync.req sent by us to trigger sync notification, this happens if we provide our own tool key
        // for decryption above
        if (syncReq && srcAddr != _deviceObj.induvidualAddress())
            return false;

        if (syncReq)
        {
            uint64_t challenge = toUInt64(&plainApdu[0], 6);
            receivedSyncRequest(srcAddr, dstAddr, dstAddrIsGroupAddr, toolAccess, seqNum, challenge);
        }
        else if (syncRes)
        {
            receivedSyncResponse(srcAddr, toolAccess, plainApdu);
        }
        else
        {
            if (srcAddr == _deviceObj.induvidualAddress())
            {
                //logger.trace("update next {}seq -> {}", toolAccess ? "tool access " : "", receivedSeq);
                updateSequenceNumber(toolAccess, receivedSeqNumber + 1);
            }
            else
            {
                //logger.trace("update last valid {}seq of {} -> {}", toolAccess ? "tool access " : "", src, receivedSeq);
                updateLastValidSequence(toolAccess, srcAddr, receivedSeqNumber);
            }
        }
        if (syncReq || syncRes)
            return false;
    }

    return true;
}

void SecureApplicationLayer::secure(uint8_t* buffer, uint16_t service, uint16_t srcAddr, uint16_t dstAddr, bool dstAddrIsGroupAddr,
                                     uint8_t tpci, uint8_t* apdu, uint16_t apduLength, bool toolAccess, bool confidentiality)
{
    if (toolAccess)
    {
        if (!confidentiality)
        {
            println("Error: tool access requires auth+conf security");
            return;
        }
        if (dstAddrIsGroupAddr && dstAddr != 0)
        {
            println("Error: tool access requires individual address");
            return;
        }
    }

    const uint8_t* key = toolAccess ? toolKey(_syncReqBroadcast ? _deviceObj.induvidualAddress() : dstAddr) : securityKey(dstAddr, dstAddrIsGroupAddr);
    if (key == nullptr)
    {
        return;
    }

    bool syncReq = service == SecureSyncRequest;
    bool syncRes = service == SecureSyncResponse;

    uint8_t snoLength = syncReq ? 6 : 0;
    //ByteBuffer secureApdu = ByteBuffer.allocate(3 + SeqSize + snoLength + apdu.length + MacSize);

    uint8_t tmpTpci = _transportLayer->getTPCI(dstAddr) | (SecureService >> 8);

    uint8_t* pBuf = buffer;
    pBuf = pushByte(tmpTpci, pBuf);                   // TPCI
    pBuf = pushByte(SecureService & 0x00FF, pBuf);    // APCI

    bool systemBcast = false; // Not implemented yet

    uint8_t scf;
    scf = service;
    scf |= toolAccess ? 0x80 : 0;
    scf |= confidentiality ? 0x10 : 0;
    scf |= systemBcast ? 0x8 : 0;

    pBuf = pushByte(scf, pBuf);                       // SCF

    uint64_t seqSend = nextSequenceNumber(toolAccess);
    if (seqSend == 0)
        println("0 is not a valid sequence number");

    uint8_t seq[6];
    sixBytes(seqSend, seq);
    if (!syncRes)
        pBuf = pushByteArray(seq, 6, pBuf);          // Sequence Number

    // Prepare associated data depending on service (SyncRequest, SyncResponse or just DataService)
    uint8_t associatedData[syncReq ? 7 : 1];
    associatedData[0] = scf;
    if (syncReq)
    {
        // TODO: NYI lookup serial number of target device for SBC sync.req
        uint8_t remoteSerialNo[6] = {0};

        uint8_t emptySerialNo[6] = {0};
        pBuf = pushByteArray(systemBcast ? remoteSerialNo : emptySerialNo, 6, pBuf);
        pushByteArray(_deviceObj.propertyData(PID_SERIAL_NUMBER), 6, &associatedData[1]);
    }
    else if (syncRes)
    {
        // Just for testing
        if (testSeq)
        {
            // Do not use a random number, but a well-known one
            sixBytes(0xaaaaaaaaaaaa, seq);
        }
        else
        {
            // use random number in SyncResponse
            uint64_t randomNumber = 0x000102030405; // TODO: generate random number
            sixBytes(randomNumber, seq);
        }

        // TODO: maybe implement something like std::map for pending SyncRequests?
        //final var request = pendingSyncRequests.remove(dst);
        //if (request == null)
        //    throw new KnxSecureException("sending sync.res without corresponding .req");

        // Now XOR the new random SeqNum with the challenge from the SyncRequest
        uint8_t rndXorChallenge[6];
        pushByteArray(seq, 6, rndXorChallenge);
        for (uint8_t i = 0; i < sizeof(rndXorChallenge); i++)
        {
            rndXorChallenge[i] ^= _challenge[i];
        }
        pBuf = pushByteArray(rndXorChallenge, 6, pBuf);
    }

    // For now only 0
    uint8_t extendedFrameFormat = 0;
    // Clear IV buffer
    uint8_t iv[16] = {0x00};
    // Create first block B0 for AES CBC MAC calculation, used as IV later
    block0(iv, seq, srcAddr, dstAddr, dstAddrIsGroupAddr, extendedFrameFormat, tpci | (SecureService >> 8), SecureService & 0x00FF, apduLength);

    // Clear block counter0 buffer
    uint8_t ctr0[16] = {0x00};
    // Create first block for block counter 0
    blockCtr0(ctr0, seq, srcAddr, dstAddr);

    if (confidentiality)
    {
        // Do calculations for Auth+Conf

        uint32_t mac = calcConfAuthMac(associatedData, sizeof(associatedData), apdu, apduLength, key, iv);

        uint8_t tmpBuffer[4 + apduLength];
        pushInt(mac, tmpBuffer);
        pushByteArray(apdu, apduLength, &tmpBuffer[4]);

        struct AES_ctx ctx;
        AES_init_ctx_iv(&ctx, key, ctr0);
        AES_CTR_xcrypt_buffer(&ctx, tmpBuffer, apduLength + 4); // APDU + MAC (4 bytes)

        pBuf = pushByteArray(tmpBuffer + 4, apduLength, pBuf); // Encrypted APDU
        pBuf = pushByteArray(tmpBuffer + 0, 4, pBuf);          // Encrypted MAC
    }
    else
    {
        // Do calculations for AuthOnly
        uint32_t tmpMac = calcAuthOnlyMac(apdu, apduLength, key, iv, ctr0);

        pBuf = pushByteArray(apdu, apduLength, pBuf);          // Plain APDU
        pBuf = pushInt(tmpMac, pBuf);                          // MAC
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
