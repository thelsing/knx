#include "knx_facade.h"

#include "knx/bau091A.h"
#include "knx/bau2920.h"

#include "knx/bits.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sched.h>
#include <sys/mman.h>

#include "fdsk.h"

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

#if MASK_VERSION == 0x091A
KnxFacade<LinuxPlatform, Bau091A> knx; // IP/TP1 coupler
#elif MASK_VERSION == 0x2920
KnxFacade<LinuxPlatform, Bau2920> knx; // TP1/RF coupler
#else
#error Mask version not supported yet!
#endif

void appLoop()
{
    if (!knx.configured())
        return;
}

void setup()
{
    srand((unsigned int)time(NULL));
    knx.readMemory();

    if (knx.individualAddress() == 0)
        knx.progMode(true);

    if (knx.configured())
    {
        printf("configured %d\n", knx.paramByte(5));
    }
    else
        println("not configured");
    knx.start();
}

int main(int argc, char **argv)
{
    printf("main() start.\n");

    uint8_t serialNumber[] = { 0x00, 0xFA, 0x01, 0x02, 0x03, 0x04};
    uint8_t key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

    FdskCalculator calc;
    char fdskString[42]; // 6 * 6 chars + 5 dashes + nullbyte = 42
    calc.snprintFdsk(fdskString, sizeof(fdskString), serialNumber, key);
    printf("FDSK: %s\n", fdskString);

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
#ifdef USE_RF
    gpio_unexport(SPI_SS_PIN);
    gpio_unexport(GPIO_GDO2_PIN);
    gpio_unexport(GPIO_GDO0_PIN);
#endif
    printf("main() exit.\n");
}
