#include "stm32_platform.h"
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include <knx/bits.h>
#include <string.h>
#include <ring_buffer.h>
#include <main.h>


#ifdef STM32L432xx
#define USER_MEMORY_ADDR 0x0803F800
#define USER_MEMORY_SIZE 0x800
#define USER_MEMORY_PAGE 127

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
#define TPUART &huart1
#define TPUART_UART USART1
#define CONSOLE_HUART &huart2
#define CONSOLE_UART USART2
#endif

// #define DEBUG_TPUSART_COMMUNICATION_RX
// #define DEBUG_TPUSART_COMMUNICATION_TX
// #define DEBUG_MEMORY_SIZE 128

#define RX_BUFFER_LEN 64
#define TX_BUFFER_LEN 64
#define DBG_BUFFER_LEN 1024


static uint8_t RX_BUF[RX_BUFFER_LEN];
static uint8_t TX_BUF[TX_BUFFER_LEN];
static uint8_t DBG_BUF[DBG_BUFFER_LEN];
static struct ring_buffer rx_buffer = BUFFER_STATIC_INIT(RX_BUF, RX_BUFFER_LEN);
static struct ring_buffer tx_buffer = BUFFER_STATIC_INIT(TX_BUF, TX_BUFFER_LEN);
static struct ring_buffer dbg_buffer = BUFFER_STATIC_INIT(DBG_BUF, DBG_BUFFER_LEN);
static uint8_t __USER_MEMORY[USER_MEMORY_SIZE];

volatile static int tx_it_enable = 0;
static unsigned char rx_byte;

#include <stdint.h>


uint32_t millis() {
    return HAL_GetTick();
}

extern "C" {
    int _write(int file, char *ptr, int len);
    void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle);
}
volatile static bool dbg_xfer_in_progress = false;
static uint8_t dbg_data_to_xfer = 0;
static inline void dbg_start_next_xfer()
{
    if(buffer_length(&dbg_buffer) > 0)
    {
        fifo_pop(&dbg_buffer, &dbg_data_to_xfer);
        HAL_UART_Transmit_IT(CONSOLE_HUART, &dbg_data_to_xfer, 1);
        dbg_xfer_in_progress = true;
    }
    else
    {
        dbg_xfer_in_progress = false;
    }
}

