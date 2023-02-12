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
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>

#include <sys/ioctl.h>        // Needed for SPI port
#include <linux/spi/spidev.h> // Needed for SPI port
#include <poll.h>             // Needed for GPIO edge detection
#include <sys/time.h>         // Needed for delayMicroseconds()

#include "knx/device_object.h"
#include "knx/address_table_object.h"
#include "knx/association_table_object.h"
#include "knx/group_object_table_object.h"
#include "knx/application_program_object.h"
#include "knx/ip_parameter_object.h"
#include "knx/bits.h"
#include "knx/ip_host_protocol_address_information.h"

#define MAX_MEM 4096

LinuxPlatform::LinuxPlatform()
{
    int socketMac = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (socketMac < 0)
    {
        printf("Lookup socket creation failed");
        return;
    }

    struct ifreq ifr;
    struct ifconf ifc;
    char buf[1024];

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(socketMac, SIOCGIFCONF, &ifc) < 0)
        return;

    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

    for (; it != end; ++it)
    {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(socketMac, SIOCGIFFLAGS, &ifr))
            continue;

        if (ifr.ifr_flags & IFF_LOOPBACK) // don't count loopback
            continue;

        if (ioctl(socketMac, SIOCGIFHWADDR, &ifr))
            continue;

        if (ifr.ifr_hwaddr.sa_family != ARPHRD_ETHER)
            continue;

        memcpy(_macAddress, ifr.ifr_hwaddr.sa_data, IFHWADDRLEN);

        ioctl(socketMac, SIOCGIFADDR, &ifr);

        struct sockaddr_in* ipaddr = (struct sockaddr_in*)&ifr.ifr_addr;
        _ipAddress = ntohl(ipaddr->sin_addr.s_addr);

        //printf("IP address: %s\n", inet_ntoa(ipaddr->sin_addr));
        ioctl(socketMac, SIOCGIFNETMASK, &ifr);
        struct sockaddr_in* netmask = (struct sockaddr_in*)&ifr.ifr_netmask;
        _netmask = ntohl(netmask->sin_addr.s_addr);
        //printf("Netmask: %s\n", inet_ntoa(ipaddr->sin_addr));
        break;
    }
    close(socketMac);

    // default GW
    FILE* f;
    char line[100], *p, *c, *g, *saveptr;

    f = fopen("/proc/net/route", "r");

    while (fgets(line, 100, f))
    {
        p = strtok_r(line, " \t", &saveptr);
        c = strtok_r(NULL, " \t", &saveptr);
        g = strtok_r(NULL, " \t", &saveptr);

        if (p != NULL && c != NULL)
        {
            if (strcmp(c, "00000000") == 0)
            {
                //printf("Default interface is : %s \n" , p);
                if (g)
                {
                    char* pEnd;
                    _defaultGateway = ntohl(strtol(g, &pEnd, 16));
                }
                break;
            }
        }
    }
    fclose(f);
}

LinuxPlatform::~LinuxPlatform()
{
    delete[] _args;
}

uint32_t millis()
{
    struct timespec spec;

    clock_gettime(CLOCK_MONOTONIC, &spec);
    return spec.tv_sec * 1000 + round(spec.tv_nsec / 1.0e6);
}

void delay(uint32_t millis)
{
    struct timespec ts;
    ts.tv_sec = millis / 1000;
    ts.tv_nsec = (millis % 1000) * 1000000;
    nanosleep(&ts, NULL);
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
    if (_multicastSocketFd >= 0)
        closeMultiCast();

    _multicastAddr = addr;
    _multicastPort = port;

    struct ip_mreq command;
    uint32_t loop = 1;

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(port);

    _multicastSocketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (_multicastSocketFd == -1)
    {
        perror("socket()");
        fatalError();
    }

    /* Mehr Prozessen erlauben, denselben Port zu nutzen */
    loop = 1;
    if (setsockopt(_multicastSocketFd, SOL_SOCKET, SO_REUSEADDR, &loop, sizeof(loop)) < 0)
    {
        perror("setsockopt:SO_REUSEADDR");
        fatalError();
    }

    if (bind(_multicastSocketFd, (struct sockaddr*)&sin, sizeof(sin)) < 0)
    {
        perror("bind");
        fatalError();
    }

    /* loopback */
    loop = 0;
    if (setsockopt(_multicastSocketFd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)) < 0)
    {
        perror("setsockopt:IP_MULTICAST_LOOP");
        fatalError();
    }

    /* Join the broadcast group: */
    command.imr_multiaddr.s_addr = htonl(addr);
    command.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(_multicastSocketFd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &command, sizeof(command)) < 0)
    {
        perror("setsockopt:IP_ADD_MEMBERSHIP");
        fatalError();
    }

    uint32_t flags = fcntl(_multicastSocketFd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(_multicastSocketFd, F_SETFL, flags);
}

