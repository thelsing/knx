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

#include <sys/ioctl.h>            // Needed for SPI port
#include <linux/spi/spidev.h>     // Needed for SPI port
#include <poll.h>                 // Needed for GPIO edge detection
#include <sys/time.h>             // Needed for delayMicroseconds()

#include "knx/device_object.h"
#include "knx/address_table_object.h"
#include "knx/association_table_object.h"
#include "knx/group_object_table_object.h"
#include "knx/application_program_object.h"
#include "knx/ip_parameter_object.h"
#include "knx/bits.h"

#define MAX_MEM 4096

LinuxPlatform::LinuxPlatform()
{
    Platform::_memoryReference = (uint8_t*)malloc(MAX_MEM);
    _currentMaxMem = Platform::_memoryReference;
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

void LinuxPlatform::closeSpi()
{
    close(_spiFd);
    printf ("SPI device closed.\r\n");
}

int LinuxPlatform::readWriteSpi (uint8_t *data, size_t len)
{
    uint16_t spiDelay = 0 ;
    uint32_t spiSpeed = 8000000; // 4 MHz SPI speed
    uint8_t spiBPW = 8; // Bits per word

    struct spi_ioc_transfer spi ;

    // Mentioned in spidev.h but not used in the original kernel documentation
    //	test program )-:

    memset (&spi, 0, sizeof (spi)) ;

    spi.tx_buf        = (uint64_t)data;
    spi.rx_buf        = (uint64_t)data;
    spi.len           = len;
    spi.delay_usecs   = spiDelay;
    spi.speed_hz      = spiSpeed;
    spi.bits_per_word = spiBPW;

    return ioctl (_spiFd, SPI_IOC_MESSAGE(1), &spi) ;
}

void LinuxPlatform::setupSpi()
{
    if ((_spiFd = open ("/dev/spidev0.0", O_RDWR)) < 0)
    {
        printf ("ERROR: SPI setup failed! Could not open SPI device!\r\n");
        return;
    }

    // Set SPI parameters.
    int mode = 0; // Mode 0
    uint8_t spiBPW = 8; // Bits per word
    int speed = 8000000; // 4 MHz SPI speed

    if (ioctl (_spiFd, SPI_IOC_WR_MODE, &mode) < 0)
    {
        printf ("ERROR: SPI Mode Change failure: %s\n", strerror (errno)) ;
        close(_spiFd);
        return;
    }

    if (ioctl (_spiFd, SPI_IOC_WR_BITS_PER_WORD, &spiBPW) < 0)
    {
        printf ("ERROR: SPI BPW Change failure: %s\n", strerror (errno)) ;
        close(_spiFd);
        return;
    }

    if (ioctl (_spiFd, SPI_IOC_WR_MAX_SPEED_HZ, &speed)   < 0)
    {
        printf ("ERROR: SPI Speed Change failure: %s\n", strerror (errno)) ;
        close(_spiFd);
        return;
    }

    printf ("SPI device setup ok.\r\n");


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

void pinMode(uint32_t dwPin, uint32_t dwMode)
{
}

void digitalWrite(uint32_t dwPin, uint32_t dwVal)
{
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

void LinuxPlatform::setupGpio(uint32_t dwPin, uint32_t dwMode)
{
    gpio_export(dwPin);
    gpio_direction(dwPin, dwMode);
}

void LinuxPlatform::closeGpio(uint32_t dwPin)
{
    gpio_unexport(dwPin);
    // Set direction to input always if we do not need the GPIO anymore? Unsure...
    //gpio_direction(dwPin, INPUT);
}

void LinuxPlatform::writeGpio(uint32_t dwPin, uint32_t dwVal)
{
    gpio_write(dwPin, dwVal);
}

uint32_t LinuxPlatform::readGpio(uint32_t dwPin)
{
    return gpio_read(dwPin);
}

/* Datenpuffer fuer die GPIO-Funktionen */
#define MAXBUFFER 100

/* GPIO-Pin aktivieren
 * Schreiben der Pinnummer nach /sys/class/gpio/export
 * Ergebnis: 0 = O.K., -1 = Fehler
 */
int gpio_export(int pin)
{
    char buffer[MAXBUFFER];    /* Output Buffer   */
    ssize_t bytes;             /* Datensatzlaenge */
    int fd;                    /* Filedescriptor  */
    int res;                   /* Ergebnis von write */

    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0)
    {
        perror("Kann nicht auf export schreiben!\n");
        return(-1);
    }

    bytes = snprintf(buffer, MAXBUFFER, "%d", pin);
    res = write(fd, buffer, bytes);

    if (res < 0)
    {
        perror("Kann Pin nicht aktivieren (write)!\n");
        return(-1);
    }

    close(fd);
    delay(100);

    return(0);
}

/* GPIO-Pin deaktivieren
 * Schreiben der Pinnummer nach /sys/class/gpio/unexport
 * Ergebnis: 0 = O.K., -1 = Fehler
 */
int gpio_unexport(int pin)
{
    char buffer[MAXBUFFER];    /* Output Buffer   */
    ssize_t bytes;             /* Datensatzlaenge */
    int fd;                    /* Filedescriptor  */
    int res;                   /* Ergebnis von write */

    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd < 0)
    {
        perror("Kann nicht auf unexport schreiben!\n");
        return(-1);
    }

    bytes = snprintf(buffer, MAXBUFFER, "%d", pin);
    res = write(fd, buffer, bytes);

    if (res < 0)
    {
        perror("Kann Pin nicht deaktivieren (write)!\n");
        return(-1);
    }

    close(fd);
    return(0);
}

/* Datenrichtung GPIO-Pin festlegen
 * Schreiben Pinnummer nach /sys/class/gpioXX/direction
 * Richtung dir: 0 = Lesen, 1 = Schreiben
 * Ergebnis: 0 = O.K., -1 = Fehler
 */
int gpio_direction(int pin, int dir)
{
    char path[MAXBUFFER];      /* Buffer fuer Pfad   */
    int fd;                    /* Filedescriptor     */
    int res;                   /* Ergebnis von write */

    snprintf(path, MAXBUFFER, "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(path, O_WRONLY);
    if (fd < 0)
    {
        perror("Kann Datenrichtung nicht setzen (open)!\n");
        return(-1);
    }

    switch (dir)
    {
    case INPUT : res = write(fd,"in",2); break;
    case OUTPUT: res = write(fd,"out",3); break;
    default: res = -1; break;
    }

    if (res < 0)
    {
        perror("Kann Datenrichtung nicht setzen (write)!\n");
        return(-1);
    }

    close(fd);
    return(0);
}

/* vom GPIO-Pin lesen
 * Ergebnis: -1 = Fehler, 0/1 = Portstatus
 */
int gpio_read(int pin)
{
    char path[MAXBUFFER];         /* Buffer fuer Pfad     */
    int fd;                       /* Filedescriptor       */
    char result[MAXBUFFER] = {0}; /* Buffer fuer Ergebnis */

    snprintf(path, MAXBUFFER, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        perror("Kann vom GPIO nicht lesen (open)!\n");
        return(-1);
    }

    if (read(fd, result, 3) < 0)
    {
        perror("Kann vom GPIO nicht lesen (read)!\n");
        return(-1);
    }

    close(fd);
    return(atoi(result));
}

/* auf GPIO schreiben
 * Ergebnis: -1 = Fehler, 0 = O.K.
 */
int gpio_write(int pin, int value)
{
    char path[MAXBUFFER];      /* Buffer fuer Pfad   */
    int fd;                    /* Filedescriptor     */
    int res;                   /* Ergebnis von write */

    snprintf(path, MAXBUFFER, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_WRONLY);

    if (fd < 0)
    {
        perror("Kann auf GPIO nicht schreiben (open)!\n");
        return(-1);
    }

    switch (value)
    {
        case LOW : res = write(fd,"0",1); break;
        case HIGH: res = write(fd,"1",1); break;
        default: res = -1; break;
    }

    if (res < 0)
    {
        perror("Kann auf GPIO nicht schreiben (write)!\n");
        return(-1);
    }

    close(fd);
    return(0);
}

/* GPIO-Pin auf Detektion einer Flanke setzen.
 * Fuer die Flanke (edge) koennen folgende Parameter gesetzt werden:
 * 'r' (rising) - steigende Flanke,
 * 'f' (falling) - fallende Flanke,
 * 'b' (both) - beide Flanken.
 */
int gpio_edge(unsigned int pin, char edge)
{
    char path[MAXBUFFER];    /* Buffer fuer Pfad   */
    int fd;                    /* Filedescriptor     */

    snprintf(path, MAXBUFFER, "/sys/class/gpio/gpio%d/edge", pin);

    fd = open(path, O_WRONLY | O_NONBLOCK );
    if (fd < 0)
    {
        perror("gpio_edge: Kann auf GPIO nicht schreiben (open)!\n");
        return(-1);
    }

    switch (edge)
    {
    case 'r': strncpy(path,"rising",8); break;
    case 'f': strncpy(path,"falling",8); break;
    case 'b': strncpy(path,"both",8); break;
    case 'n': strncpy(path,"none",8); break;
    default: close(fd);return(-2);
    }

    write(fd, path, strlen(path) + 1);

    close(fd);
    return 0;
}

/* Warten auf Flanke am GPIO-Pin.
 * Eingabewerte: pin: GPIO-Pin
 *               timeout: Wartezeit in Millisekunden
 * Der Pin muss voher eingerichtet werden (export,
 * direction, edge)
 * Rueckgabewerte: <0: Fehler, 0: poll() Timeout,
 * 1: Flanke erkannt, Pin lieferte "0"
 * 2: Flanke erkannt, Pin lieferte "1"
 */
int gpio_wait(unsigned int pin, int timeout)
{
    char path[MAXBUFFER];     /* Buffer fuer Pfad   */
    int fd;                   /* Filedescriptor     */
    struct pollfd polldat[1]; /* Variable fuer poll() */
    char buf[MAXBUFFER];      /* Lesepuffer */
    int rc;                   /* Hilfsvariablen */

    /* GPIO-Pin dauerhaft oeffnen */
    snprintf(path, MAXBUFFER, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_RDONLY | O_NONBLOCK );
    if (fd < 0)
    {
        perror("gpio_wait: Kann von GPIO nicht lesen (open)!\n");
        return(-1);
    }

    /* poll() vorbereiten */
    memset((void*)buf, 0, sizeof(buf));
    memset((void*)polldat, 0, sizeof(polldat));
    polldat[0].fd = fd;
    polldat[0].events = POLLPRI;

    /* eventuell anstehende Interrupts loeschen */
    lseek(fd, 0, SEEK_SET);
    rc = read(fd, buf, MAXBUFFER - 1);

    rc = poll(polldat, 1, timeout);
    if (rc < 0)
    { /* poll() failed! */
        perror("gpio_wait: Poll-Aufruf ging schief!\n");
        close(fd);
        return(-1);
    }

    if (rc == 0)
    { /* poll() timeout! */
        close(fd);
        return(0);
    }

    if (polldat[0].revents & POLLPRI)
    {
        if (rc < 0)
        { /* read() failed! */
            perror("gpio_wait: Kann von GPIO nicht lesen (read)!\n");
            close(fd);
            return(-2);
        }
        /* printf("poll() GPIO %d interrupt occurred: %s\n", pin, buf); */
        close(fd);
        return(1 + atoi(buf));
    }

    close(fd);
    return(-1);
}

void delayMicrosecondsHard (unsigned int howLong)
{
  struct timeval tNow, tLong, tEnd ;

  gettimeofday (&tNow, NULL) ;
  tLong.tv_sec  = howLong / 1000000 ;
  tLong.tv_usec = howLong % 1000000 ;
  timeradd (&tNow, &tLong, &tEnd) ;

  while (timercmp (&tNow, &tEnd, <))
    gettimeofday (&tNow, NULL) ;
}

void delayMicroseconds (unsigned int howLong)
{
  struct timespec sleeper ;
  unsigned int uSecs = howLong % 1000000 ;
  unsigned int wSecs = howLong / 1000000 ;

  /**/ if (howLong ==   0)
    return ;
  else if (howLong  < 100)
    delayMicrosecondsHard (howLong) ;
  else
  {
    sleeper.tv_sec  = wSecs ;
    sleeper.tv_nsec = (long)(uSecs * 1000L) ;
    nanosleep (&sleeper, NULL) ;
  }
}

#endif
