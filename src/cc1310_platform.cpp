#ifdef DeviceFamily_CC13X0

#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cmath>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)
#include <ti/drivers/GPIO.h>

#include "SEGGER_RTT.h"

#include "Board.h"

#include "knx/bits.h"
#include "cc1310_platform.h"

//#define printf(args...) (SEGGER_RTT_printf(0, args))
//#define PRINT_RTT
#define PRINT_UART

static uint8_t serialNumber[6];
// KNX_FLASH_SIZE shall be defined in CMakeLists.txt for example. It is also used in class Memory in memory.cpp
static uint8_t NVS_buffer[KNX_FLASH_SIZE];

static UART_Handle uart;

static NVS_Handle nvsHandle;

static ClockP_Handle clk0Handle;
static ClockP_Struct clk0Struct;
static volatile uint32_t msCounter = 0;

static void clk0Fxn(uintptr_t arg0)
{
    msCounter++;
}

static void setupClock()
{
    ClockP_Params clkParams;
    ClockP_Params_init(&clkParams);
    clkParams.period = 1000/ClockP_tickPeriod;
    clkParams.startFlag = true;
    ClockP_construct(&clk0Struct, (ClockP_Fxn)clk0Fxn, 1000/ClockP_tickPeriod, &clkParams);
    clk0Handle = ClockP_handle(&clk0Struct);
}

