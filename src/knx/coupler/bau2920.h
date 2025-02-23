#pragma once

#include "../config.h"

#include "../tp/tpuart_data_link_layer.h"
#include "bau_systemB_coupler.h"
#if defined(DeviceFamily_CC13X0)
#include "../rf/rf_physical_layer_cc1310.h"
#else
#include "../rf/rf_physical_layer_cc1101.h"
#endif
#include "../cemi_server/cemi_server_object.h"
#include "../rf/rf_data_link_layer.h"
#include "../rf/rf_medium_object.h"

namespace Knx
{
    class Bau2920 : public BauSystemBCoupler
    {
        public:
            Bau2920(Platform& platform);
            void loop() override;
            bool enabled() override;
            void enabled(bool value) override;

            TpUartDataLinkLayer* getPrimaryDataLinkLayer();
            RfDataLinkLayer* getSecondaryDataLinkLayer();

        protected:
            InterfaceObject* getInterfaceObject(uint8_t idx);
            InterfaceObject* getInterfaceObject(ObjectType objectType, uint16_t objectInstance);

            void doMasterReset(EraseCode eraseCode, uint8_t channel) override;

        private:
            RouterObject _rtObjPrimary;
            RouterObject _rtObjSecondary;
            RfMediumObject _rfMediumObject;
            TpUartDataLinkLayer _dlLayerPrimary;
            RfDataLinkLayer _dlLayerSecondary;
#ifdef USE_CEMI_SERVER
            CemiServer _cemiServer;
            CemiServerObject _cemiServerObject;
#endif
    };
} // namespace Knx