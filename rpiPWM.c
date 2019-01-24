#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "rpiPWM.h"

static volatile unsigned int *clk_mem, *pwm_mem, *gpio_mem;

/* 
* ===  FUNCTION  =============================================================
*         Name:  pwm_enable(int pin, bool enable)
*  Description:  enable or disable PWM pin
* ============================================================================
*/
int pwm_enable(int pin, _Bool enable)
{
    int reg = 0;
    int shift = 0;

    shift = PWM_ENABLE + pin*8;
    reg = *(pwm_mem + PWM_CTL);
    reg &= ~(1 << shift);
    reg |= (enable << shift);

    *(pwm_mem + PWM_CTL) = reg;

    return 0;
}

/* 
* ===  FUNCTION  =============================================================
*         Name:  setMode(int pin, int mode)
*  Description:  set pin to MSMODE(normal) or PWMMODE(distributed)
* ============================================================================
*/
int setMode(int pin, int mode)
{
    int reg = 0;
    int shift = 0;

    if((mode != PWMMODE) && (mode != MSMODE)) {
        return ERRMODE;
    } else if(pin > 1) {
        return ERRMODE;
    } else {
        shift = 7 + pin*8;
        reg = *(pwm_mem + PWM_CTL);
        reg &= ~(1 << shift);
        reg |= (mode << shift);

        *(pwm_mem + PWM_CTL) = reg;
        return 0;
    }
}

/* 
* ===  FUNCTION  =============================================================
*         Name:  setPolarity(int pin, int pol)
*  Description:  set pin to specified polarity (0, normal, 1, inverted)
* ============================================================================
*/
int setPolarity(int pin, int pol)
{
    int reg = 0;
    int shift = 0;

    shift = POLARITY + pin*8;
    reg = *(pwm_mem + PWM_CTL);
    reg &= ~(1 << shift);
    reg |= (pol << shift);

    *(pwm_mem + PWM_CTL) = reg;
    return 0;
}

/* 
* ===  FUNCTION  =============================================================
*         Name:  *mapRegAddr
*  Description:  map 4KB of physical memory into the calling process
*
* ============================================================================
*/
volatile unsigned int *mapRegAddr(unsigned int baseAddr)
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
        PROT_READ|PROT_WRITE|PROT_EXEC,
        MAP_SHARED|MAP_LOCKED,
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
    return (volatile unsigned int *)regAddrMap;
}

/* 
* ===  FUNCTION  =============================================================
*         Name:  configPWMPin(int gpio)
*  Description:  set GPIO pin to PWM mode
* ============================================================================
*/
#define FSEL_OFFSET  0
#define ALT0         4    /* PWM for pin 12/13 */
#define ALT5         2    /* PWM for pin 18/19 */
int configPWMPin(int gpio)
{
    unsigned int offset = FSEL_OFFSET + (gpio/10);
    int shift = (gpio%10)*3;
    int reg;

    reg = *(gpio_mem + offset);
    reg &= ~(7 << shift);
    if (gpio == 12 || gpio == 13) {
        reg |= (ALT0 << shift);
    }
    else if (gpio == 18 || gpio == 19) {
        reg |= (ALT5 << shift);
    }

    *(gpio_mem+offset) = reg;
    return 0;
}

/* 
* ===  FUNCTION  =============================================================
*         Name:  setClock(_Bool init)
*  Description:  sett PWM clock. when init, set also frequency and dutycycle
* ============================================================================
*/
#define BCM_PASSWD  0x5A000000
#define CLK_OSC     0     
#define CLK_ENABLE  4
#define CLK_KILL    5
#define CLK_BUSY    7
int setClock(_Bool init)
{
    int divisor = 75;
	int counts = 0;

    /* stop clock and waiting for busy flag doesn't work,
     * so kill clock  first. '5A' is CLK password */
    *(clk_mem + PWMCLK_CNTL) = BCM_PASSWD | (1 << CLK_KILL);
    usleep(10);  

    // wait until busy flag is set 
    while ((*(clk_mem + PWMCLK_CNTL)) & (1<<CLK_BUSY)) { }   

    /* set divisor */
    *(clk_mem + PWMCLK_DIV) = BCM_PASSWD | (divisor << 12);

    /* source=osc and enable clock */
    *(clk_mem + PWMCLK_CNTL) = BCM_PASSWD | (1<<CLK_ENABLE) | (1<<CLK_OSC);

    if(init) {
        *(pwm_mem + PWM_CTL) = 0;
        usleep(10); /* prevent from PWM module crashes */

        counts = 19200000.0/75.0/1000.0; /* default set to 1000Hz */
        *(pwm_mem + PWM_RNG1) = counts;
        *(pwm_mem + PWM_RNG2) = counts;
        usleep(10);

        *(pwm_mem + PWM_DAT1) = counts / 2; /* default duty cycle to 50% */
        *(pwm_mem + PWM_DAT2) = counts / 2;
        usleep(10); /* prevent from PWM module crashes */
    }

    return 0;
}

