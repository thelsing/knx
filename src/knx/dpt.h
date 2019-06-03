#pragma once

class Dpt
{
  public:
    Dpt();
    Dpt(short mainGroup, short subGroup, short index = 0);
    unsigned short mainGroup;
    unsigned short subGroup;
    unsigned short index;
    bool operator==(const Dpt& other) const;
    bool operator!=(const Dpt& other) const;
};