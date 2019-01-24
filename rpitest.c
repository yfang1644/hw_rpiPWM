#include <stdio.h>
#include <unistd.h>

#include "rpiPWM.h"

int main (int argc, char *argv[])
{
    pwm_init();

    setFrequency(0, 100);
    setDutyCycle(0, 10);
    pwm_enable(0, 1);

    usleep(2000000);
    pwm_enable(0, 0);
    usleep(2000000);
    pwm_stop();
    return 0;
}
