#pragma once

#include "config.h"

#include "router_object.h"
#include "knx_types.h"

class Memory;

class RouterObjectFilterTable: public RouterObject
{
public:
  RouterObjectFilterTable(Memory& memory);

  virtual void masterReset(EraseCode eraseCode, uint8_t channel) override;

  bool isLoaded();

  LoadState loadState();
  uint8_t* save(uint8_t* buffer) override;
  const uint8_t* restore(const uint8_t* buffer) override;
  uint16_t saveSize() override;

private:
  uint32_t tableReference();
  void errorCode(ErrorCode errorCode);

  void loadEvent(const uint8_t* data);
  void loadEventUnloaded(const uint8_t* data);
  void loadEventLoading(const uint8_t* data);
  void loadEventLoaded(const uint8_t* data);
  void loadEventError(const uint8_t* data);

  void loadState(LoadState newState);
  LoadState _state = LS_UNLOADED;

  Memory& _memory;
  uint8_t *_data = 0;
};
