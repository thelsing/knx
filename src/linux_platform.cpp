#include "linux_platform.h"
#ifdef __linux__
#include <cstdio>
#include <string>
#include <cstring>
#include <cstdlib>
#include <stdexcept>
#include <cmath>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>

#include "knx/device_object.h"
#include "knx/address_table_object.h"
#include "knx/association_table_object.h"
#include "knx/group_object_table_object.h"
#include "knx/application_program_object.h"
#include "knx/ip_parameter_object.h"
#include "knx/bits.h"

#define MAX_MEM 4096

LinuxPlatform::LinuxPlatform(int argc, char** argv)
{
    Platform::_memoryReference = (uint8_t*)malloc(MAX_MEM);
    _currentMaxMem = Platform::_memoryReference;
    _args = new char*[argc + 1];
    memcpy(_args, argv, argc * sizeof(char*));
    _args[argc] = 0;
}

LinuxPlatform::~LinuxPlatform()
{
    delete[] _args;
}

uint32_t LinuxPlatform::currentIpAddress()
{
    return 0;
}

uint32_t LinuxPlatform::currentSubnetMask()
{
    return 0;
}

uint32_t LinuxPlatform::currentDefaultGateway()
{
    return 0;
}

uint32_t LinuxPlatform::millis()
{
    struct timespec spec;

    clock_gettime(CLOCK_MONOTONIC, &spec);
    return spec.tv_sec * 1000 + round(spec.tv_nsec / 1.0e6);
}

void LinuxPlatform::mdelay(uint32_t millis)
{
    struct timespec ts;
    ts.tv_sec = millis / 1000;
    ts.tv_nsec = (millis % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

void LinuxPlatform::macAddress(uint8_t* data)
{
    // hardcode some address
    data[0] = 0x08;
    data[1] = 0x00;
    data[2] = 0x27;
    data[3] = 0x6c;
    data[4] = 0xa8;
    data[5] = 0x2a;
}

void LinuxPlatform::restart()
{
    execv(_args[0], _args);
}

void LinuxPlatform::fatalError()
{
    printf("A fatal error occured. Stopping.\n");
    while (true)
        sleep(1);
}

void LinuxPlatform::setupMultiCast(uint32_t addr, uint16_t port)
{
    _multicastAddr = addr;
    _port = port;

    struct ip_mreq command;
    uint32_t loop = 1;

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(port);

    _socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (_socketFd == -1) {
        perror("socket()");
        fatalError();
    }

    /* Mehr Prozessen erlauben, denselben Port zu nutzen */
    loop = 1;
    if (setsockopt(_socketFd, SOL_SOCKET, SO_REUSEADDR, &loop, sizeof(loop)) < 0)
    {
        perror("setsockopt:SO_REUSEADDR");
        fatalError();
    }

    if (bind(_socketFd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        perror("bind");
        fatalError();
    }

    /* loopback */
    loop = 0;
    if (setsockopt(_socketFd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)) < 0)
    {
        perror("setsockopt:IP_MULTICAST_LOOP");
        fatalError();
    }

    /* Join the broadcast group: */
    command.imr_multiaddr.s_addr = htonl(addr);
    command.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(_socketFd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &command, sizeof(command)) < 0)
    {
        perror("setsockopt:IP_ADD_MEMBERSHIP");
        fatalError();
    }

    uint32_t flags = fcntl(_socketFd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(_socketFd, F_SETFL, flags);
}

void LinuxPlatform::closeMultiCast()
{
    struct ip_mreq command;
    command.imr_multiaddr.s_addr = htonl(_multicastAddr);
    command.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(_socketFd,
        IPPROTO_IP,
        IP_DROP_MEMBERSHIP,
        &command, sizeof(command)) < 0) {
        perror("setsockopt:IP_DROP_MEMBERSHIP");
    }
    close(_socketFd);
}

bool LinuxPlatform::sendBytes(uint8_t* buffer, uint16_t len)
{
    struct sockaddr_in address = { 0 };
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(_multicastAddr);
    address.sin_port = htons(_port);

    ssize_t retVal = 0;
    do
    {
        retVal = sendto(_socketFd, buffer, len, 0, (struct sockaddr *) &address, sizeof(address));
        if (retVal == -1)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
                return false;
        }
    } while (retVal == -1);
//    printHex("<-", buffer, len);
    return true;
}

int LinuxPlatform::readBytes(uint8_t * buffer, uint16_t maxLen)
{
    uint32_t sin_len;
    struct sockaddr_in sin;

    sin_len = sizeof(sin);
    ssize_t len = recvfrom(_socketFd, buffer, maxLen, 0, (struct sockaddr *) &sin, &sin_len);
//    if (len > 0)
//        printHex("->", buffer, len);
    
    return len;
}

uint8_t * LinuxPlatform::getEepromBuffer(uint16_t size)
{
    if (_fd < 0)
        doMemoryMapping();
    
    return _mappedFile + 2;
}

void LinuxPlatform::commitToEeprom()
{
    if (_fd < 0)
        doMemoryMapping();
    
    fsync(_fd);
}

#define FLASHSIZE 0x10000
void LinuxPlatform::doMemoryMapping()
{
    _fd = open(_flashFilePath.c_str(), O_RDWR | O_CREAT, S_IRWXU | S_IRGRP | S_IROTH);
    if (_fd < 0)
    {
        puts("Error in file opening");
        //exit(-1);
    }

    struct stat st;
    uint32_t ret = fstat(_fd, &st);
    if (ret < 0)
    {
        puts("Error in fstat");
        //exit(-1);
    }

    size_t len_file = st.st_size;
    if (len_file < FLASHSIZE)
    {
        if (ftruncate(_fd, FLASHSIZE) != 0)
        {
            puts("Error extending file");
            //exit(-1);
        }
        len_file = FLASHSIZE;
    }
    unsigned char* addr = (unsigned char*)mmap(NULL, len_file, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0);
    if (addr[0] != 0xAF || addr[1] != 0xFE)
    {
        memset(addr, 0, FLASHSIZE);
        addr[0] = 0xAF;
        addr[1] = 0xFE;
    }

    if (addr == MAP_FAILED)
    {
        puts("Error in mmap");
        //exit(-1);
    }
    _mappedFile = addr;
}

size_t LinuxPlatform::readBytesUart(uint8_t *buffer, size_t length)
{
    return 0;
}


int LinuxPlatform::readUart()
{
    return -1;
}


size_t LinuxPlatform::writeUart(const uint8_t *buffer, size_t size)
{
    return 0;
}


size_t LinuxPlatform::writeUart(const uint8_t data)
{
    return 0;
}


int LinuxPlatform::uartAvailable()
{
    return 0;
}


void LinuxPlatform::closeUart()
{
}


void LinuxPlatform::setupUart()
{
}

/*
 * On linux the memory addresses from malloc may be to big for usermermory_write.
 * So we allocate some memory at the beginning and use it for address table, group object table etc.
 *
 **/
uint8_t* LinuxPlatform::allocMemory(size_t size)
{
    uint8_t* addr = _currentMaxMem;
    _currentMaxMem += size;
    if ((_currentMaxMem - Platform::_memoryReference) > MAX_MEM)
        throw std::overflow_error("MAX_MEM was to small");
    return addr;
}

void LinuxPlatform::freeMemory(uint8_t* ptr)
{
    /* do nothing. Memory is freed on restart()*/
}

void LinuxPlatform::flashFilePath(const std::string path)
{
    _flashFilePath = path;
}


std::string LinuxPlatform::flashFilePath()
{
    return _flashFilePath;
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

#endif