#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "rpiGPIO.h"
#include "rpiI2C.h"

volatile unsigned int *i2c_mem;
extern volatile unsigned int *gpio_mem;

/* 
* ===  FUNCTION  =============================================================
*         Name:  i2c_init(int div)
*  Description:  init I2C  with clock. clock = 150MHz/div
*                when div==0 div=32768
* ============================================================================
*/
int i2c_init (int div)
{
    i2c_mem  = mapRegAddr(BCM2709_PERI_BASE_DEFAULT + I2C1_BASE_OFFSET);
    if (gpio_mem == NULL)
        gpio_mem = mapRegAddr(BCM2709_PERI_BASE_DEFAULT + GPIO_BASE_OFFSET);

    configPinAlt(2, ALT0); /* configure GPIO2/GPIO3 to I2C1 */
    configPinAlt(3, ALT0);

    *(i2c_mem + I2C_CTL) = I2C_EN;

    *(i2c_mem + I2C_DIV) = div;
	return 0;
}

/* 
* ===  FUNCTION  =============================================================
*         Name:  i2c_write(int address, char word)
*  Description:  write 1 byte to I2C slave
* ============================================================================
*/
int i2c_write(int address, char word)
{
    *(i2c_mem + I2C_SLAVE) = address;
    *(i2c_mem + I2C_DLEN) = 1;
    *(i2c_mem + I2C_FIFO) = word;

    return 0;
}
/* 
* ===  FUNCTION  =============================================================
*         Name:  i2c_stop()
*  Description:  Puts all Peripheral registers in their original (reset state)
* ============================================================================
*/
int i2c_stop(void)
{
    /* put the PWM peripheral registers in their original state */
    *(i2c_mem + I2C_CTL) = 0;

    /* reset GPIO2 and GPIO3 to GPIO INPUT */
	configPinAlt(2, INPUT);
	configPinAlt(3, INPUT);

    if(munmap((void*)gpio_mem, BLOCK_SIZE) < 0){
        perror("munmap (gpio)");
        exit(1);
    }
    if(munmap((void*)i2c_mem, BLOCK_SIZE) < 0){
        perror("munmap (i2c)");
        exit(1);
    }
	return 0;
}
