/*
 * Angel daemon for receiving remote data from the cars
 */

/*
 * Car message format:
 *
 * '{' fields '}'
 *
 * fields:
 *  't' integer(milliseconds) ';'
 *  'g' float(latitude) ',' number(longitude) ';'
 */

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <inttypes.h>
#include <poll.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sqlite3.h>


#define END_PARAM(param) { \
	nparam++; \
	type = PARAM_NONE; \
	(param) = NULL; \
	continue; \
}


enum {
	FIELD_GPS,
	FIELD_TIME,

	FIELD_COUNT,
	FIELD_NONE
};

enum {
	PARAM_REAL,
	PARAM_INTEGER,

	PARAM_COUNT,
	PARAM_NONE
};

struct datum {
	int64_t milliseconds; // As per Unix time
	double latitude, longitude;
};


int pidfd;
sqlite3 *db;
int serialfd;


void die(char *msg) {
	fprintf(stderr,"fatal: %s\n",msg);
	exit(EXIT_FAILURE);
}

void kill_daemon() {
	int n;
	char c;
	pid_t pid;

	for(pid = 0, n = 1, c = ' '; n && (isspace(c) || isdigit(c));
			n = read(pidfd,&c,1))
		if(isdigit(c)) 
			pid = 10*pid + c - '0';

	if(pid > 0) kill(pid,SIGTERM);
}

void init_com() {
	int err;
	char *msg;
	char *dbname, *serialname;

	umask(0);

	serialname = getenv("ANGEL_SERIAL");
	if(!serialname) serialname = "/dev/ttyACM0";
	serialfd = open(serialname,O_RDONLY);
	if(serialfd < 0) die("cannot open serial device");

	dbname = getenv("ANGEL_DB");
	if(!dbname) dbname = "angel.sqlite";
	err = sqlite3_open(dbname,&db);
	if(err != SQLITE_OK) die("cannot open database");

	sqlite3_exec(db,"PRAGMA journal_mode=WAL",NULL,NULL,&msg);
	if(msg) die(msg);
	sqlite3_exec(db,"CREATE TABLE IF NOT EXISTS cars ("
		"car TEXT, time INTEGER, latitude REAL, longitude REAL"
	")",NULL,NULL,&msg);
	if(msg) die(msg);
}

void store(struct datum *datum) {
	sqlite3_stmt *stmt;

	sqlite3_prepare_v2(db,"INSERT INTO cars ("
		"car, time, latitude, longitude"
	") VALUES ("
		"?, ?, ?, ?"
	")",-1,&stmt,NULL);

	sqlite3_bind_text(stmt,1,"alpha",-1,SQLITE_TRANSIENT);
	sqlite3_bind_int64(stmt,2,datum->milliseconds);
	sqlite3_bind_double(stmt,3,datum->latitude);
	sqlite3_bind_double(stmt,4,datum->longitude);

	sqlite3_step(stmt);

	sqlite3_finalize(stmt);
}

void make_daemon() {
	pid_t pid, sid;

	pid = fork();
	if(pid < 0) die("cannot fork");
	if(pid > 0) exit(EXIT_SUCCESS);

	

	/* Finish separation */
	sid = setsid();
	if(sid < 0) die("cannot set sid");

	if(chdir("/") < 0) die("cannot change directory");

	/* Might as well */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
}

void parse(char *buf, int n) {
	static struct datum datum = {0,0,0};

	static double *real = NULL;
	static int64_t *integer = NULL;

	static int indatum = 0, field = FIELD_NONE, nparam = 0,
		type = PARAM_NONE;

	static int fraction = 0;

	int gpstypes[] = {PARAM_REAL,PARAM_REAL,PARAM_NONE};
	int timetypes[] = {PARAM_INTEGER,PARAM_NONE};
	int *typetable[] = {gpstypes,timetypes};

	void *gpsparams[] = {&datum.latitude,&datum.longitude,NULL};
	void *timeparams[] = {&datum.milliseconds,NULL};
	void **paramtable[] = {gpsparams,timeparams};

	void **params[] = {(void **) &real,(void **) &integer};

	char *fieldchars = "gt";

	int i;
	char str[2];

	str[1] = '\0';

	for(i = 0; i < n; i++) {
reparsechar:
		if(!indatum) {
			if(buf[i] == '{') indatum = 1;
			continue;
		}

		if(buf[i] == '}') {
			store(&datum);
			memset(&datum,0,sizeof datum);
			indatum = 0;
			continue;
		}

		switch(type) {
		case PARAM_NONE:
			switch(field) {
			case FIELD_NONE:
				str[0] = buf[i];
				field = strcspn(fieldchars,str);
				if(field >= FIELD_COUNT) field = FIELD_NONE;
				break;

			default:
				type = typetable[field][nparam];
				if(type == PARAM_NONE) {
					field = FIELD_NONE;
					nparam = 0;
					goto reparsechar;
				}

				*params[type] = paramtable[field][nparam];

				goto reparsechar;
			}
			break;

		case PARAM_INTEGER:
			if(!integer) END_PARAM(integer);

			if('0' <= buf[i] && buf[i] <= '9')
				*integer = 10**integer + buf[i] - '0';
			else END_PARAM(integer);
			break;

		case PARAM_REAL:
			if(!real) END_PARAM(real);

			if(buf[i] == '.') {
				if(fraction) {
					fraction = 0;
					END_PARAM(real);
				} else {
					fraction = 1;
					continue;
				}
			}

			if('0' > buf[i] || buf[i] > '9') {
				fraction = 0;
				END_PARAM(real);
			}

			if(fraction)
				*real += (buf[i] - '0')*pow(10,-(fraction++));
			else *real = 10**real + buf[i] - '0';
			break;
		}
	}
}

void monitor() {
	ssize_t nread;
	char buf[1024];
	struct pollfd pollfd;

	pollfd.fd = serialfd;

	while(1) {
		poll(&pollfd,1,-1);

		nread = read(serialfd,buf,1024);
		parse(buf,nread);
	}
}

int main(int argc, char **argv) {
	int kill, live;
	char *home, *pidname;

	/* Set defaults */
	kill = 0;
	live = 0;

	/* Parse arguments */
	if(argc <= 1) goto help;

	if(strcmp(argv[1],"start") == 0) live = 1;
	else if(strcmp(argv[1],"restart") == 0) kill = live = 1;
	else if(strcmp(argv[1],"stop") == 0) kill = 1;
	else {
		/* Fools! */
help:
		puts("usage: angeld (start|restart|stop)");
		exit(EXIT_SUCCESS);
	}

	/* Check PID file */
	if(kill || live) {
		home = getenv("HOME");
		pidname = calloc(strlen(home) + strlen("/.angeld.pid") + 1,
			sizeof *pidname);
		strcpy(pidname,home);
		strcpy(pidname + strlen(home),"/.angeld.pid");

		pidfd = open(pidname,O_RDWR|O_CREAT);
		if(pidfd < 0) pidfd = open(pidname,O_RDONLY|O_CREAT);

		free(pidname);

		if(pidfd < 0) die("cannot open PID file");

		kill = kill && flock(pidfd,LOCK_EX|LOCK_NB) < 0;
	}

	/* To be or not to be... */
	if(kill) kill_daemon();

	if(live) {
		init_com();
		//make_daemon();
		monitor();
	}

	return 0;
}

