#include "knx_facade.h"

#include "knx/bau57B0.h"
#include "knx/bau27B0.h"
#include "knx/bau07B0.h"

#include "knx/group_object_table_object.h"
#include "knx/bits.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sched.h>
#include <sys/mman.h>

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
#if MEDIUM_TYPE == 5
KnxFacade<LinuxPlatform, Bau57B0> knx;
#elif MEDIUM_TYPE == 2
KnxFacade<LinuxPlatform, Bau27B0> knx;
#else
#error Only MEDIUM_TYPE IP and RF supported
#endif

long lastsend = 0;

#define CURR knx.getGroupObject(1)
#define MAX knx.getGroupObject(2)
#define MIN knx.getGroupObject(3)
#define RESET knx.getGroupObject(4)

void measureTemp()
{
    long now = millis();
    if ((now - lastsend) < 10000)
        return;

    lastsend = now;
    int r = rand();
    double currentValue = (r * 1.0) / (RAND_MAX * 1.0);
    currentValue *= 100;
    currentValue -= 50;
    //    currentValue *= (670433.28 + 273);
    //    currentValue -= 273;
    println(currentValue);
    CURR.value(currentValue);

    double max = MAX.value();
    if (currentValue > max)
        MAX.value(currentValue);

    if (currentValue < (double)MIN.value())
        MIN.value(currentValue);
}

void resetCallback(GroupObject& go)
{
    if (go.value())
    {
        MAX.valueNoSend(-273.0);
        MIN.valueNoSend(670433.28);
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
    knx.readMemory();

    if (knx.induvidualAddress() == 0)
        knx.progMode(true);

    if (knx.configured())
    {
        CURR.dataPointType(Dpt(9, 1));
        MIN.dataPointType(Dpt(9, 1));
        MIN.value(670433.28);
        MAX.dataPointType(Dpt(9, 1));
        MAX.valueNoSend(-273.0);
        RESET.dataPointType(Dpt(1, 15));
        RESET.callback(resetCallback);
        printf("Timeout: %d\n", knx.paramWord(0));
        printf("Zykl. senden: %d\n", knx.paramByte(2));
        printf("Min/Max senden: %d\n", knx.paramByte(3));
        printf("Aenderung senden: %d\n", knx.paramByte(4));
        printf("Abgleich %d\n", knx.paramByte(5));
    }
    else
        println("not configured");
    knx.start();
}

int main(int argc, char **argv)
{
    printf("main() start.\n");

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
