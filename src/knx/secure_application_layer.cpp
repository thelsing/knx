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

// Select what cipher modes to include. We need AES128-CBC and AES128-CTR modes.
#define CBC 1
#define CTR 1
#define ECB 0
#include "aes.hpp"

const uint8_t SecureDataPdu = 0;
const uint8_t SecureSyncRequest = 2;
const uint8_t SecureSyncResponse = 3;

uint64_t sequenceNumberToolAccess = 50;
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
    println("dataGroupIndication");

    if (apdu.type() == SecureService)
    {
        // Decrypt secure APDU
        // Somehow ugly that we need to know the size in advance here at this point
        uint16_t plainApduLength = apdu.length() - 1 - 6 - 4; // secureAdsuLength - sizeof(scf) - sizeof(seqNum) - sizeof(mac)
        CemiFrame plainFrame(plainApduLength);
        // Decrypt secure APDU
        if (decodeSecureApdu(apdu, plainFrame.apdu()))
        {
            // Process decrypted inner APDU
            ApplicationLayer::dataGroupIndication(hopType, priority, tsap, plainFrame.apdu());
        }
        return;
    }

    ApplicationLayer::dataGroupIndication(hopType, priority, tsap, apdu);
}

void SecureApplicationLayer::dataGroupConfirm(AckType ack, HopCountType hopType, Priority priority,  uint16_t tsap, APDU& apdu, bool status)
{
    println("dataGroupConfirm");

    if (apdu.type() == SecureService)
    {
        // Decrypt secure APDU
        // Somehow ugly that we need to know the size in advance here at this point
        uint16_t plainApduLength = apdu.length() - 1 - 6 - 4; // secureAdsuLength - sizeof(scf) - sizeof(seqNum) - sizeof(mac)
        CemiFrame plainFrame(plainApduLength);
        // Decrypt secure APDU
        if (decodeSecureApdu(apdu, plainFrame.apdu()))
        {
            // Process decrypted inner APDU
            ApplicationLayer::dataGroupConfirm(ack, hopType, priority, tsap, plainFrame.apdu(), status);
        }
        return;
    }

    ApplicationLayer::dataGroupConfirm(ack, hopType, priority, tsap, apdu, status);
}

void SecureApplicationLayer::dataBroadcastIndication(HopCountType hopType, Priority priority, uint16_t source, APDU& apdu)
{
    println("dataBroadcastIndication");

    if (apdu.type() == SecureService)
    {
        // Decrypt secure APDU
        // Somehow ugly that we need to know the size in advance here at this point
        uint16_t plainApduLength = apdu.length() - 1 - 6 - 4; // secureAdsuLength - sizeof(scf) - sizeof(seqNum) - sizeof(mac)
        CemiFrame plainFrame(plainApduLength);
        // Decrypt secure APDU
        if (decodeSecureApdu(apdu, plainFrame.apdu()))
        {
            // Process decrypted inner APDU
            ApplicationLayer::dataBroadcastIndication(hopType, priority, source, plainFrame.apdu());
        }
        return;
    }

    ApplicationLayer::dataBroadcastIndication(hopType, priority, source, apdu);
}

void SecureApplicationLayer::dataBroadcastConfirm(AckType ack, HopCountType hopType, Priority priority, APDU& apdu, bool status)
{
    println("dataBroadcastConfirm");

    if (apdu.type() == SecureService)
    {
/*
        // Decrypt secure APDU
        // Somehow ugly that we need to know the size in advance here at this point
        uint16_t plainApduLength = apdu.length() - 1 - 6 - 4; // secureAdsuLength - sizeof(scf) - sizeof(seqNum) - sizeof(mac)
        CemiFrame plainFrame(plainApduLength);
        // Decrypt secure APDU
        if (decodeSecureApdu(apdu, plainFrame.apdu()))
        {
            // Process decrypted inner APDU
            ApplicationLayer::dataBroadcastConfirm(ack, hopType, priority, plainFrame.apdu(), status);
        }
*/
        return;
    }
    ApplicationLayer::dataBroadcastConfirm(ack, hopType, priority, apdu, status);
}