void LinuxPlatform::closeMultiCast()
{
    struct ip_mreq command;
    command.imr_multiaddr.s_addr = htonl(_multicastAddr);
    command.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(_multicastSocketFd,
                   IPPROTO_IP,
                   IP_DROP_MEMBERSHIP,
                   &command, sizeof(command)) < 0)
    {
        perror("setsockopt:IP_DROP_MEMBERSHIP");
    }
    close(_multicastSocketFd);
    _multicastSocketFd = -1;
}

bool LinuxPlatform::sendBytesMultiCast(uint8_t* buffer, uint16_t len)
{
    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(_multicastAddr);
    address.sin_port = htons(_multicastPort);

    ssize_t retVal = 0;
    do
    {
        retVal = sendto(_multicastSocketFd, buffer, len, 0, (struct sockaddr*)&address, sizeof(address));
        if (retVal == -1)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
                return false;
        }
    } while (retVal == -1);
    //    printHex("<-", buffer, len);
    return true;
}

int LinuxPlatform::readBytesMultiCast(uint8_t* buffer, uint16_t maxLen)
{
    uint32_t sin_len;
    struct sockaddr_in sin;

    sin_len = sizeof(sin);
    ssize_t len = recvfrom(_multicastSocketFd, buffer, maxLen, 0, (struct sockaddr*)&sin, &sin_len);
    //    if (len > 0)
    //        printHex("->", buffer, len);

    return len;
}

uint8_t* LinuxPlatform::getEepromBuffer(uint32_t size)
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

void LinuxPlatform::closeSpi()
{
    close(_spiFd);
    printf("SPI device closed.\r\n");
}

int LinuxPlatform::readWriteSpi(uint8_t* data, size_t len)
{
    uint16_t spiDelay = 0;
    uint32_t spiSpeed = 8000000; // 4 MHz SPI speed
    uint8_t spiBPW = 8;          // Bits per word

    struct spi_ioc_transfer spi;

    // Mentioned in spidev.h but not used in the original kernel documentation
    //	test program )-:

    memset(&spi, 0, sizeof(spi));

    spi.tx_buf = (uint64_t)data;
    spi.rx_buf = (uint64_t)data;
    spi.len = len;
    spi.delay_usecs = spiDelay;
    spi.speed_hz = spiSpeed;
    spi.bits_per_word = spiBPW;

    return ioctl(_spiFd, SPI_IOC_MESSAGE(1), &spi);
}

