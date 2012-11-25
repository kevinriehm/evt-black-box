#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <pthread.h>

#include "data.h"
#include "scheduler.h"
#include "serial.h"


#define SAMPLE_DELAY_MS 100


static datum_t currentdatum;
static pthread_mutex_t *datummutex;
static schedule_t datathread;

static time_t unixepoch;


static void *read_data(void *arg)
{
	char buf[31];
	datum_t datum;

	// Get the data
	datum.time = difftime(time(NULL),unixepoch);

	serial_cmd(buf,30,"a0");
	datum.potentiometer = atoi(buf);

	serial_cmd(buf,30,"gp");
	sscanf(buf,"%lf,%lf",&datum.latitude,&datum.longitude);

	// Store it in a thread-safe manner
	pthread_mutex_lock(datummutex);
	currentdatum = datum;
	pthread_mutex_unlock(datummutex);

	return NULL;
}

void data_init()
{
	struct tm epoch = {
		.tm_year = 70, // 1970
		.tm_mon = 0,   // January
		.tm_mday = 1,  // 1st
		.tm_hour = 0,  // 00
		.tm_min = 0,   //   :00
		.tm_sec = 0    //      :00
	};

	unixepoch = mktime(&epoch);

	// Spin off another thread to keep stuff as in sync as possible
	datummutex = calloc(1, sizeof *datummutex);
	pthread_mutex_init(datummutex,NULL);
	datathread = schedule(SAMPLE_DELAY_MS,(callback_t) read_data,NULL);
}

void data_stop()
{
	unschedule(datathread);
	pthread_mutex_destroy(datummutex);
}

void data_get(datum_t *datum)
{
	pthread_mutex_lock(datummutex);
	*datum = currentdatum;
	pthread_mutex_unlock(datummutex);
}

datum_value_id_t data_get_id(char *value)
{
	if(strcmp(value,"time") == 0) return DATUM_TIME;
	else if(strcmp(value,"potentiometer") == 0) return DATUM_POTENTIOMETER;
	else if(strcmp(value,"latitude") == 0) return DATUM_LATITUDE;
	else if(strcmp(value,"longitude") == 0) return DATUM_LONGITUDE;
	else return DATUM_INVALID;
}

void data_get_value(void *value, datum_value_id_t id)
{
	pthread_mutex_lock(datummutex);
	switch(id)
	{
		case DATUM_TIME: *(uint64_t *) value = currentdatum.time; break;
		case DATUM_POTENTIOMETER: *(uint16_t *) value = currentdatum.potentiometer; break;
		case DATUM_LATITUDE: *(double *) value = currentdatum.latitude; break;
		case DATUM_LONGITUDE: *(double *) value = currentdatum.longitude; break;
		case DATUM_INVALID: break;
	}
	pthread_mutex_unlock(datummutex);
}
