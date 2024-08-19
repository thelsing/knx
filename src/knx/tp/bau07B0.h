#pragma once

#include "../config.h"

#include "../bau/bau_systemB_device.h"
#include "../tp/tpuart_data_link_layer.h"
#include "../cemi_server/cemi_server.h"
#include "../cemi_server/cemi_server_object.h"

class Bau07B0 : public BauSystemBDevice, public ITpUartCallBacks, public DataLinkLayerCallbacks
{
    public:
        Bau07B0(Platform& platform);
        void loop() override;
        bool enabled() override;
        void enabled(bool value) override;

        TpUartDataLinkLayer* getDataLinkLayer();
    protected:
        InterfaceObject* getInterfaceObject(uint8_t idx);
        InterfaceObject* getInterfaceObject(ObjectType objectType, uint16_t objectInstance);

        // For TP1 only
        TPAckType isAckRequired(uint16_t address, bool isGrpAddr) override;

    private:
        TpUartDataLinkLayer _dlLayer;
#ifdef USE_CEMI_SERVER
        CemiServer _cemiServer;
        CemiServerObject _cemiServerObject;
#endif
};
