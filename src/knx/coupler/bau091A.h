#pragma once

#include "../cemi_server/cemi_server_object.h"
#include "../config.h"
#include "../ip/ip_data_link_layer.h"
#include "../ip/ip_parameter_object.h"
#include "../tp/tpuart_data_link_layer.h"
#include "bau_systemB_coupler.h"
#include "router_object.h"

namespace Knx
{
    class Bau091A : public BauSystemBCoupler, public ITpUartCallBacks, public DataLinkLayerCallbacks
    {
        public:
            Bau091A(Platform& platform);
            void loop() override;
            bool enabled() override;
            void enabled(bool value) override;
            bool configured() override;

            IpDataLinkLayer* getPrimaryDataLinkLayer();
            TpUartDataLinkLayer* getSecondaryDataLinkLayer();

        protected:
            InterfaceObject* getInterfaceObject(uint8_t idx);
            InterfaceObject* getInterfaceObject(ObjectType objectType, uint16_t objectInstance);

            // For TP1 only
            TPAckType isAckRequired(uint16_t address, bool isGrpAddr) override;

            void doMasterReset(EraseCode eraseCode, uint8_t channel) override;

        private:
            RouterObject _routerObj;
            IpParameterObject _ipParameters;
            IpDataLinkLayer _dlLayerPrimary;
            TpUartDataLinkLayer _dlLayerSecondary;
#ifdef USE_CEMI_SERVER
            CemiServer _cemiServer;
            CemiServerObject _cemiServerObject;
#endif
    };
} // namespace Knx