static void setupGPIO()
{
    /* Configure the LED and button pins */
    GPIO_setConfig(Board_GPIO_LED0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(Board_GPIO_LED1, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(Board_GPIO_BUTTON0, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);
    GPIO_setConfig(Board_GPIO_BUTTON1, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);
}

static void setupUART()
{
     UART_Params uartParams;
     UART_Params_init(&uartParams);
     uartParams.writeDataMode = UART_DATA_BINARY;
     uartParams.readDataMode = UART_DATA_BINARY;
     uartParams.readReturnMode = UART_RETURN_FULL;
     uartParams.readEcho = UART_ECHO_OFF;
     uartParams.baudRate = 115200;
     uart = UART_open(Board_UART0, &uartParams);
     if (uart == NULL) 
     {
        while(true)
        {}
     }
 }

static void setupNVS()
{
    NVS_Params nvsParams;
    NVS_Params_init(&nvsParams);
    nvsHandle = NVS_open(Board_NVSINTERNAL, &nvsParams);
    if (nvsHandle == NULL) 
    {
        println("NVS_open() failed.");
        return;
    }

    NVS_Attrs attrs;
    NVS_getAttrs(nvsHandle, &attrs);
    print("NVS flash size: "); println((int)attrs.regionSize);
    print("NVS flash sector size: "); println((int)attrs.sectorSize);

    if (GPIO_read(Board_GPIO_BUTTON1) == 0)
    {
        println("Button1 is pressed. Erasing flash...");
        int_fast16_t result = NVS_erase(nvsHandle, 0, attrs.regionSize);
        if (result != NVS_STATUS_SUCCESS)
        {
            print("Error erasing NVS, result: "); println(result);
        }
        else
        {
            println("NVS successfully erased.");
        }
    }
}

void sleep(uint32_t sec)
{
    ClockP_sleep(sec);
}

void usleep(uint32_t usec)
{
    ClockP_usleep(usec);
}

uint32_t millis()
{
    // we use our own ms clock because the Os tick counter has counts 10us ticks and following calculation would not wrap correctly at 32bit boundary
    //return Clock_getTicks() * (uint64_t) Clock_tickPeriod / 1000; // rtos
    //return ClockP_getTicks( * (uint64_t) Clock_tickPeriod / 1000); //nortos
    return msCounter;
}

void delay(uint32_t ms)
{
    ClockP_usleep(ms * 1000);
    //sleep(ms * (1000 / ClockP_tickPeriod));   //rtos
    //sleepTicks(millis * 1000ULL / ClockP_tickPeriod); //nortos
}

void delayMicroseconds (unsigned int howLong)
{
    ClockP_usleep(howLong);
}

size_t write(uint8_t c)
{
#if defined(PRINT_UART)
    uint8_t buffer[1] = {c};
    return UART_write(uart, buffer, sizeof(buffer));
#elif defined (PRINT_RTT)    
    return SEGGER_RTT_PutChar(0, (char)c);
#else
    return 1;    
#endif
}

#if 0
size_t write(const uint8_t *buffer, size_t size)
{
    size_t n = 0;
    while (size--) 
    {
        if (write(*buffer++)) 
        {
            n++;
        }
        else 
        {
            break;
        }
    }
    return n;
}
#else
size_t write(const uint8_t *buffer, size_t size)
{
#if defined(PRINT_UART)
    return UART_write(uart, buffer, size);
#elif defined (PRINT_RTT)    
    return SEGGER_RTT_Write(0, buffer, size);
#else
    return size;    
#endif
}
#endif

size_t write(const char *buffer, size_t size) 
{
    return write((const uint8_t *)buffer, size);
}

void print(const char* s)
{
    if (s == NULL) 
    {
        return;
    }
    write(s, strlen(s));
}
void print(char c)
{
    write(c);
}

void printUint64(uint64_t value, int base = DEC)
  {
    char buf[8 * sizeof(uint64_t) + 1];
    char* str = &buf[sizeof(buf) - 1];
    *str = '\0';

    uint64_t n = value;
    do {
      char c = n % base;
      n /= base;

      *--str = c < 10 ? c + '0' : c + 'A' - 10;
    } while (n > 0);

     print(str);
}

void print(long long num, int base)
{
    if (base == 0) 
    {
        write(num);
        return;
    }
    else if (base == 10) 
    {
        if (num < 0) 
        {
            print('-');
            num = -num;
            printUint64(num, 10);
            return;
        }
        printUint64(num, 10);
        return;
    } 
    else 
    {
        printUint64(num, base);
        return;
    }
}

void print(unsigned long long num, int base)
{
    if (base == 0) 
    {
        write(num);
        return;
    }
    else 
    {
        printUint64(num, base);   
        return;
    }
}

void print(unsigned char num, int base)
{
    print((unsigned long long)num, base);
}

void print(int num, int base)
{
    print((long long)num, base);
}

void print(unsigned int num, int base)
{
    print((unsigned long long)num, base);
}

void print(long num, int base)
{
    print((long long)num, base);
}

void print(unsigned long num, int base)
{
    print((unsigned long long)num, base);
}

void printFloat(double number, uint8_t digits)
{
    if (std::isnan(number)) 
    {
        print("nan");
        return;
    } 
    if (std::isinf(number)) 
    {
        print("inf");
        return;
    }
    if (number > 4294967040.0) 
    {
        print("ovf");  // constant determined empirically
        return;
    }
    if (number <-4294967040.0)
    {
        print("ovf");  // constant determined empirically
        return;
    } 

    // Handle negative numbers
    if (number < 0.0)
    {
        print('-');
        number = -number;
    }

    // Round correctly so that print(1.999, 2) prints as "2.00"
    double rounding = 0.5;
    for (uint8_t i=0; i<digits; ++i)
    rounding /= 10.0;

    number += rounding;

    // Extract the integer part of the number and print it
    unsigned long int_part = (unsigned long)number;
    double remainder = number - (double)int_part;
    printUint64(int_part);

    // Print the decimal point, but only if there are digits beyond
    if (digits > 0) 
    {
        print('.');
    }

    // Extract digits from the remainder one at a time
    while (digits-- > 0)
    {
        remainder *= 10.0;
        unsigned int toPrint = (unsigned int)(remainder);
        printUint64(toPrint);
        remainder -= toPrint;
    }
}

void print(double num, int digits = 2)
{
    printFloat(num, digits);
}

void println(void)
{
    print("\r\n");
}

void println(const char* s)
{
    print(s);
    println();
}
void println(char c)
{
    print(c);
    println();
}

void println(unsigned char num, int base)
{
    print(num, base);
    println();
}

void println(int num, int base)
{
    print(num, base);
    println();
}

void println(unsigned int num, int base)
{
    print(num, base);
    println();
}

void println(long num, int base)
{
    print(num, base);
    println();
}

void println(unsigned long num, int base)
{
    print(num, base);
    println();
}

void println(unsigned long long num, int base)
{
    printUint64(num, base);
    println();
}

void println(double num, int digits = 2)
{
    print(num, digits);
    println();
}

void println(double num)
{
    // default: print 10 digits
    println(num, 10);
}

uint32_t digitalRead(uint32_t dwPin)
{
    print("ignoring digitalRead: pin: ");print(dwPin);
    println(", returning 0");
    return 0;
}

void digitalWrite(unsigned long pin, unsigned long value)
{
    if (pin == Board_GPIO_LED0)
    {
        if (value > 0)
        {
            GPIO_write(Board_GPIO_LED0, Board_GPIO_LED_ON);
        }
        else
        {
            GPIO_write(Board_GPIO_LED0, Board_GPIO_LED_OFF);
        }
    }
    else
    {
        print("dummy digitalWrite: pin: ");print(pin);
        print(", value: ");println(value, HEX);
    }
}

void pinMode(unsigned long pin, unsigned long mode)
{
    print("ignoring pinMode: pin: ");print(pin);
    print(", mode: ");println(mode, HEX);
}

typedef void (*IsrFuncPtr)();
static IsrFuncPtr gpioCallback;
static void gpioButtonFxn0(uint_least8_t index)
{
    gpioCallback();
}

void attachInterrupt(uint32_t pin, IsrFuncPtr callback, uint32_t mode) 
{
    if (pin == Board_GPIO_BUTTON0)
    {
        gpioCallback = callback;
        /* install Button callback */
        GPIO_setCallback(Board_GPIO_BUTTON0, gpioButtonFxn0);

        /* Enable interrupts */
        GPIO_enableInt(Board_GPIO_BUTTON0);
    }
    else
    {
        print("dummy attachInterrupt: pin: ");print(pin);
        print(", mode: ");println(mode, HEX);
    }
}

CC1310Platform::CC1310Platform()
{
    // build serialNumber from IEEE MAC Address (MAC is 8 bytes, serialNumber 6 bytes only)
    *(uint32_t*)(serialNumber+2) = HWREG(FCFG1_BASE+FCFG1_O_MAC_15_4_0) ^ HWREG(FCFG1_BASE+FCFG1_O_MAC_15_4_1);  // make a 6 byte hash from 8 bytes    
}

CC1310Platform::~CC1310Platform()
{
}

void CC1310Platform::init() 
{
    // TI Drivers init
    // According to SDK docs it is safe to call them AFTER NoRTOS_Start()
    // If RTOS is used and multiple thread use the same driver, then the init shall be performed before BIOS_Start()
    GPIO_init();
    UART_init();
    NVS_init();

    // Init GPIO
    setupGPIO();

    // Init UART
    setupUART();

    // tick Period on this controller 10us so we use our own millisecond clock
    setupClock();

    // Init flash
    setupNVS();
}

uint8_t* CC1310Platform::getEepromBuffer(uint16_t size)
{
    if(size > KNX_FLASH_SIZE)
    {
        fatalError();
    }

    NVS_read(nvsHandle, 0, (void *) NVS_buffer, size);

    for (int i=0; i<size; i++)
    {
        if (NVS_buffer[i] != 0)
        {
            return NVS_buffer;
        }
    }

    memset(NVS_buffer, 0xff, size);

    return NVS_buffer;
}

void CC1310Platform::commitToEeprom()
{
    println("CC1310Platform::commitToEeprom() ...");

    int_fast16_t result = NVS_write(nvsHandle, 0, (void *)NVS_buffer, KNX_FLASH_SIZE, NVS_WRITE_ERASE | NVS_WRITE_POST_VERIFY);

    if (result != NVS_STATUS_SUCCESS)
    {
        print("Error writing to NVS, result: "); println(result);
    }
    else
    {
        println("NVS successfully written");
    }

    delay(500);
}

void CC1310Platform::restart()
{
    println("System restart in 500ms.");
    delay(500);
    SysCtrlSystemReset();
    // Should neber be reached!
    fatalError();
}

void CC1310Platform::fatalError()
{
    println("A fatal error occured. Stopped.");
    while(true) 
    {
        /* Turn on user LED */
        GPIO_write(Board_GPIO_LED0, Board_GPIO_LED_OFF);
        GPIO_write(Board_GPIO_LED1, Board_GPIO_LED_ON);
        delay(500);
        GPIO_write(Board_GPIO_LED0, Board_GPIO_LED_ON);
        GPIO_write(Board_GPIO_LED1, Board_GPIO_LED_OFF);
        delay(500);
    }
}

#endif // DeviceFamily_CC13X0
