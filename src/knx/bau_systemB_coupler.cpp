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
    _transLayer(_appLayer), _netLayer(_deviceObj, _transLayer)
{
#ifdef USE_DATASECURE
    _secIfObj.secureApplicationLayer(_appLayer);
#endif
    _appLayer.transportLayer(_transLayer);
    _transLayer.networkLayer(_netLayer);
    _memory.addSaveRestore(&_deviceObj);
    _memory.addSaveRestore(&_appProgram);
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
    dataLinkLayer().loop();
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
    
    return _configured;
}

void BauSystemBCoupler::doMasterReset(EraseCode eraseCode, uint8_t channel)
{
}