void SecureApplicationLayer::dataSystemBroadcastIndication(HopCountType hopType, Priority priority, uint16_t source, APDU& apdu)
{
    println("dataSystemBroadcastIndication");

    if (apdu.type() == SecureService)
    {
        // Decrypt secure APDU
        // Somehow ugly that we need to know the size in advance here at this point
        uint16_t plainApduLength = apdu.length() - 1 - 6 - 4; // secureAdsuLength - sizeof(scf) - sizeof(seqNum) - sizeof(mac)
        CemiFrame plainFrame(plainApduLength);
        // Decrypt secure APDU
        if (decodeSecureApdu(apdu, plainFrame.apdu()))
        {
            // Process decrypted inner APDU
            ApplicationLayer::dataSystemBroadcastIndication(hopType, priority, source, plainFrame.apdu());
        }
        return;
    }

    ApplicationLayer::dataSystemBroadcastIndication(hopType, priority, source, apdu);
}

void SecureApplicationLayer::dataSystemBroadcastConfirm(HopCountType hopType, Priority priority, APDU& apdu, bool status)
{
    println("dataSystemBroadcastConfirm");

    if (apdu.type() == SecureService)
    {
/*
        // Decrypt secure APDU
        // Somehow ugly that we need to know the size in advance here at this point
        uint16_t plainApduLength = apdu.length() - 1 - 6 - 4; // secureAdsuLength - sizeof(scf) - sizeof(seqNum) - sizeof(mac)
        CemiFrame plainFrame(plainApduLength);
        // Decrypt secure APDU
        if (decodeSecureApdu(apdu, plainFrame.apdu()))
        {
            // Process decrypted inner APDU
            ApplicationLayer::dataSystemBroadcastConfirm(hopType, priority, plainFrame.apdu(), status);
        }
*/
        return;
    }

    ApplicationLayer::dataSystemBroadcastConfirm(hopType, priority, apdu, status);
}

void SecureApplicationLayer::dataIndividualIndication(HopCountType hopType, Priority priority, uint16_t source, APDU& apdu)
{
    println("dataIndividualIndication");

    if (apdu.type() == SecureService)
    {
        // Decrypt secure APDU
        // Somehow ugly that we need to know the size in advance here at this point
        uint16_t plainApduLength = apdu.length() - 1 - 6 - 4; // secureAdsuLength - sizeof(scf) - sizeof(seqNum) - sizeof(mac)
        CemiFrame plainFrame(plainApduLength);
        // Decrypt secure APDU
        if (decodeSecureApdu(apdu, plainFrame.apdu()))
        {
            // Process decrypted inner APDU
            ApplicationLayer::dataIndividualIndication(hopType, priority, source, plainFrame.apdu());
        }
        return;
    }

    ApplicationLayer::dataIndividualIndication(hopType, priority, source, apdu);
}

void SecureApplicationLayer::dataIndividualConfirm(AckType ack, HopCountType hopType, Priority priority, uint16_t source, APDU& apdu, bool status)
{
    println("dataIndividualConfirm");

    if (apdu.type() == SecureService)
    {
/*
        // Decrypt secure APDU

        // Process decrypted inner APDU
        ApplicationLayer::dataIndividualConfirm(ack, hopType, priority, source, apdu, status);
*/
        return;
    }
    else
    {
        ApplicationLayer::dataIndividualConfirm(ack, hopType, priority, source, apdu, status);
    }
}

void SecureApplicationLayer::dataConnectedIndication(Priority priority, uint16_t tsap, APDU& apdu)
{
    println("dataConnectedIndication");

    if (apdu.type() == SecureService)
    {
        // Somehow ugly that we need to know the size in advance here at this point
        uint16_t plainApduLength = apdu.length() - 1 - 6 - 4; // secureAdsuLength - sizeof(scf) - sizeof(seqNum) - sizeof(mac)
        CemiFrame plainFrame(plainApduLength);
        // Decrypt secure APDU
        if (decodeSecureApdu(apdu, plainFrame.apdu()))
        {
            // Process decrypted inner APDU
            ApplicationLayer::dataConnectedIndication(priority, tsap, plainFrame.apdu());
        }
        return;
    }

    ApplicationLayer::dataConnectedIndication(priority, tsap, apdu);
}

void SecureApplicationLayer::dataConnectedConfirm(uint16_t tsap)
{
    ApplicationLayer::dataConnectedConfirm(tsap);
}

/* to transport layer */

