#ifndef _RPIPWM1_H
#define _PRIPWM1_H

/* Two PWM modes */
#define PWMMODE     0
#define MSMODE      1

/* error code */

#define ERRFREQ     1
#define ERRCOUNT    2
#define ERRDUTY     3
#define ERRMODE     4
#define ERRPIN		5

/* hardware related constants */
#define PWM_BASE_OFFSET     0x20c000
#define CLK_BASE_OFFSET     0x101000

/* PWM registers */
#define PWM_CTL     0
#    define   PWM_ENABLE 0
#    define   MODESHIFT  1
#    define   MSSHIFT    7
#    define   POLARITY   4

#define PWM_RNG(x)		(4 + 4*(x))
#define PWM_DAT(x)		(5 + 4*(x))

#define PWMCLK_CNTL 40
#define PWMCLK_DIV  41

int setFrequency(int pwm, float frequency);

int setDutyCycle(int pwm, float dutycycle);

int setMode(int pwm, int mode);
int setPolarity(int pwm, int pol);
int pwm_enable(int pwm, _Bool enable);

void pwm_init(int div);
void pwm_stop(void);
#endif      /* _RPIPWM1_H */
