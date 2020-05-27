#include "secure_application_layer.h"
#include "transport_layer.h"
#include "cemi_frame.h"
#include "association_table_object.h"
#include "apdu.h"
#include "bau.h"
#include "string.h"
#include "bits.h"
#include <stdio.h>

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
