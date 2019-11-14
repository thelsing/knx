#pragma once

#include <stdint.h>
#include "knx_types.h"
#include "usb_data_link_layer.h"

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
     * @param tunnelInterface The TunnelInterface of the KNX tunnel (e.g. USB or KNXNET/IP)
     * @param bau methods are called here depending of the content of the APDU
     */
    CemiServer(BauSystemB& bau);

    void dataLinkLayer(DataLinkLayer& layer);

    // from data link layer
    // Only L_Data service
    void dataIndicationToTunnel(CemiFrame& frame);

    // From tunnel interface
    void frameReceived(CemiFrame& frame);

    void loop();
    
  private:
    DataLinkLayer* _dataLinkLayer;
    BauSystemB& _bau;
    UsbDataLinkLayer _usbTunnelInterface;
};
