#include "bau_systemB_coupler.h"
#include "bits.h"
#include <string.h>
#include <stdio.h>

BauSystemBCoupler::BauSystemBCoupler(Platform& platform) :
    BauSystemB(platform),
    _platform(platform),
#ifdef USE_DATASECURE
    _appLayer(_deviceObj, _secIfObj, *this),
#else
    _appLayer(*this),
#endif
    _transLayer(_appLayer),
    _netLayer(_deviceObj, _transLayer)
{
    _appLayer.transportLayer(_transLayer);
    _transLayer.networkLayer(_netLayer);
    _memory.addSaveRestore(&_deviceObj);
#ifdef USE_DATASECURE
    _memory.addSaveRestore(&_secIfObj);
#endif
}

ApplicationLayer& BauSystemBCoupler::applicationLayer()
{
    return _appLayer;
}

void BauSystemBCoupler::loop()
{
    _transLayer.loop();
#ifdef USE_DATASECURE
    _appLayer.loop();
#endif
}

bool BauSystemBCoupler::configured()
{
    // _configured is set to true initially, if the device was configured with ETS it will be set to true after restart
    
    if (!_configured)
        return false;
    
    _configured = _appProgram.loadState() == LS_LOADED;
#ifdef USE_DATASECURE
    _configured &= _secIfObj.loadState() == LS_LOADED;
#endif
    
    return _configured;
}

void BauSystemBCoupler::doMasterReset(EraseCode eraseCode, uint8_t channel)
{
    BauSystemB::doMasterReset(eraseCode, channel);

#ifdef USE_DATASECURE
    _secIfObj.masterReset(eraseCode, channel);
#endif
}