void SecureApplicationLayer::dataGroupRequest(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu)
{
    // TODO:
    // get flags for this TSAP from PID_GO_SECURITY_FLAGS from SecIntObj
    bool needsEncryption = false;

    if (needsEncryption)
    {
        uint16_t secureApduLength = apdu.length() + 2 + 6 + 4; // 2(TPCI,APCI,SCF) + sizeof(seqNum) + apdu.length() + 4
        CemiFrame secureFrame(secureApduLength);
        // create secure APDU
        if (createSecureApdu(apdu, secureFrame.apdu(), true, true)) // TODO: toolAccess, confidentialty
        {
            ApplicationLayer::dataGroupRequest(ack, hopType, priority, tsap, secureFrame.apdu());
        }
        return;
    }

    ApplicationLayer::dataGroupRequest(ack, hopType, priority, tsap, apdu);
}

void SecureApplicationLayer::dataBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, APDU& apdu)
{
    // TODO:
    // get flags for this TSAP from PID_GO_SECURITY_FLAGS from SecIntObj
    bool needsEncryption = true;

    if (needsEncryption)
    {
        uint16_t secureApduLength = apdu.length() + 2 + 6 + 4; // 2(TPCI,APCI,SCF) + sizeof(seqNum) + apdu.length() + 4
        CemiFrame secureFrame(secureApduLength);
        // create secure APDU
        if (createSecureApdu(apdu, secureFrame.apdu(), true, true)) // TODO: toolAccess, confidentialty
        {
            ApplicationLayer::dataBroadcastRequest(ack, hopType, SystemPriority, secureFrame.apdu());
        }
        return;
    }

    ApplicationLayer::dataBroadcastRequest(ack, hopType, SystemPriority, apdu);
}

void SecureApplicationLayer::dataSystemBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, APDU& apdu)
{
    // TODO:
    // get flags for this TSAP from PID_GO_SECURITY_FLAGS from SecIntObj
    bool needsEncryption = true;

    if (needsEncryption)
    {
        uint16_t secureApduLength = apdu.length() + 2 + 6 + 4; // 2(TPCI,APCI,SCF) + sizeof(seqNum) + apdu.length() + 4
        CemiFrame secureFrame(secureApduLength);
        // create secure APDU
        if (createSecureApdu(apdu, secureFrame.apdu(), true, true)) // TODO: toolAccess, confidentialty
        {
            ApplicationLayer::dataSystemBroadcastRequest(ack, hopType, SystemPriority, secureFrame.apdu());
        }
        return;
    }

    ApplicationLayer::dataSystemBroadcastRequest(ack, hopType, SystemPriority, apdu);
}

void SecureApplicationLayer::dataIndividualRequest(AckType ack, HopCountType hopType, Priority priority, uint16_t destination, APDU& apdu)
{
    // TODO:
    // get flags for this TSAP from PID_GO_SECURITY_FLAGS from SecIntObj
    bool needsEncryption = true;

    if (needsEncryption)
    {
        uint16_t secureApduLength = apdu.length() + 2 + 6 + 4; // 2(TPCI,APCI,SCF) + sizeof(seqNum) + apdu.length() + 4
        CemiFrame secureFrame(secureApduLength);
        // create secure APDU
        if (createSecureApdu(apdu, secureFrame.apdu(), true, true)) // TODO: toolAccess, confidentialty
        {
            ApplicationLayer::dataIndividualRequest(ack, hopType, priority, destination, secureFrame.apdu());
        }
        return;
    }

    ApplicationLayer::dataIndividualRequest(ack, hopType, priority, destination, apdu);
}

void SecureApplicationLayer::dataConnectedRequest(uint16_t tsap, Priority priority, APDU& apdu)
{
    // TODO:
    // get flags for this TSAP from PID_GO_SECURITY_FLAGS from SecIntObj
    bool needsEncryption = true;

    if (needsEncryption)
    {
        uint16_t secureApduLength = apdu.length() + 2 + 6 + 4; // 2(TPCI,APCI,SCF) + sizeof(seqNum) + apdu.length() + 4
        CemiFrame secureFrame(secureApduLength);
        // create secure APDU
        if (createSecureApdu(apdu, secureFrame.apdu(), true, true)) // TODO: toolAccess, confidentialty
        {
            ApplicationLayer::dataConnectedRequest(tsap, priority, secureFrame.apdu());
        }
        return;
    }

    // apdu must be valid until it was confirmed
    ApplicationLayer::dataConnectedRequest(tsap, priority, apdu);
}