/* 
* ===  FUNCTION  =============================================================
*         Name:  setFrequency(int pin, float frequency)
*  Description:  set PWM pin frequency between 0.1Hz and 19.2MHz
* ============================================================================
*/
int setFrequency(int pin, float frequency)
{
    int counts, bits;

    /* make sure the frequency is valid */
    if (frequency < 0 || frequency > 19200000.0f)
        return ERRFREQ;

    setClock(0);
    if (pin == 0) {
        bits = *(pwm_mem + PWM_DAT1);
        counts = (int)(frequency / (1.0*bits));
        *(pwm_mem + PWM_RNG1) = counts;
    } else if (pin == 2) {
        bits = *(pwm_mem + PWM_DAT2);
        counts = (int)(frequency / (1.0*bits));
        *(pwm_mem + PWM_RNG2) = counts;
    } else {
        return ERRFREQ;
    }

    usleep(10);
    return 0;
}

/* 
* ===  FUNCTION  =============================================================
*         Name:  setDutyCycle(int pin, float dutycycle)
*  Description:  set duty cycle from 0% to 100%
* ============================================================================
*/
int setDutyCycle(int pin, float dutycycle)
{
    int counts, bits;

    if((dutycycle < 0 || dutycycle > 100.0))
        return ERRDUTY;

    setClock(0);

    if (pin == 0) {
        counts = *(pwm_mem + PWM_RNG1);
        bits = dutycycle/100.0 * counts;
        *(pwm_mem + PWM_DAT1) = bits;
    } else if (pin == 1) {
        counts = *(pwm_mem + PWM_RNG2);
        bits = dutycycle/100.0 * counts;
        *(pwm_mem + PWM_DAT2) = bits;
    } else {
        return ERRDUTY;
    }

    usleep(10);
    return 0;
}

/* 
* ===  FUNCTION  =============================================================
*         Name:  pwm_stop()
*  Description:  Puts all Peripheral registers in their original (reset state)
* ============================================================================
*/
void pwm_stop()
{
    /* put the PWM peripheral registers in their original state */
    *(pwm_mem + PWM_CTL) = 0;
    *(pwm_mem + PWM_RNG1) = 0x20;
    *(pwm_mem + PWM_DAT1) = 0;
    *(pwm_mem + PWM_RNG2) = 0x20;
    *(pwm_mem + PWM_DAT2) = 0;

    if(munmap((void*)pwm_mem, BLOCK_SIZE) < 0){
        perror("munmap (pwm)");
        exit(1);
    }
    /* put the PWM Clock in their original state */
    *(clk_mem + PWMCLK_CNTL) = BCM_PASSWD | (1 << CLK_KILL);
    usleep(10);

    while((*(clk_mem + PWMCLK_CNTL)) & (1<<CLK_BUSY)) { }

    *(clk_mem + PWMCLK_DIV) = BCM_PASSWD;  /* reset divisor */
    usleep(10);

    /* source=osc and enable clock */
    *(clk_mem + PWMCLK_CNTL) = BCM_PASSWD | (1<<CLK_ENABLE) | (1<<CLK_OSC);

    if(munmap((void*)clk_mem, BLOCK_SIZE) < 0){
        perror("munmap (clk)");
        exit(1);
    }

    /* reset GPIO18 and GPIO19 to GPIO INPUT */
    *(gpio_mem+FSEL_OFFSET) &= ~(7 << 24);
    *(gpio_mem+FSEL_OFFSET) &= ~(7 << 27);

    if(munmap((void*)gpio_mem, BLOCK_SIZE) < 0){
        perror("munmap (gpio)");
        exit(1);
    }
}

/* 
* ===  FUNCTION  =============================================================
*         Name:  pwm_init()
*  Description:  map registers, set frequency to 1kHz, dutycycle to 50%
*                without output signal
* ============================================================================
*/
void pwm_init()
{
    pwm_mem  = mapRegAddr(BCM2709_PERI_BASE_DEFAULT + PWM_BASE_OFFSET);
    clk_mem  = mapRegAddr(BCM2709_PERI_BASE_DEFAULT + CLK_BASE_OFFSET);
    gpio_mem = mapRegAddr(BCM2709_PERI_BASE_DEFAULT + GPIO_BASE_OFFSET);

    configPWMPin(18); //configure GPIO18 to ALT5 (PWM output)
    configPWMPin(19); //configure GPIO19 to ALT5 (PWM output)

    setClock(1);
    setClock(1);
    setMode(0, MSMODE);
    setMode(1, MSMODE);
}