volatile static bool knx_xfer_in_progress = false;
static uint8_t knx_data_to_xfer = 0;
static inline void knx_start_next_xfer()
{
    if(buffer_length(&tx_buffer) > 0)
    {
        fifo_pop(&tx_buffer, &knx_data_to_xfer);
        HAL_UART_Transmit_IT(TPUART, &knx_data_to_xfer, 1);
        knx_xfer_in_progress = true;
    }
    else
    {
        knx_xfer_in_progress = false;
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
    /* Set transmission flag: transfer complete */
    if(UartHandle->Instance == CONSOLE_UART)
    {
        dbg_start_next_xfer();
    }
    if(UartHandle->Instance == TPUART_UART)
    {
        knx_start_next_xfer();
    }
}

int _write(int file, char *ptr, int len)
{
    int i;
    if (file == STDOUT_FILENO || file == STDERR_FILENO) {
        for (i = 0; i < len; i++) {
            if (ptr[i] == '\n') {
                while(fifo_push(&dbg_buffer, '\r') == -1);
            }
            while(fifo_push(&dbg_buffer, static_cast<uint8_t>(ptr[i])) == -1);
            if(!dbg_xfer_in_progress) dbg_start_next_xfer();
        }
        return i;
    }
    errno = EIO;
    return -1;
}

void delay(uint32_t delay)
{
    uint32_t wake = HAL_GetTick() + delay;
    while (wake > HAL_GetTick());
}

void tpuart_usart_setup(void)
{
    if(HAL_UART_Receive_IT(TPUART, (unsigned char *)(&rx_byte), 1) != HAL_OK)
    {
        printf("!!! Error on tpuart_usart_setup, Can't HAL_UART_Receive_IT in start_tpuart_reception\n");
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
#ifdef DEBUG_TPUSART_COMMUNICATION_RX
    print('<');
#endif
    fifo_push(&rx_buffer, rx_byte);
    if(HAL_UART_Receive_IT(TPUART, (unsigned char *)(&rx_byte), 1) != HAL_OK)
    {
        printf("!!! Error on start_next_telegram, Can't HAL_UART_Receive_IT in start_tpuart_reception\n");
    }
}

void tpuart_send(uint8_t data)
{
#ifdef DEBUG_TPUSART_COMMUNICATION_TX
    print('>');
#endif
    while(fifo_push(&tx_buffer, data) == -1);
    if(!knx_xfer_in_progress) knx_start_next_xfer();
}

uint32_t tpuart_rx_size()
{
    return buffer_length(&rx_buffer);
}

uint32_t tpuart_tx_size()
{
    return buffer_length(&rx_buffer);
}

uint8_t tpuart_get()
{
    uint8_t data = 0;
    fifo_pop(&rx_buffer, &data);
    return data;
}


static void flash_read_data(uint32_t start_address, uint16_t num_elements, uint8_t *output_data)
{
    uint16_t iter;
    uint32_t *memory_ptr= (uint32_t*)start_address;

    for(iter=0; iter<num_elements/4; iter++)
    {
        *(uint32_t*)output_data = *(memory_ptr + iter);
        output_data += 4;
    }
}


Stm32Platform::Stm32Platform() : Platform()
{

}

Stm32Platform::~Stm32Platform()
{

}

void Stm32Platform::commitToEeprom()
{
#ifdef DEBUG_MEMORY_SIZE
    printf("commitToEeprom()\n");
    for(int i = 0; i < DEBUG_MEMORY_SIZE; i++)
    {
        printf("%02X ", *(((uint8_t*)&__USER_MEMORY) + i));
        if(i%16 == 15) printf("\n");
    }
    printf("\n");
#endif

    FLASH_EraseInitTypeDef erase_page {
        FLASH_TYPEERASE_PAGES,
        FLASH_BANK_1,
        USER_MEMORY_PAGE,
        1
    };
    uint32_t error;

    printf("HAL_FLASH_Unlock()\n");
    HAL_FLASH_Unlock();
    printf("HAL_FLASHEx_Erase()\n");
    HAL_FLASHEx_Erase(&erase_page, &error);
    printf("HAL_FLASHEx_Erase(): %li\n", error);
    printf("HAL_FLASH_Program()\n");
    delay(200);

    for (int i = 0; i < (USER_MEMORY_SIZE / 8); i++)
    {

        uint64_t dw = ((uint64_t)__USER_MEMORY[(i*8) + 0])
                    | ((uint64_t)__USER_MEMORY[(i*8) + 1]) << 8
                    | ((uint64_t)__USER_MEMORY[(i*8) + 2]) << 16
                    | ((uint64_t)__USER_MEMORY[(i*8) + 3]) << 24
                    | ((uint64_t)__USER_MEMORY[(i*8) + 4]) << 32
                    | ((uint64_t)__USER_MEMORY[(i*8) + 5]) << 40
                    | ((uint64_t)__USER_MEMORY[(i*8) + 6]) << 48
                    | ((uint64_t)__USER_MEMORY[(i*8) + 7]) << 56;
        uint32_t addr = (USER_MEMORY_ADDR + (i*8));
        printf("\tWrite 0x%llx at 0x%lx\n", dw, addr);
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr, dw) != HAL_OK)
        {
            printf("ERROR during flash\n");
            return;
        }
    }

    //HAL_FLASH_Program(FLASH_TYPEPROGRAM_FAST_AND_LAST, USER_MEMORY_ADDR, (uint64_t)&__USER_MEMORY);

    printf("HAL_FLASH_Lock()\n");
    HAL_FLASH_Lock();

#ifdef DEBUG_MEMORY_SIZE
    printf("commitToEeprom() OK\n");
    printf("FIRST BYTES: 0x%X\n", *((uint8_t*)USER_MEMORY_ADDR));
#endif
}

uint8_t *Stm32Platform::getEepromBuffer(uint16_t size)
{
    if(!m_initialized)
    {
        flash_read_data(USER_MEMORY_ADDR, USER_MEMORY_SIZE, (uint8_t*)&__USER_MEMORY);
#ifdef DEBUG_MEMORY_SIZE
        printf("readFromEeprom()\n");
        for(int i = 0; i < DEBUG_MEMORY_SIZE; i++)
        {
            printf("%02X ", *(((uint8_t*)&__USER_MEMORY) + i));
            if(i%16 == 15) printf("\n");
        }
        printf("\n");
#endif
        m_initialized = true;
    }

    if(size > USER_MEMORY_SIZE)
        fatalError();

    return __USER_MEMORY;
}

size_t Stm32Platform::readBytesUart(uint8_t *buffer, size_t length)
{
    return 0;
}

size_t Stm32Platform::writeUart(const uint8_t *buffer, size_t size)
{
    size_t i = 0;
    for(i = 0; i < size; i++)
    {
        tpuart_send(*(buffer + i));
    }
    return size;
}

void Stm32Platform::closeUart()
{

}

void Stm32Platform::setupUart()
{
    tpuart_usart_setup();
}