void SecureApplicationLayer::encryptAesCbc(uint8_t* buffer, uint16_t bufLen, const uint8_t* iv, const uint8_t* key)
{
    // Use zeroes as IV for first round
    uint8_t zeroIv[16] = {0x00};

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, zeroIv);

    // Now encrypt first block B0.
    AES_CBC_encrypt_buffer(&ctx, (uint8_t*) iv, 16);

    // Encrypt remaining buffer
    AES_CBC_encrypt_buffer(&ctx, buffer, bufLen);
}

void SecureApplicationLayer::xcryptAesCtr(uint8_t* buffer, uint16_t bufLen, const uint8_t* iv, const uint8_t* key)
{
    struct AES_ctx ctx;

    AES_init_ctx_iv(&ctx, key, iv);

    AES_CTR_xcrypt_buffer(&ctx, buffer, bufLen);
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

    encryptAesCbc(buffer, bufLenPadded, iv, key);
    xcryptAesCtr(buffer, 4, ctr0, key); // 4 bytes only for the MAC

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

    encryptAesCbc(buffer, bufLenPadded, iv, key);

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

void SecureApplicationLayer::sixBytesFromUInt64(uint64_t num, uint8_t* toByteArray)
{
    toByteArray[0] = ((num >> 40) & 0xff);
    toByteArray[1] = ((num >> 32) & 0xff);
    toByteArray[2] = ((num >> 24) & 0xff);
    toByteArray[3] = ((num >> 16) & 0xff);
    toByteArray[4] = ((num >> 8) & 0xff);
    toByteArray[5] = (num & 0xff);
}

uint64_t SecureApplicationLayer::sixBytesToUInt64(uint8_t* data)
{
    uint64_t l = 0;

    for (uint8_t i = 0; i < 6; i++)
    {
        l = (l << 8) + data[i];
    }
    return l;
}

const uint8_t* SecureApplicationLayer::toolKey(uint16_t devAddr)
{
    //TODO: multiple tool keys possible
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
    {
        sequenceNumberToolAccess = seqNum;
        //TODO: securityInterface.set(Pid.ToolSequenceNumberSending, sixBytes(seqNo).array());
    }
    else
    {
        sequenceNumber = seqNum;
        //TODO: securityInterface.set(Pid.SequenceNumberSending, sixBytes(seqNo).array());
    }
}

uint64_t SecureApplicationLayer::lastValidSequenceNumber(bool toolAcces, uint16_t srcAddr)
{
    if (toolAcces)
    {
        // TODO: add map to handle multiplpe lastValidSequenceNumberTool for each srcAddr
        // lastValidSequence.getOrDefault(remote, 0L);
        return lastValidSequenceNumberTool;
    }
    else
    {
/*
 *  TODO:
        byte[] addresses = securityInterface.get(Pid.SecurityIndividualAddressTable);
        var addr = remote.toByteArray();
        // precondition: array size is multiple of entrySize
        int entrySize = 2 + 6; // Address and SeqNum
        for (int offset = 0; offset < addresses.length; offset += entrySize)
        {
            if (Arrays.equals(addr, 0, addr.length, addresses, offset, offset + 2))
                return unsigned(Arrays.copyOfRange(addresses, offset + 2, offset + 2 + 6));
        }
*/
    }

    return 0;
}

void SecureApplicationLayer::updateLastValidSequence(bool toolAccess, uint16_t remoteAddr, uint64_t seqNo)
{
    if (toolAccess)
        // TODO: add map to handle multiple lastValidSequenceNumberTool for each srcAddr
        //lastValidSequenceToolAccess.put(remoteAddr, seqNo);
        lastValidSequenceNumberTool = seqNo;
    else
    {
/*
 * TODO:
        byte[] addresses = securityInterface.get(Pid.SecurityIndividualAddressTable);
        var addr = remote.toByteArray();

        int entrySize = addr.length + 6; // Address + SeqNum
        // precondition: array size is multiple of entrySize
        for (int offset = 0; offset < addresses.length; offset += entrySize) {
            if (Arrays.equals(addr, 0, addr.length, addresses, offset, offset + 2)) {
                final var start = 1 + offset / entrySize;
                final var data = ByteBuffer.allocate(8).put(addr).put(sixBytes(seqNo));
                securityInterface.set(Pid.SecurityIndividualAddressTable, start, 1, data.array());
                break;
            }
        }
*/
    }
}

void SecureApplicationLayer::sendSyncResponse(uint16_t dstAddr, bool dstAddrIsGroupAddr, bool toolAccess, uint64_t remoteNextSeqNum)
{
    uint64_t ourNextSeqNum = nextSequenceNumber(toolAccess);

    uint8_t asdu[12];
    sixBytesFromUInt64(ourNextSeqNum, &asdu[0]);
    sixBytesFromUInt64(remoteNextSeqNum, &asdu[6]);

    CemiFrame response(2 + 6 + sizeof(asdu) + 4); // 2 bytes (APCI, SCF) + 6 bytes (SeqNum) + 12 bytes + 4 bytes (MAC)
                                                      // Note: additional TPCI byte is already handled internally!

    uint8_t tpci = 0;
    if (!_syncReqBroadcast)
    {
        tpci = _transportLayer->getTPCI(dstAddr); // get next TPCI sequence number for MAC calculation from TL
    }
    print("sendSyncResponse: TPCI: ");
    println(tpci, HEX);

    if(secure(response.data() + APDU_LPDU_DIFF, SecureSyncResponse, _deviceObj.induvidualAddress(), dstAddr, dstAddrIsGroupAddr, tpci, asdu, sizeof(asdu), toolAccess, true))
    {
        _lastSyncRes = millis();

        println("SyncResponse: ");
        response.apdu().printPDU();

        if (_syncReqBroadcast)
        {
            _transportLayer->dataBroadcastRequest(AckType::AckDontCare, HopCountType::NetworkLayerParameter, Priority::SystemPriority, response.apdu());
        }
        else
        {
            //TODO: either send on T_DATA_INDIVIDUAL or T_DATA_CONNECTED depending on reception

            // Send encrypted SyncResponse message using T_DATA_INDIVIDUAL
            _transportLayer->dataIndividualRequest(AckType::AckDontCare, NetworkLayerParameter, SystemPriority, dstAddr, response.apdu());
        }
    }
}

void SecureApplicationLayer::receivedSyncRequest(uint16_t srcAddr, uint16_t dstAddr, bool dstAddrIsGroupAddr, bool toolAccess, uint8_t* seqNum, uint64_t challenge)
{
    uint64_t nextRemoteSeqNum = sixBytesToUInt64(seqNum);
    uint64_t nextSeqNum = 1 + lastValidSequenceNumber(toolAccess, srcAddr);

    if (nextRemoteSeqNum > nextSeqNum)
    {
        updateLastValidSequence(toolAccess, srcAddr, nextRemoteSeqNum - 1);
        nextSeqNum = nextRemoteSeqNum;
    }

    // Remember challenge for securing the sync.res later
    //stashSyncRequest(srcAddr, challenge); // TODO: save challenge in a map to handle multiple requests
    sixBytesFromUInt64(challenge, _challenge);

    _syncReqBroadcast = (dstAddr == 0x0000) && dstAddrIsGroupAddr;

    uint16_t toAddr = _syncReqBroadcast ? dstAddr : srcAddr;
    bool toIsGroupAddress = _syncReqBroadcast;

    _challengeSrcAddr = toAddr;
    _isChallengeValid = true;

    sendSyncResponse(toAddr, toIsGroupAddress, toolAccess, nextSeqNum);
}

void SecureApplicationLayer::receivedSyncResponse(uint16_t remoteAddr, bool toolAccess, uint8_t* plainApdu)
{
//    final var request = pendingSyncRequests.get(remote);
//    if (request == null)
//        return;
    if (_challengeSrcAddr != remoteAddr)
    {
        println("receivedSyncResponse(): Did not find matching challenge for remoteAddr!");
        return;
    }

    // Bytes 0-5 in the "APDU" buffer contain the remote sequence number
    // Bytes 6-11 in the "APDU" buffer contain the local sequence number
    uint64_t remoteSeq = sixBytesToUInt64(plainApdu + 0);
    uint64_t localSeq = sixBytesToUInt64(plainApdu + 6);

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

bool SecureApplicationLayer::decrypt(uint8_t* plainApdu, uint16_t plainApduLength, uint16_t srcAddr, uint16_t dstAddr, bool dstAddrIsGroupAddr, uint8_t tpci, uint8_t* secureAsdu)
{
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

    //const uint8_t* key = dstAddrIsGroupAddr ? securityKey(dstAddr, dstAddrIsGroupAddr) : toolAccess ? toolKey(srcAddr == _deviceObj.induvidualAddress() ? dstAddr : srcAddr) : securityKey(srcAddr, false);
    const uint8_t* key = dstAddrIsGroupAddr && (dstAddr != 0) ? securityKey(dstAddr, dstAddrIsGroupAddr) : toolAccess ? toolKey(srcAddr == _deviceObj.induvidualAddress() ? dstAddr : srcAddr) : securityKey(srcAddr, false);
    if (key == nullptr)
    {
        print("Error: No key found. toolAccess: ");
        println(toolAccess ? "true" : "false");
        return false;
    }

    uint8_t seqNum[6];
    pBuf = popByteArray(seqNum, 6, pBuf);
    uint64_t receivedSeqNumber = sixBytesToUInt64(seqNum);

    // Provide array for KNX serial number if it is a SyncRequest
    // DataService and SyncResponse do not use this variable.
    uint8_t knxSerialNumber[6];

    uint16_t remainingPlainApduLength = plainApduLength;

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
        remainingPlainApduLength -= 6;

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
        // TODO: fetch challenge depending on srcAddr to handle multiple requests (would use std::unordered_map normally)
        //final var request = pendingSyncRequests.get(src);
        //if (request == null)
        //    return null;
        if (_challengeSrcAddr != srcAddr)
        {
            println("Did not find matching challenge for source address!");
            return false;
        }

        // in a sync.res, seq actually contains our challenge from sync.req xored with a random value
        // extract the random value and store it in seqNum to use it for block0 and ctr0
        for (uint8_t i = 0; i < sizeof(seqNum); i++)
        {
            seqNum[i] ^= _challenge[i];
        }
    }

    pBuf = popByteArray(plainApdu, remainingPlainApduLength, pBuf);

    // No LTE-HEE for now
    // Data Secure always uses extended frame format with APDU length > 15 octets (long messages), no standard frames
    uint8_t extendedFrameFormat = 0;
    // Clear IV buffer
    uint8_t iv[16] = {0x00};
    // Create first block B0 for AES CBC MAC calculation, used as IV later
    block0(iv, seqNum, srcAddr, dstAddr, dstAddrIsGroupAddr, extendedFrameFormat, tpci | (SecureService >> 8), SecureService & 0x00FF, remainingPlainApduLength);

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
        uint32_t calculatedMac = calcAuthOnlyMac(plainApdu, remainingPlainApduLength, key, iv, ctr0);
        if (calculatedMac != mac)
        {
            // security failure
            print("security failure(auth): calculated MAC: ");
            print(calculatedMac, HEX);
            print(" != received MAC: ");
            print(mac, HEX);
            println("\n");

            return false;
        }

        memcpy(plainApdu, secureAsdu, remainingPlainApduLength);
    }
    else
    {
        // APDU is encrypted and needs decryption

        uint16_t bufLen = 4 + remainingPlainApduLength;
        // AES-128 operates on blocks of 16 bytes, add padding
        uint16_t bufLenPadded = (bufLen + 15) / 16 * 16;
        uint8_t buffer[bufLenPadded];
        //uint8_t buffer[bufLen];
        // Make sure to have zeroes everywhere, because of the padding
        memset(buffer, 0x00, bufLenPadded);

        pushInt(mac, &buffer[0]);
        pushByteArray(plainApdu, remainingPlainApduLength, &buffer[4]); // apdu is still encrypted

        xcryptAesCtr(buffer, bufLenPadded, ctr0, key);
        //xcryptAesCtr(buffer, bufLen, ctr0, key);

        uint32_t decryptedMac;
        popInt(decryptedMac, &buffer[0]);
        popByteArray(plainApdu, remainingPlainApduLength, &buffer[4]); // apdu is now decrypted (overwritten)

        // Do calculations for Auth+Conf
        uint8_t associatedData[syncReq ? 7 : 1];
        associatedData[0] = scf;
        if (syncReq)
        {
            memcpy(&associatedData[1], knxSerialNumber, 6);
        }
        uint32_t calculatedMac = calcConfAuthMac(associatedData, sizeof(associatedData), plainApdu, remainingPlainApduLength, key, iv);
        if (calculatedMac != decryptedMac)
        {
            // security failure
            print("security failure(conf+auth): calculated MAC: ");
            print(calculatedMac, HEX);
            print(" != decrypted MAC: ");
            print(decryptedMac, HEX);
            println("\n");

            return false;
        }

        // prevent a sync.req sent by us to trigger sync notification, this happens if we provide our own tool key
        // for decryption above
        if (syncReq && (srcAddr == _deviceObj.induvidualAddress()))
            return false;

        if (syncReq)
        {
            uint64_t challenge = sixBytesToUInt64(&plainApdu[0]);
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
        // In case it was a sync.req or a sync.res we not have anything for the application layer to send to
        if (syncReq || syncRes)
            return false;
    }

    return true;
}

bool SecureApplicationLayer::decodeSecureApdu(APDU& secureApdu, APDU& plainApdu)
{
    // Decode secure APDU

    println("decodeSecureApdu: Secure APDU: ");
    secureApdu.printPDU();

    uint16_t srcAddress = secureApdu.frame().sourceAddress();
    uint16_t dstAddress = secureApdu.frame().destinationAddress();
    bool isDstAddrGroupAddr = secureApdu.frame().addressType() == GroupAddress;
    uint8_t tpci = secureApdu.frame().data()[TPDU_LPDU_DIFF]; // FIXME: when cEMI class is refactored, there might be additional info fields in cEMI [fixed TPDU_LPDU_DIFF]
    print("decodeSecureApdu: TPCI: ");
    println(tpci, HEX);
    // Note:
    // The TPCI is also included in the MAC calculation to provide authenticity for this field.
    // However, a secure APDU (with a MAC) is only included in transport layer PDUs T_DATA_GROUP, T_DATA_TAG_GROUP, T_DATA_INDIVIDUAL, T_DATA_CONNECTED
    // and not in T_CONNECT, T_DISCONNECT, T_ACK, T_NACK.
    // This means the DATA/CONTROL flag is always 0(=DATA). The flag "NUMBERED" differentiates between T_DATA_INDIVIDUAL and T_DATA_CONNECTED.
    // The seqNumber is only used in T_DATA_CONNECTED and 0 in case of T_DATA_GROUP and T_DATA_GROUP (leaving out T_DATA_TAG_GROUP).
    // Summary: effectively only the "NUMBERED" flag (bit6) and the SeqNumber (bit5-2) are used from transport layer.
    //          In T_DATA_* services the bits1-0 of TPCI octet are used as bits9-8 for APCI type which is fixed to 0x03. SecureService APCI is 0x03F1.

    // FIXME: when cEMI class is refactored, there might be additional info fields in cEMI (fixed APDU_LPDU_DIFF)
    if (decrypt(plainApdu.frame().data()+APDU_LPDU_DIFF, plainApdu.length()-1, srcAddress, dstAddress, isDstAddrGroupAddr, tpci, secureApdu.data()+1))
    {
        println("decodeSecureApdu: Plain APDU: ");
        plainApdu.frame().apdu().printPDU();

        return true;
    }

    return false;
}

bool SecureApplicationLayer::secure(uint8_t* buffer, uint16_t service, uint16_t srcAddr, uint16_t dstAddr, bool dstAddrIsGroupAddr, uint8_t tpci,
                                    uint8_t* apdu, uint16_t apduLength, bool toolAccess, bool confidentiality)
{
    if (toolAccess)
    {
        if (!confidentiality)
        {
            println("Error: tool access requires auth+conf security");
            return false;
        }
        if (dstAddrIsGroupAddr && dstAddr != 0)
        {
            println("Error: tool access requires individual address");
            return false;
        }
    }

    const uint8_t* key = toolAccess ? toolKey(_syncReqBroadcast ? _deviceObj.induvidualAddress() : dstAddr) : securityKey(dstAddr, dstAddrIsGroupAddr);
    if (key == nullptr)
    {
        print("Error: No key found. toolAccess: ");
        println(toolAccess ? "true" : "false");
        return false;
    }

    bool syncReq = service == SecureSyncRequest;
    bool syncRes = service == SecureSyncResponse;

    tpci |= SecureService >> 8; // OR'ing upper two APCI bits
    uint8_t apci = SecureService & 0x00FF;
    uint8_t* pBuf = buffer;
    pBuf = pushByte(tpci, pBuf);                      // TPCI
    pBuf = pushByte(apci, pBuf);                      // APCI

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
    sixBytesFromUInt64(seqSend, seq);
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
        // use random number in SyncResponse
        uint64_t randomNumber = 0x000102030405; // TODO: generate random number
        sixBytesFromUInt64(randomNumber, seq);

        // TODO: maybe implement something like std::map for pending SyncRequests?
        //final var request = pendingSyncRequests.remove(dst);
        //if (request == null)
        //    throw new KnxSecureException("sending sync.res without corresponding .req");
        if (_isChallengeValid && (_challengeSrcAddr == dstAddr))
        {
            // Invalidate _challengeSrcAddr
            _challengeSrcAddr = 0;
            _isChallengeValid = false;
        }
        else
        {
            println("sending sync.res without corresponding .req");
        }

        //printHex("Decrypted challenge: ", _challenge, 6);

        // Now XOR the new random SeqNum with the challenge from the SyncRequest
        uint8_t rndXorChallenge[6];
        pushByteArray(seq, 6, rndXorChallenge);
        for (uint8_t i = 0; i < sizeof(rndXorChallenge); i++)
        {
            rndXorChallenge[i] ^= _challenge[i];
        }
        pBuf = pushByteArray(rndXorChallenge, 6, pBuf);
    }

    // No LTE-HEE for now
    // Data Secure always uses extended frame format with APDU length > 15 octets (long messages), no standard frames
    uint8_t extendedFrameFormat = 0;
    // Clear IV buffer
    uint8_t iv[16] = {0x00};
    // Create first block B0 for AES CBC MAC calculation, used as IV later
    block0(iv, seq, srcAddr, dstAddr, dstAddrIsGroupAddr, extendedFrameFormat, tpci, apci, apduLength);

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

        xcryptAesCtr(tmpBuffer, apduLength + 4, ctr0, key); // APDU + MAC (4 bytes)

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

    return true;
}

