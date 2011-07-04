#include <stdio.h>
#include <time.h>
#include "../elm/lib/elm.h"
#include "../elm/lib/elmlib.h"
#include <xview/xview.h>
#include <xview/notify.h>

static char feature[100];

static Notify_value
lm_alarm(client, which)
Notify_client client;
int which;
{
	feature[0]='\0';
	(void)elm_alive(feature);
	return(NOTIFY_DONE);
}

#define ITIMER_NULL ((struct itimerval *)0)

void
align_heartbeat2()
{
        static int my_client_object;
        int *elm_client = &my_client_object;
        struct itimerval elm_timer;

        elm_timer.it_interval.tv_usec =0;
        elm_timer.it_interval.tv_sec =ALIGN_BEAT_RATE;
        elm_timer.it_value.tv_usec =0;
        elm_timer.it_value.tv_sec =ALIGN_BEAT_RATE;

        (void)notify_set_itimer_func(elm_client, lm_alarm,ITIMER_REAL,
                &elm_timer, ITIMER_NULL);

        return;
}
