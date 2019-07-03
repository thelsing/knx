#include "dpt.h"

Dpt::Dpt()
{}

Dpt::Dpt(short mainGroup, short subGroup, short index /* = 0 */)
    : mainGroup(mainGroup), subGroup(subGroup), index(index)
{}

bool Dpt::operator==(const Dpt& other) const
{
    return other.mainGroup == mainGroup && other.subGroup == subGroup && other.index == index;
}

bool Dpt::operator!=(const Dpt& other) const
{
    return !(other == *this);
}
