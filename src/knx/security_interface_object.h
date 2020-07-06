#pragma once

#include "config.h"
#ifdef USE_DATASECURE

#include "interface_object.h"
#include "knx_types.h"

class SecureApplicationLayer;

class SecurityInterfaceObject: public InterfaceObject
{
public:
  SecurityInterfaceObject();

  void secureApplicationLayer(SecureApplicationLayer& secAppLayer);

  virtual void masterReset(EraseCode eraseCode, uint8_t channel) override;

  bool isSecurityModeEnabled();

  bool isLoaded();

  const uint8_t* toolKey();                           // returns single tool key (ETS)
  const uint8_t* p2pKey(uint16_t addressIndex);       // returns p2p key for IA index
  const uint8_t* groupKey(uint16_t addressIndex);     // returns group key for group address index

  uint16_t indAddressIndex(uint16_t indAddr);         // returns 1-based index of address in security IA table

  void setSequenceNumber(bool toolAccess, uint64_t seqNum);
  uint64_t getLastValidSequenceNumber(uint16_t deviceAddr);
  void setLastValidSequenceNumber(uint16_t deviceAddr, uint64_t seqNum);

  DataSecurity getGroupObjectSecurity(uint16_t index);

  LoadState loadState();
  uint8_t* save(uint8_t* buffer) override;
  const uint8_t* restore(const uint8_t* buffer) override;
  uint16_t saveSize() override;

private:
  SecureApplicationLayer* _secAppLayer = nullptr;

  void setSecurityMode(bool enabled);

  void errorCode(ErrorCode errorCode);

  void loadEvent(const uint8_t* data);
  void loadEventUnloaded(const uint8_t* data);
  void loadEventLoading(const uint8_t* data);
  void loadEventLoaded(const uint8_t* data);
  void loadEventError(const uint8_t* data);

  void loadState(LoadState newState);
  LoadState _state = LS_UNLOADED;

  bool _securityModeEnabled {false};

  uint16_t getNumberOfElements(PropertyID propId);

  // Our FDSK
  static const uint8_t _fdsk[];
  static uint8_t _secReport[];
  static uint8_t _secReportCtrl[];
};
#endif
