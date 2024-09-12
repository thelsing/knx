#include <knx.h>

#include "knx/ip/bau57B0.h"
#include "knx/rf/bau27B0.h"
#include "knx/tp/bau07B0.h"

#include "knx/interface_object/group_object_table_object.h"
#include "knx/bits.h"
#include "knx/group_object/dpt/dpts.h"

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sched.h>
#include <sys/mman.h>

#include "fdsk.h"

#define LOGGER Logger::logger("App")

volatile sig_atomic_t loopActive = 1;
void signalHandler(int sig)
{
  (void)sig;

  // can be called asynchronously
  loopActive = 0;
}

bool sendHidReport(uint8_t* data, uint16_t length)
{
    return false;
}
bool isSendHidReportPossible()
{
    return false;
}

#if MASK_VERSION == 0x57B0
KnxFacade<LinuxPlatform, Bau57B0> knx;
#elif MASK_VERSION == 0x27B0
KnxFacade<LinuxPlatform, Bau27B0> knx;
#elif MASK_VERSION == 0x07B0
KnxFacade<LinuxPlatform, Bau07B0> knx;
#else
#error Mask version not supported yet!
#endif

long lastsend = 0;

#define GO_CURR knx.getGroupObject(1)
#define GO_MAX knx.getGroupObject(2)
#define GO_MIN knx.getGroupObject(3)
#define GO_RESET knx.getGroupObject(4)

void measureTemp()
{
    long now = millis();
    if ((now - lastsend) < 10000)
        return;

    lastsend = now;
    int r = rand();
    float currentValue = (r * 1.0f) / (RAND_MAX * 1.0f);
    currentValue *= 100;
    currentValue -= 50;
    //    currentValue *= (670433.28 + 273);
    //    currentValue -= 273;
    LOGGER.info("current value: %f",currentValue);
    GO_CURR.value<Dpt9>(currentValue);

    float max = GO_MAX.value<Dpt9>();
    if (currentValue > max)
        GO_MAX.value<Dpt9>(currentValue);

    if (currentValue < GO_MIN.value<Dpt9>())
        GO_MIN.value<Dpt9>(currentValue);
}

void handler(GroupObject& go)
{
    if (go == GO_RESET && go.value<Dpt1>())
    {
        GO_MAX.valueNoSend<Dpt9>(-273.0f);
        GO_MIN.valueNoSend<Dpt9>(670433.28f);
    }
}

void appLoop()
{
    if (!knx.configured())
        return;
    
    measureTemp();
}

void setup()
{
    srand((unsigned int)time(NULL));

    Logger::logLevel("App", Logger::Info);
    Logger::logLevel("TableObject", Logger::Info);
    Logger::logLevel("Memory", Logger::Info);
    knx.readMemory();

    if (knx.individualAddress() == 0xFFFF)
        knx.progMode(true);

    GroupObject::classCallback(handler);

    if (knx.configured())
    {
        GO_MIN.value<Dpt9>(670433.28f);
        GO_MAX.valueNoSend<Dpt9>(-273.0f);
        LOGGER.info("Startverzögerung s: %d", knx.paramByte(0));
        LOGGER.info("Aenderung senden (*0.1K): %d", knx.paramByte(1));
        LOGGER.info("Zykl. senden min: %d", knx.paramByte(2));
        LOGGER.info("Min/Max senden: %d", knx.paramByte(3));
        LOGGER.info("Abgleich %d", knx.paramInt(4));
    }
    else
        LOGGER.info("not configured");
    knx.start();
}

int main(int argc, char **argv)
{
    LOGGER.info("main() start.");

    uint8_t serialNumber[] = { 0x00, 0xFA, 0x01, 0x02, 0x03, 0x04};
    uint8_t key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

    FdskCalculator calc;
    char fdskString[42]; // 6 * 6 chars + 5 dashes + nullbyte = 42
    calc.snprintFdsk(fdskString, sizeof(fdskString), serialNumber, key);
    LOGGER.info("FDSK: %s", fdskString);

    // Prevent swapping of this process
    struct sched_param sp;
    memset(&sp, 0, sizeof(sp));
    sp.sched_priority = sched_get_priority_max(SCHED_FIFO);
    sched_setscheduler(0, SCHED_FIFO, &sp);
    mlockall(MCL_CURRENT | MCL_FUTURE);

    // Register signals
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    knx.platform().cmdLineArgs(argc, argv);

    setup();
    
    while (loopActive)
    {
        knx.loop();
        if(knx.configured())
            appLoop();
        delayMicroseconds(100);
    }

    // pinMode() will automatically export GPIO pin in sysfs
    // Read or writing the GPIO pin for the first time automatically
    // opens the "value" sysfs file to read or write the GPIO pin value.
    // The following calls will close the "value" sysfs fiel for the pin
    // and unexport the GPIO pin.
    LOGGER.info("main() exit.");
}
