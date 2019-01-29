#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "rpiGPIO.h"
#include "rpiPWM.h"

volatile unsigned int *clk_mem, *pwm_mem;
extern volatile unsigned int *gpio_mem;

static int divisor = 8;
/* 
* ===  FUNCTION  =============================================================
*         Name:  pwm_enable(int pwm, bool enable)
*  Description:  enable or disable PWM pin
* ============================================================================
*/
int pwm_enable(int pwm, _Bool enable)
{
    int reg = 0;
    int shift = 0;

    shift = PWM_ENABLE + pwm*8;
    reg = *(pwm_mem + PWM_CTL);
    reg &= ~(1 << shift);
    reg |= (enable << shift);

    *(pwm_mem + PWM_CTL) = reg;

    return 0;
}

/* 
* ===  FUNCTION  =============================================================
*         Name:  setMode(int pwm, int mode)
*  Description:  set pwm to MSMODE(normal) or PWMMODE(distributed)
* ============================================================================
*/
int setMode(int pwm, int mode)
{
    int reg = 0;
    int shift = 0;

    if((mode != PWMMODE) && (mode != MSMODE)) {
        return ERRMODE;
    } else if(pwm > 1) {
        return ERRMODE;
    } else {
        shift = MSSHIFT + pwm*8;
        reg = *(pwm_mem + PWM_CTL);
        reg &= ~(1 << shift);
        reg |= (mode << shift);

        *(pwm_mem + PWM_CTL) = reg;
        return 0;
    }
}

/* 
* ===  FUNCTION  =============================================================
*         Name:  setPolarity(int pwm, int pol)
*  Description:  set pin to specified polarity (0, normal, 1, inverted)
* ============================================================================
*/
int setPolarity(int pwm, int pol)
{
    int reg = 0;
    int shift = 0;

    shift = POLARITY + pwm*8;
    reg = *(pwm_mem + PWM_CTL);
    reg &= ~(1 << shift);
    reg |= (pol << shift);

    *(pwm_mem + PWM_CTL) = reg;
    return 0;
}

/* 
* ===  FUNCTION  =============================================================
*         Name:  setClock(int div)
*  Description:  sett PWM clock.
* ============================================================================
*/
#define BCM_PASSWD  0x5A000000
#define CLK_OSC     0     
#define CLK_ENABLE  4
#define CLK_KILL    5
#define CLK_BUSY    7
int setClock(int div)
{
    int counts = 0;

    /* stop clock and waiting for busy flag doesn't work,
     * so kill clock  first. '5A' is CLK password */
    *(clk_mem + PWMCLK_CNTL) = BCM_PASSWD | (1 << CLK_KILL);
    usleep(2);  

    // wait until busy flag is set 
    while ((*(clk_mem + PWMCLK_CNTL)) & (1<<CLK_BUSY)) { }   

    /* set divisor */
    *(clk_mem + PWMCLK_DIV) = BCM_PASSWD | (divisor << 12);

    /* source=osc and enable clock */
    *(clk_mem + PWMCLK_CNTL) = BCM_PASSWD | (1<<CLK_ENABLE) | (1<<CLK_OSC);

    return 0;
}

/* 
* ===  FUNCTION  =============================================================
*         Name:  setFrequency(int pin, float frequency)
*  Description:  set PWM pin frequency between 0.1Hz and 19.2MHz
* ============================================================================
*/
int setFrequency(int pwm, float freq)
{
    int counts, bits;
	float f;

    /* make sure the frequency is valid */
    if (freq < 0 || freq > 19200000.0f)
        return ERRFREQ;

    if (pwm < 0 || pwm > 1)
        return ERRPIN;

    counts = *(pwm_mem + PWM_RNG(pwm));
    bits = *(pwm_mem + PWM_DAT(pwm));

	if (counts)
		f = (float)bits/counts;
	else
		f = 0;

    counts = 19200000.0/divisor/freq;
    bits = counts * f;
    *(pwm_mem + PWM_RNG(pwm)) = counts;
    *(pwm_mem + PWM_DAT(pwm)) = bits;

    return 0;
}

/* 
* ===  FUNCTION  =============================================================
*         Name:  setDutyCycle(int pwm, float dutycycle)
*  Description:  set duty cycle from 0% to 100%
* ============================================================================
*/
int setDutyCycle(int pwm, float duty)
{
    int counts, bits;

    if((duty< 0 || duty> 100.0))
        return ERRDUTY;

    if (pwm < 0 || pwm > 1)
        return ERRPIN;

    counts = *(pwm_mem + PWM_RNG(pwm));

    bits = counts * duty/100.0;
    *(pwm_mem + PWM_DAT(pwm)) = bits;

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
    *(pwm_mem + PWM_RNG(0)) = 0x20;
    *(pwm_mem + PWM_DAT(0)) = 0;
    *(pwm_mem + PWM_RNG(1)) = 0x20;
    *(pwm_mem + PWM_DAT(1)) = 0;

    if(munmap((void*)pwm_mem, BLOCK_SIZE) < 0){
        perror("munmap (pwm)");
        exit(1);
    }
    setClock(0);

    if(munmap((void*)clk_mem, BLOCK_SIZE) < 0){
        perror("munmap (clk)");
        exit(1);
    }

    /* reset GPIO18 and GPIO19 to GPIO INPUT */
    *(gpio_mem+FSEL_OFFSET+1) &= ~(7 << 24);
    *(gpio_mem+FSEL_OFFSET+1) &= ~(7 << 27);

    if(munmap((void*)gpio_mem, BLOCK_SIZE) < 0){
        perror("munmap (gpio)");
        exit(1);
    }
    gpio_mem = NULL;
}

/* 
* ===  FUNCTION  =============================================================
*         Name:  pwm_init()
*  Description:  map registers, set frequency to 1kHz, dutycycle to 50%
*                without output signal
* ============================================================================
*/
void pwm_init(int div)
{
    pwm_mem  = mapRegAddr(BCM2709_PERI_BASE_DEFAULT + PWM_BASE_OFFSET);
    clk_mem  = mapRegAddr(BCM2709_PERI_BASE_DEFAULT + CLK_BASE_OFFSET);
    if (gpio_mem == NULL)
        gpio_mem = mapRegAddr(BCM2709_PERI_BASE_DEFAULT + GPIO_BASE_OFFSET);

    configPinAlt(18, ALT5); //configure GPIO18 to ALT5 (PWM output)
    configPinAlt(19, ALT5); //configure GPIO19 to ALT5 (PWM output)

    divisor = div;
    setClock(div);
    setMode(0, MSMODE);
    setMode(1, MSMODE);
}
