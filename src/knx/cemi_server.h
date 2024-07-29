#pragma once

#include "config.h"
#ifdef USE_CEMI_SERVER

#include <stdint.h>
#include "knx_types.h"
#include "usb_tunnel_interface.h"

class BauSystemB;
class DataLinkLayer;
class CemiFrame;

/**
 * This is an implementation of the cEMI server as specified in @cite knx:3/6/3.
 * Overview on page 57.
 * It provides methods for the BusAccessUnit to do different things and translates this 
 * call to an cEMI frame and calls the correct method of the data link layer. 
 * It also takes calls from data link layer, decodes the submitted cEMI frames and calls the corresponding
 * methods of the BusAccessUnit class.
 */
class CemiServer
{
  public:
    /**
     * The constructor.
     * @param bau methods are called here depending of the content of the APDU
     */
    CemiServer(BauSystemB& bau);

    void dataLinkLayer(DataLinkLayer& layer);
#ifdef KNX_TUNNELING
    void dataLinkLayerPrimary(DataLinkLayer& layer);
#endif

    // from data link layer
    // Only L_Data service
    void dataIndicationToTunnel(CemiFrame& frame);
    void dataConfirmationToTunnel(CemiFrame& frame);

    // From tunnel interface
    void frameReceived(CemiFrame& frame);

    uint16_t clientAddress() const;
    void clientAddress(uint16_t value);

    void loop();
    
  private:
    uint16_t _clientAddress = 0;
    uint8_t _frameNumber = 0;

    void handleLData(CemiFrame& frame);
    void handleMPropRead(CemiFrame& frame);
    void handleMPropWrite(CemiFrame& frame);
    void handleMReset(CemiFrame& frame);

    DataLinkLayer* _dataLinkLayer = nullptr;
#ifdef KNX_TUNNELING
    DataLinkLayer* _dataLinkLayerPrimary = nullptr;
#endif
    BauSystemB& _bau;
#ifdef USE_USB
    UsbTunnelInterface _usbTunnelInterface;
#endif
};

#endif