void LinuxPlatform::setupSpi()
{
    if ((_spiFd = open("/dev/spidev0.0", O_RDWR)) < 0)
    {
        printf("ERROR: SPI setup failed! Could not open SPI device!\r\n");
        return;
    }

    // Set SPI parameters.
    int mode = 0;        // Mode 0
    uint8_t spiBPW = 8;  // Bits per word
    int speed = 8000000; // 4 MHz SPI speed

    if (ioctl(_spiFd, SPI_IOC_WR_MODE, &mode) < 0)
    {
        printf("ERROR: SPI Mode Change failure: %s\n", strerror(errno));
        close(_spiFd);
        return;
    }

    if (ioctl(_spiFd, SPI_IOC_WR_BITS_PER_WORD, &spiBPW) < 0)
    {
        printf("ERROR: SPI BPW Change failure: %s\n", strerror(errno));
        close(_spiFd);
        return;
    }

    if (ioctl(_spiFd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0)
    {
        printf("ERROR: SPI Speed Change failure: %s\n", strerror(errno));
        close(_spiFd);
        return;
    }

    printf("SPI device setup ok.\r\n");
}

void LinuxPlatform::flashFilePath(const std::string path)
{
    _flashFilePath = path;
}

std::string LinuxPlatform::flashFilePath()
{
    return _flashFilePath;
}



size_t LinuxPlatform::readBytesUart(uint8_t *buffer, size_t length)
{
    return read(_uartFd, buffer, length);
}

int LinuxPlatform::readUart()
{
    uint8_t x ;

    if (read(_uartFd, &x, 1) != 1)
    {
        return -1;
    }

    return ((int)x) & 0xFF ;
}

size_t LinuxPlatform::writeUart(const uint8_t *buffer, size_t size)
{
    return write(_uartFd, buffer, size) ;
}

size_t LinuxPlatform::writeUart(const uint8_t data)
{
    return write(_uartFd, &data, 1) ;
}

int LinuxPlatform::uartAvailable()
{
    int result ;

     if (ioctl(_uartFd, FIONREAD, &result) == -1)
     {
        return -1;
     }

     return result ;
}

void LinuxPlatform::closeUart()
{
    if (_uartFd >= 0)
    {
        close(_uartFd);
    }
}

void LinuxPlatform::setupUart()
{
    /*
    * 19200,8E1, no handshake
    */
    struct termios options;    /* Schnittstellenoptionen */

    /* Port oeffnen - read/write, kein "controlling tty", Status von DCD ignorieren */
    _uartFd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
    if (_uartFd >= 0)
    {
        /* get the current options */
        fcntl(_uartFd, F_SETFL, 0);
        if (tcgetattr(_uartFd, &options) != 0)
        {
            close(_uartFd);
            _uartFd = -1;
            return;
        }
        memset(&options, 0, sizeof(options)); /* Structur loeschen, ggf. vorher sichern
                                             und bei Programmende wieder restaurieren */
        /* Baudrate setzen */
        cfsetispeed(&options, B19200);
        cfsetospeed(&options, B19200);

        /* setze Optionen */
        options.c_cflag |= PARENB;          /* Enable Paritybit */
        options.c_cflag &= ~PARODD;         /* Even parity */
        options.c_cflag &= ~CSTOPB;         /* 1 Stoppbit */
        options.c_cflag &= ~CSIZE;          /* 8 Datenbits */
        options.c_cflag |= CS8;

        /* 19200 bps, 8 Datenbits, CD-Signal ignorieren, Lesen erlauben */
        options.c_cflag |= (CLOCAL | CREAD);

        /* Kein Echo, keine Steuerzeichen, keine Interrupts */
        options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
        options.c_iflag = IGNPAR;           /* Parity-Fehler ignorieren */
        options.c_oflag &= ~OPOST;          /* setze "raw" Input */
        options.c_cc[VMIN]  = 0;            /* warten auf min. 0 Zeichen */
        options.c_cc[VTIME] = 10;           /* Timeout 1 Sekunde */
        tcflush(_uartFd,TCIOFLUSH);         /* Puffer leeren */

        if (tcsetattr(_uartFd, TCSAFLUSH, &options) != 0)
        {
            close(_uartFd);
            _uartFd = -1;
            return;
        }
    }
}

#ifndef KNX_NO_PRINT
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

void print(unsigned long long num)
{
    printUint64(num);
}

void print(unsigned long long num, int base)
{
    printUint64(num, base);
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

void println(unsigned long long num)
{
    printUint64(num);
    println("");
}

void println(unsigned long long num, int base)
{
    printUint64(num, base);
    println("");
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
#endif // KNX_NO_PRINT

void pinMode(uint32_t dwPin, uint32_t dwMode)
{
    gpio_export(dwPin);
    gpio_direction(dwPin, dwMode);
}

void digitalWrite(uint32_t dwPin, uint32_t dwVal)
{
    gpio_write(dwPin, dwVal);
}

uint32_t digitalRead(uint32_t dwPin)
{
    return gpio_read(dwPin);
}

typedef void (*voidFuncPtr)(void);
void attachInterrupt(uint32_t pin, voidFuncPtr callback, uint32_t mode)
{
}

void LinuxPlatform::cmdLineArgs(int argc, char** argv)
{
    if (_args)
        delete[] _args;

    _args = new char*[argc + 1];
    memcpy(_args, argv, argc * sizeof(char*));
    _args[argc] = 0;
}

/* Buffer size for string operations (e.g. snprintf())*/
#define MAX_STRBUF_SIZE 100
#define MAX_NUM_GPIO 64

static int gpioFds[MAX_NUM_GPIO] =
    {
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
};

/* Activate GPIO-Pin
 * Write GPIO pin number to /sys/class/gpio/export
 * Result: 0 = success, -1 = error
 */
int gpio_export(int pin)
{
    char buffer[MAX_STRBUF_SIZE]; /* Output Buffer       */
    ssize_t bytes;                /* Used Buffer length  */
    int fd;                       /* Filedescriptor      */
    int res;                      /* Result from write() */

    fprintf(stderr, "Export GPIO pin %d\n", pin);

    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0)
    {
        perror("Could not export GPIO pin(open)!\n");
        return (-1);
    }

    bytes = snprintf(buffer, MAX_STRBUF_SIZE, "%d", pin);
    res = write(fd, buffer, bytes);

    if (res < 0)
    {
        perror("Could not export GPIO pin(write)!\n");
        return (-1);
    }

    close(fd);
    delay(100);

    return (0);
}

/* Deactivate GPIO pin
 * Write GPIO pin number to /sys/class/gpio/unexport
 * Result: 0 = success, -1 = error
 */
int gpio_unexport(int pin)
{
    char buffer[MAX_STRBUF_SIZE]; /* Output Buffer       */
    ssize_t bytes;                /* Used Buffer length  */
    int fd;                       /* Filedescriptor      */
    int res;                      /* Result from write() */

    fprintf(stderr, "Unexport GPIO pin %d\n", pin);

    close(gpioFds[pin]);

    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd < 0)
    {
        perror("Could not unexport GPIO pin(open)!\n");
        return (-1);
    }

    bytes = snprintf(buffer, MAX_STRBUF_SIZE, "%d", pin);
    res = write(fd, buffer, bytes);

    if (res < 0)
    {
        perror("Could not unexport GPIO pin(write)!\n");
        return (-1);
    }

    close(fd);
    return (0);
}

/* Set GPIO pin mode (input/output)
 * Write GPIO pin number to /sys/class/gpioXX/direction
 * Direction: 0 = input, 1 = output
 * Result: 0 = success, -1 = error
 */
int gpio_direction(int pin, int dir)
{
    char path[MAX_STRBUF_SIZE]; /* Buffer for path     */
    int fd;                     /* Filedescriptor      */
    int res;                    /* Result from write() */

    fprintf(stderr, "Set GPIO direction for pin %d to %s\n", pin, (dir == INPUT) ? "INPUT" : "OUTPUT");

    snprintf(path, MAX_STRBUF_SIZE, "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(path, O_WRONLY);
    if (fd < 0)
    {
        perror("Could not set mode for GPIO pin(open)!\n");
        return (-1);
    }

    switch (dir)
    {
        case INPUT:
            res = write(fd, "in", 2);
            break;
        case OUTPUT:
            res = write(fd, "out", 3);
            break;
        default:
            res = -1;
            break;
    }

    if (res < 0)
    {
        perror("Could not set mode for GPIO pin(write)!\n");
        return (-1);
    }

    close(fd);
    return (0);
}

/* Read from GPIO pin
 * Result: -1 = error, 0/1 = GPIO pin state
 */
int gpio_read(int pin)
{
    char path[MAX_STRBUF_SIZE]; /* Buffer for path     */
    char c;

    snprintf(path, MAX_STRBUF_SIZE, "/sys/class/gpio/gpio%d/value", pin);
    if (gpioFds[pin] < 0)
        gpioFds[pin] = open(path, O_RDWR);
    if (gpioFds[pin] < 0)
    {
        perror("Could not read from GPIO(open)!\n");
        return (-1);
    }

    lseek(gpioFds[pin], 0L, SEEK_SET);
    if (read(gpioFds[pin], &c, 1) < 0)
    {
        perror("Could not read from GPIO(read)!\n");
        return (-1);
    }

    return (c == '0') ? LOW : HIGH;
}

/* Write to GPIO pin
 * Result: -1 = error, 0 = success
 */
int gpio_write(int pin, int value)
{
    char path[MAX_STRBUF_SIZE]; /* Buffer for path    */
    int res;                    /* Result from write()*/

    snprintf(path, MAX_STRBUF_SIZE, "/sys/class/gpio/gpio%d/value", pin);
    if (gpioFds[pin] < 0)
        gpioFds[pin] = open(path, O_RDWR);

    if (gpioFds[pin] < 0)
    {
        perror("Could not write to GPIO(open)!\n");
        return (-1);
    }

    switch (value)
    {
        case LOW:
            res = write(gpioFds[pin], "0\n", 2);
            break;
        case HIGH:
            res = write(gpioFds[pin], "1\n", 2);
            break;
        default:
            res = -1;
            break;
    }

    if (res < 0)
    {
        perror("Could not write to GPIO(write)!\n");
        return (-1);
    }

    return (0);
}

/* Set GPIO pin edge detection
 * 'r' (rising)
 * 'f' (falling)
 * 'b' (both)
 */
int gpio_edge(unsigned int pin, char edge)
{
    char path[MAX_STRBUF_SIZE]; /* Buffer for path    */
    int fd;                     /* Filedescriptor     */

    snprintf(path, MAX_STRBUF_SIZE, "/sys/class/gpio/gpio%d/edge", pin);

    fd = open(path, O_WRONLY | O_NONBLOCK);
    if (fd < 0)
    {
        perror("Could not set GPIO edge detection(open)!\n");
        return (-1);
    }

    switch (edge)
    {
        case 'r':
            strncpy(path, "rising", 8);
            break;
        case 'f':
            strncpy(path, "falling", 8);
            break;
        case 'b':
            strncpy(path, "both", 8);
            break;
        case 'n':
            strncpy(path, "none", 8);
            break;
        default:
            close(fd);
            return (-2);
    }

    write(fd, path, strlen(path) + 1);

    close(fd);
    return 0;
}

/* Wait for edge on GPIO pin
 * timeout in milliseconds
 * Result: <0: error, 0: poll() Timeout,
 * 1: edge detected, GPIO pin reads "0"
 * 2: edge detected, GPIO pin reads "1"
 */
int gpio_wait(unsigned int pin, int timeout)
{
    char path[MAX_STRBUF_SIZE]; /* Buffer for path     */
    int fd;                     /* Filedescriptor      */
    struct pollfd polldat[1];   /* Variable for poll() */
    char buf[MAX_STRBUF_SIZE];  /* Read buffer         */
    int rc;                     /* Result              */

    /* Open GPIO pin */
    snprintf(path, MAX_STRBUF_SIZE, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_RDONLY | O_NONBLOCK);
    if (fd < 0)
    {
        perror("Could not wait for GPIO edge(open)!\n");
        return (-1);
    }

    /* prepare poll() */
    memset((void*)buf, 0, sizeof(buf));
    memset((void*)polldat, 0, sizeof(polldat));
    polldat[0].fd = fd;
    polldat[0].events = POLLPRI;

    /* clear any existing detected edges before */
    lseek(fd, 0, SEEK_SET);
    rc = read(fd, buf, MAX_STRBUF_SIZE - 1);

    rc = poll(polldat, 1, timeout);
    if (rc < 0)
    { /* poll() failed! */
        perror("Could not wait for GPIO edge(poll)!\n");
        close(fd);
        return (-1);
    }

    if (rc == 0)
    { /* poll() timeout! */
        close(fd);
        return (0);
    }

    if (polldat[0].revents & POLLPRI)
    {
        if (rc < 0)
        { /* read() failed! */
            perror("Could not wait for GPIO edge(read)!\n");
            close(fd);
            return (-2);
        }
        /* printf("poll() GPIO %d interrupt occurred: %s\n", pin, buf); */
        close(fd);
        return (1 + atoi(buf));
    }

    close(fd);
    return (-1);
}

void delayMicrosecondsHard(unsigned int howLong)
{
    struct timeval tNow, tLong, tEnd;

    gettimeofday(&tNow, NULL);
    tLong.tv_sec = howLong / 1000000;
    tLong.tv_usec = howLong % 1000000;
    timeradd(&tNow, &tLong, &tEnd);

    while (timercmp(&tNow, &tEnd, <))
        gettimeofday(&tNow, NULL);
}

void delayMicroseconds(unsigned int howLong)
{
    struct timespec sleeper;
    unsigned int uSecs = howLong % 1000000;
    unsigned int wSecs = howLong / 1000000;

    /**/ if (howLong == 0)
        return;
    else if (howLong < 100)
        delayMicrosecondsHard(howLong);
    else
    {
        sleeper.tv_sec = wSecs;
        sleeper.tv_nsec = (long)(uSecs * 1000L);
        nanosleep(&sleeper, NULL);
    }
}

bool LinuxPlatform::sendBytesUniCast(uint32_t addr, uint16_t port, uint8_t* buffer, uint16_t len)
{
    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(addr);
    address.sin_port = htons(port);

    ssize_t retVal = 0;
    do
    {
        retVal = sendto(_multicastSocketFd, buffer, len, 0, (struct sockaddr*)&address, sizeof(address));
        if (retVal == -1)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
                return false;
        }
    } while (retVal == -1);
    //    printHex("<-", buffer, len);
    return true;
}

void LinuxPlatform::macAddress(uint8_t* mac_address)
{
    memcpy(mac_address, _macAddress, IFHWADDRLEN);
}

uint32_t LinuxPlatform::currentIpAddress()
{
    return _ipAddress;
}

uint32_t LinuxPlatform::currentSubnetMask()
{
    return _netmask;
}

uint32_t LinuxPlatform::currentDefaultGateway()
{
    return _defaultGateway;
}
#endif
