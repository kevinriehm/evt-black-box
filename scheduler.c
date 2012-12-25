#include <stdlib.h>

#include <pthread.h>
#include <stdint.h>
#include <time.h>

#include "scheduler.h"


typedef struct {
	int periodms;
	callback_t callback;
	void *arg;
} schedule_data_t;


static uint64_t ceiling(uint64_t x, int mod) {
	x += mod - 1;
	return x - x%mod;
}

static void *scheduler_func(void *arg) {
	struct timespec time;
	schedule_data_t *data = arg;
	uint64_t startms, nowms, nextms;

	clock_gettime(CLOCK_MONOTONIC,&time);
	startms = 1000*time.tv_sec + time.tv_nsec/1000000;

	while(1) {
		clock_gettime(CLOCK_MONOTONIC,&time);
		nowms = 1000*time.tv_sec + time.tv_nsec/1000000;

		nextms = startms + ceiling(nowms - startms + 1,data->periodms);

		time.tv_sec = nextms/1000;
		time.tv_nsec = nextms%1000*1000000;

		while(clock_nanosleep(CLOCK_MONOTONIC,TIMER_ABSTIME,&time,NULL));

		data->callback(data->arg);
	}

	return NULL;
}

schedule_t schedule(int periodms, callback_t callback, void *arg) {
	pthread_t *thread;
	schedule_data_t *data;

	thread = calloc(1,sizeof *thread);

	data = calloc(1,sizeof *data);
	data->periodms = periodms;
	data->callback = callback;
	data->arg = arg;

	pthread_create(thread,NULL,scheduler_func,data);

	return thread;
}

void unschedule(schedule_t schedule) {
	pthread_cancel(*(pthread_t *) schedule);
	free(schedule);
}

