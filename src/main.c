#include <ch.h>
#include <hal.h>

#define GPIOD_LED_ORANGE GPIOD_LED3 
#define GPIOD_LED_RED    GPIOD_LED5
#define GPIOD_LED_BLUE   GPIOD_LED6
#define GPIOD_LED_GREEN  GPIOD_LED4

#define GPIOD_RPM_OUT GPIOD_LED_ORANGE

#define TEETH 60
#define MISSING 2

static WORKING_AREA(wheelStack, 32);
Thread *wheelTP;

volatile int edgeTicks = 200;

msg_t wheelThd(void *arg)
{
    (void) arg;

    systime_t lastEdgeTime;
    int tooth = 0;
    int direction = 0;
    while (1) {
        lastEdgeTime = chTimeNow();
        if (tooth < (TEETH - MISSING)) {
            palTogglePad(GPIOD, GPIOD_RPM_OUT);
        }

        if (direction) {
            ++tooth;
            if (tooth >= TEETH) {
                tooth = 0;
            }
        }

        direction = !direction;
        chThdSleepUntil(lastEdgeTime + edgeTicks);
    }
}

void updateEdgeTicks(int rpm)
{
    if (rpm == 0) {
        rpm = 1;
    }
    edgeTicks = (30 * CH_FREQUENCY) / (TEETH * rpm);
}

int main(void)
{
    halInit();
    chSysInit();

    palSetGroupMode(GPIOD, GPIOD_LED_BLUE | GPIOD_LED_GREEN | GPIOD_LED_ORANGE | GPIOD_LED_RED, 0, PAL_MODE_OUTPUT_PUSHPULL);

    chThdCreateStatic(wheelStack, sizeof(wheelStack), LOWPRIO, wheelThd, NULL);

#define RPM_RAMP_PER_MIN 20000
#define MAX_RPM  20000
    int usPerRpmInc = 60000000/RPM_RAMP_PER_MIN;

    int rpm = 100;
    int rampUp = 1;
    while (1) {
        palTogglePad(GPIOD, GPIOD_LED_BLUE);
        systime_t lastUpdateTime = chTimeNow();
        updateEdgeTicks(rpm);
        rpm = rampUp ? rpm + 1 : rpm - 1;
        if (rpm == MAX_RPM) {
            rampUp = FALSE;
        } else if (rpm == 100) {
            rampUp = TRUE;
        }
        palTogglePad(GPIOD, GPIOD_LED_BLUE);
        chThdSleepUntil(lastUpdateTime + US2ST(usPerRpmInc));
    }

    chThdExit(1);

    return 0;
}

