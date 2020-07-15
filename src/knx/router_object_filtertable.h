#pragma once

#include "config.h"

#include "router_object.h"
#include "knx_types.h"

class Memory;

class RouterObjectFilterTable: public RouterObject
{
public:
  RouterObjectFilterTable(Memory& memory);

  bool isGroupAddressInFilterTable(uint16_t groupAddress);

  bool isRfSbcRoutingEnabled();

  virtual void masterReset(EraseCode eraseCode, uint8_t channel) override;

  bool isLoaded();

  LoadState loadState();
  uint8_t* save(uint8_t* buffer) override;
  const uint8_t* restore(const uint8_t* buffer) override;
  uint16_t saveSize() override;


private:
  // Function properties
  void functionRouteTableControl(bool isCommand, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength);
  void functionRfEnableSbc(bool isCommand, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength);

  void updateMcb();

  uint32_t tableReference();
  bool allocTable(uint32_t size, bool doFill, uint8_t fillByte);
  void errorCode(ErrorCode errorCode);

  void loadEvent(const uint8_t* data);
  void loadEventUnloaded(const uint8_t* data);
  void loadEventLoading(const uint8_t* data);
  void loadEventLoaded(const uint8_t* data);
  void loadEventError(const uint8_t* data);
  void additionalLoadControls(const uint8_t* data);
  void beforeStateChange(LoadState& newState);

  void loadState(LoadState newState);
  LoadState _state = LS_UNLOADED;

  Memory& _memory;
  uint8_t *_data = 0;
  bool _rfSbcRoutingEnabled = false;
  uint16_t* _filterTableGroupAddresses = 0;
};