int Stm32Platform::readBytes(uint8_t *buffer, uint16_t maxLen)
{
    return 0;
}

bool Stm32Platform::sendBytes(uint8_t *buffer, uint16_t len)
{
    return false;
}

void Stm32Platform::closeMultiCast()
{

}

void Stm32Platform::setupMultiCast(uint32_t addr, uint16_t port)
{

}

void Stm32Platform::fatalError()
{

}

void Stm32Platform::restart()
{
    HAL_NVIC_SystemReset();
}

void Stm32Platform::macAddress(uint8_t *data)
{

}

uint32_t Stm32Platform::currentDefaultGateway()
{
    return 0;
}

uint32_t Stm32Platform::currentSubnetMask()
{
    return 0;
}

int Stm32Platform::uartAvailable()
{
    return (tpuart_rx_size() > 0);
}

size_t Stm32Platform::writeUart(const uint8_t data)
{
    tpuart_send(data);
    return 1;
}

int Stm32Platform::readUart()
{
    uint64_t m = millis() + 100;

    while((millis() < m) && (buffer_length(&rx_buffer) < 1));
    if(buffer_length(&rx_buffer) < 1)
    {
#ifdef DEBUG_TPUSART_COMMUNICATION_RX
        print('T');
#endif
        return 0;
    }
#ifdef DEBUG_TPUSART_COMMUNICATION_RX
    print('R');
#endif
    return tpuart_get();
}

void digitalWrite(uint32_t dwPin, uint32_t dwVal)
{
    if(dwPin == 1)
    {
        /* Prog led */
        if(dwVal == 0)
            HAL_GPIO_WritePin(KNX_PROG_LED_GPIO_Port, KNX_PROG_LED_Pin, GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(KNX_PROG_LED_GPIO_Port, KNX_PROG_LED_Pin, GPIO_PIN_RESET);
    }
}

void pinMode(uint32_t dwPin, uint32_t dwMode)
{
}

typedef void (*voidFuncPtr)(void);
void attachInterrupt(uint32_t pin, voidFuncPtr callback, uint32_t mode)
{

}

void print(const char* s)
{
    printf("%s", s);
}
void print(char c)
{
    printf("%c", c);
}

void print(unsigned char num)
{
    print(num, DEC);
}

void print(unsigned char num, int base)
{
    if (base == HEX)
        printf("%X", num);
    else
        printf("%d", num);
}

void print(int num)
{
    print(num, DEC);
}

void print(int num, int base)
{
    if (base == HEX)
        printf("%X", num);
    else
        printf("%d", num);
}

void print(unsigned int num)
{
    print(num, DEC);
}

void print(unsigned int num, int base)
{
    if (base == HEX)
        printf("%X", num);
    else
        printf("%d", num);
}

void print(long num)
{
    print(num, DEC);
}

void print(long num, int base)
{
    if (base == HEX)
        printf("%lX", num);
    else
        printf("%ld", num);
}

void print(unsigned long num)
{
    print(num, DEC);
}

void print(unsigned long num, int base)
{
    if (base == HEX)
        printf("%lX", num);
    else
        printf("%ld", num);
}

void print(double num)
{
    printf("%f", num);
}

void println(const char* s)
{
    printf("%s\n", s);
}
void println(char c)
{
    printf("%c\n", c);
}

void println(unsigned char num)
{
    println(num, DEC);
}

void println(unsigned char num, int base)
{
    if (base == HEX)
        printf("%X\n", num);
    else
        printf("%d\n", num);
}

void println(int num)
{
    println(num, DEC);
}

void println(int num, int base)
{
    if (base == HEX)
        printf("%X\n", num);
    else
        printf("%d\n", num);
}

void println(unsigned int num)
{
    println(num, DEC);
}

void println(unsigned int num, int base)
{
    if (base == HEX)
        printf("%X\n", num);
    else
        printf("%d\n", num);
}

void println(long num)
{
    println(num, DEC);
}

void println(long num, int base)
{
    if (base == HEX)
        printf("%lX\n", num);
    else
        printf("%ld\n", num);
}

void println(unsigned long num)
{
    println(num, DEC);
}

void println(unsigned long num, int base)
{
    if (base == HEX)
        printf("%lX\n", num);
    else
        printf("%ld\n", num);
}

void println(double num)
{
    printf("%f\n", num);
}

void println(double num, int places)
{
    printf("%f\n", num);
}

void println(void)
{
    printf("\n");
}

uint32_t Stm32Platform::currentIpAddress()
{
    return 0;
}

void Stm32Platform::setupSpi()
{

}

void Stm32Platform::closeSpi()
{

}

int Stm32Platform::readWriteSpi (uint8_t *data, size_t len)
{
    return 0;
}
