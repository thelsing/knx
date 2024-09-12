#include "dpt.h"

#include "../../util/logger.h"
#include "../../bits.h"

#define LOGGER Logger::logger("Dpt")

namespace Knx
{
    Dpt::Dpt()
    {}

    Dpt::Dpt(short mainGroup, short subGroup, short index /* = 0 */)
        : mainGroup(mainGroup), subGroup(subGroup), index(index)
    {
        if (subGroup == 0)
            println("WARNING: You used and invalid Dpt *.0");
    }
}