bool SecureApplicationLayer::createSecureApdu(APDU& plainApdu, APDU& secureApdu, bool toolAccess, bool confidentialty)
{
    // Create secure APDU

    println("createSecureApdu: Plain APDU: ");
    plainApdu.printPDU();

    uint16_t srcAddress = plainApdu.frame().sourceAddress();
    uint16_t dstAddress = plainApdu.frame().destinationAddress();
    bool isDstAddrGroupAddr = plainApdu.frame().addressType() == GroupAddress;
    uint8_t tpci = _transportLayer->getTPCI(dstAddress); // get next TPCI sequence number for MAC calculation from TL
    print("createSecureApdu: TPCI: ");
    println(tpci, HEX);
    // Note:
    // The TPCI is also included in the MAC calculation to provide authenticity for this field.
    // However, a secure APDU (with a MAC) is only included in transport layer PDUs T_DATA_GROUP, T_DATA_TAG_GROUP, T_DATA_INDIVIDUAL, T_DATA_CONNECTED
    // and not in T_CONNECT, T_DISCONNECT, T_ACK, T_NACK.
    // This means the DATA/CONTROL flag is always 0(=DATA). The flag "NUMBERED" differentiates between T_DATA_INDIVIDUAL and T_DATA_CONNECTED.
    // The seqNumber is only used in T_DATA_CONNECTED and 0 in case of T_DATA_GROUP and T_DATA_GROUP (leaving out T_DATA_TAG_GROUP).
    // Summary: effectively only the "NUMBERED" flag (bit6) and the SeqNumber (bit5-2) are used from transport layer.
    //          In T_DATA_* services the bits1-0 of TPCI octet are used as bits9-8 for APCI type which is fixed to 0x03. SecureService APCI is 0x03F1.

    // FIXME: when cEMI class is refactored, there might be additional info fields in cEMI (fixed APDU_LPDU_DIFF)
    if(secure(secureApdu.frame().data()+APDU_LPDU_DIFF, SecureDataPdu, srcAddress, dstAddress, isDstAddrGroupAddr, tpci, plainApdu.data()+1, plainApdu.length()-1, toolAccess, confidentialty))
    {
        updateSequenceNumber(toolAccess, nextSequenceNumber(toolAccess) + 1);

        println("createSecureApdu: Secure APDU: ");
        plainApdu.frame().apdu().printPDU();

        return true;
    }

    return false;
}

void SecureApplicationLayer::setSecurityMode(bool enabled)
{
    _securityModeEnabled = enabled;
}

bool SecureApplicationLayer::isSecurityModeEnabled()
{
    return _securityModeEnabled;
}

void SecureApplicationLayer::clearFailureLog()
{
    println("clearFailureLog()");
}

void SecureApplicationLayer::getFailureCounters(uint8_t* data)
{
    memset(data, 0, 8);
    println("getFailureCounters()");
}

uint8_t SecureApplicationLayer::getFromFailureLogByIndex(uint8_t index, uint8_t* data, uint8_t maxDataLen)
{
    print("getFromFailureLogByIndex(): Index: ");
    println(index);
    return 0;
}
