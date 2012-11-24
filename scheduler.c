#include <pthread.h>
#include <time.h>


typedef void (*callback_t)(void *);

typedef struct {
	int period;
	callback_t callback;
	void *arg;
} schedule_data_t;


static void *scheduler_func(void *arg) {
	schedule_data_t *data = arg;
	struct timespec lasttime, nexttime;

	clock_gettime(CLOCK_MONOTONIC,&nexttime);

	while(1) {
		lasttime = nexttime;

		nexttime.tv_sec += data->period/1000;
		nexttime.tv_nsec += (uint32_t) data->period%1000*1000000;

		clock_nanosleep(CLOCK_MONOTONIC,TIMER_ABSTIME,&nexttime,NULL);

		data->callback(data->arg);
	}
}

pthread_t *schedule(int period, callback_t callback, void *arg) {
	pthread_t thread;
	schedule_data_t *data;

	data = calloc(1,sizeof *data);
	data->period = period;
	data->callback = callback;
	data->arg = arg;

	pthread_create(&thread,NULL,scheduler_func,data);

	return thread;
}

