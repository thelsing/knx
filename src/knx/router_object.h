#pragma once

#include "config.h"

#include "table_object.h"
#include "knx_types.h"

class Memory;

enum CouplerModel
{
    Model_1x,
    Model_20
};

class RouterObject : public TableObject
{
public:
  RouterObject(Memory& memory);

  void initialize(CouplerModel model, uint8_t objIndex, DptMedium mediumType, bool useHopCount, bool useTable, uint16_t maxApduSize);

  bool isGroupAddressInFilterTable(uint16_t groupAddress);

  bool isRfSbcRoutingEnabled();

  virtual void masterReset(EraseCode eraseCode, uint8_t channel) override;

  const uint8_t* restore(const uint8_t* buffer) override;

protected:
  virtual void beforeStateChange(LoadState& newState) override;

private:
  // Function properties
  void functionRouteTableControl(bool isCommand, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength);
  void functionRfEnableSbc(bool isCommand, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength);

  void commandClearSetRoutingTable(bool bitIsSet);
  bool statusClearSetRoutingTable(bool bitIsSet);
  void commandClearSetGroupAddress(uint16_t startAddress, uint16_t endAddress, bool bitIsSet);
  bool statusClearSetGroupAddress(uint16_t startAddress, uint16_t endAddress, bool bitIsSet);

  void updateMcb();

  bool _rfSbcRoutingEnabled = false;
  uint16_t* _filterTableGroupAddresses = 0;
};
