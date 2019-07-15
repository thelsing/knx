#pragma once

#include <stdint.h>

class CemiFrame;
class TPDU;

class NPDU
{
    friend class CemiFrame;

  public:
    uint8_t octetCount() const;
    void octetCount(uint8_t value);
    uint8_t length() const;
    uint8_t hopCount() const;
    void hopCount(uint8_t value);
    CemiFrame& frame();
    TPDU& tpdu();

  protected:
    NPDU(uint8_t* data, CemiFrame& frame);

  private:
    uint8_t* _data = 0;
    CemiFrame& _frame;
};