#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "rpiGPIO.h"

volatile unsigned int *gpio_mem;
/* 
* ===  FUNCTION  =============================================================
*         Name:  *mapRegAddr
*  Description:  map 4KB of physical memory into the calling process
*
* ============================================================================
*/
unsigned int *mapRegAddr(unsigned int baseAddr)
{
    int mem_fd = 0;
    void *regAddrMap = MAP_FAILED;

    /* open /dev/mem.....need to run program as root i.e. use sudo or su */
    if (!mem_fd) {
        if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
            perror("can't open /dev/mem");
            exit (1);
        }
    }

    /* mmap IO */
    regAddrMap = mmap(
        NULL,
        BLOCK_SIZE,
        PROT_READ|PROT_WRITE,
        MAP_SHARED,
        mem_fd,
        baseAddr
        );

    if(close(mem_fd) < 0) {
        perror("close /dev/mem");
        exit(1);
    }

    if (regAddrMap == MAP_FAILED) {
        perror("mmap error");
        exit (1);
    }
    return (unsigned int *)regAddrMap;
}

/* 
* ===  FUNCTION  =============================================================
*         Name:  configPinAlt(int gpio, int mode)
*  Description:  set GPIO pin to an alternative mode
* ============================================================================
*/
int configPinAlt(int gpio, int mode)
{
    unsigned int offset = FSEL_OFFSET + (gpio/10);
    int shift = (gpio%10)*3;
    int reg;

    reg = *(gpio_mem + offset);
    reg &= ~(7 << shift);
    reg |= (mode << shift);

    *(gpio_mem+offset) = reg;
    return 0;
}

void init_gpio()
{
    gpio_mem = mapRegAddr(BCM2709_PERI_BASE_DEFAULT + GPIO_BASE_OFFSET);
}
