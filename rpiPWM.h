#ifndef RPIPWM1_H
#define PRIPWM1_H

/* Two PWM modes */
#define PWMMODE     0
#define MSMODE      1

/* error code */

#define ERRFREQ     1
#define ERRCOUNT    2
#define ERRDUTY     3
#define ERRMODE     4

/* hardware related constants */
#define BCM2709_PERI_BASE_DEFAULT   0x3f000000
#define GPIO_BASE_OFFSET    0x200000
#define PWM_BASE_OFFSET     0x20c000
#define CLK_BASE_OFFSET     0x101000

#define BLOCK_SIZE   4096

/* PWM registers */
#define PWM_CTL     0
#    define   PWM_ENABLE 0
#    define   MODESHIFT  1
#    define   MSSHIFT    7
#    define   POLARITY   4

#define PWM_RNG1    4
#define PWM_DAT1    5
#define PWM_RNG2    8
#define PWM_DAT2    9

#define PWMCLK_CNTL 40
#define PWMCLK_DIV  41

int setFrequency(int pin, float frequency);

int setDutyCycle(int pin, float dutycycle);

int setMode(int pin, int mode);
int setPolarity(int pin, int pol);
int pwm_enable(int pin, _Bool enable);

void pwm_init(void);
void pwm_stop(void);
#endif
