/*
 * Angel daemon for receiving remote data from the cars
 */

/*
 * Car message format:
 *
 * '{' fields '}'
 *
 * fields:
 *  't' ':' integer(milliseconds) ';'
 *  'a' ':' float(amperage) ';'
 *  'v' ':' float(voltage) ';'
 *  'g' ':' float(latitude) ',' float(longitude) ';'
 *  's' ':' float(speed_mph) ';'
 */

#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <glob.h>
#include <inttypes.h>
#include <poll.h>
#include <signal.h>
#include <syslog.h>
#include <sys/file.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

#include <sqlite3.h>

#include "hmac_sha256.h"


#define VARDIR "/var/opt/staevt.com/angel"

#define XBEEDEVBASE "ttyACM"
#define BAUD B9600


struct string {
	int cap, len;
	char *str;
};

struct datum {
	int64_t milliseconds; // As per Unix time
	float amperage;
	float voltage;
	float latitude, longitude;
	float speed;
};


FILE *pidfp;
sqlite3 *db;
int hmackeysize;
uint8_t *hmackey;
int inotifyfd, pidfd, serialfd;


void die(char *format, ...) {
	va_list ap;

	va_start(ap,format);
	vsyslog(LOG_CRIT,format,ap);
	va_end(ap);

	exit(EXIT_FAILURE);
}

int is_alive() {
	pidfd = open(VARDIR"/.angeld.pid",O_RDWR | O_NOCTTY);
	if(pidfd < 0) pidfd = open(VARDIR"/.angeld.pid",O_RDONLY | O_NOCTTY);

	pidfp = fdopen(pidfd,"w");
	if(!pidfp) pidfp = fdopen(pidfd,"r");

	if(pidfd < 0 || !pidfp) {
		syslog(LOG_WARNING,"cannot open PID file (%m)");
		return 0;
	}

	return flock(pidfd,LOCK_EX | LOCK_NB) < 0;
}

int try_open(char *path, int flags) {
	int fd;
	struct termios serialios;

	fd = open(path,flags);

	if(fd < 0) syslog(LOG_WARNING,"cannot open '%s' (%m)",path);
	else {
		syslog(LOG_NOTICE,"opening '%s'",path);

		// Set up the port
		memset(&serialios,0,sizeof(serialios));
		cfsetospeed(&serialios,BAUD);
		cfsetispeed(&serialios,BAUD);
		serialios.c_cflag |= CS8;
		serialios.c_iflag |= IGNBRK;
		serialios.c_lflag |= NOFLSH;
		tcsetattr(fd,TCSANOW,&serialios);
	}

	return fd;
}

/* Wait until a suitable serial device exists, then open it */
void find_serial() {
	int wd;
	glob_t g;
	int nread, i;
	struct pollfd pollfd;
	struct inotify_event *e;
	char buf[sizeof(struct inotify_event) + FILENAME_MAX + 1],
		path[FILENAME_MAX + 1];

	wd = inotify_add_watch(inotifyfd,"/dev",IN_CREATE);
	e = (struct inotify_event *) buf;

	syslog(LOG_NOTICE,"looking for serial device");

	glob("/dev/"XBEEDEVBASE"*",0,NULL,&g);

	for(i = 0; i < g.gl_pathc; i++) {
		serialfd = try_open(g.gl_pathv[i],O_RDONLY | O_NOCTTY);
		if(serialfd >= 0) return;
	}

	globfree(&g);

	syslog(LOG_INFO,"nothing there, using inotify");

	pollfd.fd = inotifyfd;
	pollfd.events = POLLIN;

	while(serialfd < 0) {
		poll(&pollfd,1,-1);

		nread = read(inotifyfd,e,sizeof(struct inotify_event)
			+ FILENAME_MAX);

		if(nread < 0) syslog(LOG_WARNING,"read error (%m)");
		else if(strncmp(e->name,XBEEDEVBASE,strlen(XBEEDEVBASE)) == 0
				&& !(e->mask & IN_ISDIR)) {
			strcpy(path,"/dev/");
			strcat(path,e->name);
			serialfd = try_open(path,O_RDONLY | O_NOCTTY);
		}
	}

	inotify_rm_watch(inotifyfd,wd);
}

void kill_daemon() {
	pid_t pid;

	if(!pidfp) {
		syslog(LOG_WARNING,"PID file not open");
		return;
	}

	fscanf(pidfp,"%i",&pid);

	if(pid > 0) {
		syslog(LOG_INFO,"sending SIGTERM to old instance");
		kill(pid,SIGTERM);
	}
}

void make_daemon() {
	int n;
	pid_t pid, sid;

	pid = fork();
	if(pid < 0) syslog(LOG_WARNING,"cannot fork");
	if(pid > 0) exit(EXIT_SUCCESS);

	syslog(LOG_INFO,"now operating as a daemon");

	/* Leave some contact info */
	n = fprintf(pidfp,"%i\n",getpid());
	if(n <= 0) syslog(LOG_WARNING,"cannot write PID to file (%m)");
	ftruncate(pidfd,n);
	fflush(pidfp);
	fsync(pidfd);

	/* Finish separation */
	sid = setsid();
	if(sid < 0) syslog(LOG_WARNING,"cannot set sid");

	if(chdir("/") < 0) syslog(LOG_WARNING,"cannot change directory");

	/* Might as well */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
}

void init_com() {
	char *msg;
	int err, fd;
	struct stat stat;
	char *dbname, *keyname;

	umask(0);

	/* XBee serial device */
	inotifyfd = inotify_init();
	find_serial();

	/* SQLite3 database */
	dbname = getenv("ANGEL_DB");
	if(!dbname) dbname = "/var/opt/staevt.com/angel/angel.sqlite3";
	err = sqlite3_open(dbname,&db);
	if(err != SQLITE_OK) die("cannot open database '%s'",dbname);

	sqlite3_exec(db,"PRAGMA journal_mode=WAL",NULL,NULL,&msg);
	if(msg) die(msg);
	sqlite3_exec(db,"CREATE TABLE IF NOT EXISTS cars ("
		"car TEXT, time_received TIMESTAMP"
			" DEFAULT (strftime('%s','now')),"
		"time INTEGER, amperage REAL, voltage REAL, latitude REAL,"
		"longitude REAL, speed REAL, PRIMARY KEY (car, time_received)"
	")",NULL,NULL,&msg);
	if(msg) die(msg);

	/* HMAC key */
	keyname = getenv("ANGEL_KEY");
	if(!keyname) keyname = "/var/opt/staevt.com/angel/hmac.key";
	fd = open(keyname,O_RDONLY);
	if(fd < 0) die("cannot open HMAC key file '%s'",keyname);
	fstat(fd,&stat);
	hmackeysize = stat.st_size;
	hmackey = malloc(hmackeysize);
	if(!hmackey) die("cannot allocate memory for HMAC key");
	read(fd,hmackey,hmackeysize);
	close(fd);
}

void store(struct datum *datum) {
	sqlite3_stmt *stmt;

	syslog(LOG_INFO,"%"PRIi64"ms, %fA, %fV, (%f, %f), %fmph\n",
		datum->milliseconds,datum->amperage,datum->voltage,
		datum->latitude,datum->longitude,datum->speed);

	sqlite3_prepare_v2(db,"INSERT INTO cars ("
		"car, time, amperage, voltage, latitude, longitude, speed"
	") VALUES ("
		"?, ?, ?, ?, ?, ?, ?"
	")",-1,&stmt,NULL);

	sqlite3_bind_text(stmt,1,"alpha",-1,SQLITE_TRANSIENT);
	sqlite3_bind_int64(stmt,2,datum->milliseconds);
	sqlite3_bind_double(stmt,3,datum->amperage);
	sqlite3_bind_double(stmt,4,datum->voltage);
	sqlite3_bind_double(stmt,5,datum->latitude);
	sqlite3_bind_double(stmt,6,datum->longitude);
	sqlite3_bind_double(stmt,7,datum->speed);

	sqlite3_step(stmt);

	sqlite3_finalize(stmt);
}

void append_char(struct string *s, char c) {
	if(s->len++ >= s->cap) {
		s->cap = 2*s->len;
		s->str = realloc(s->str,s->cap*sizeof *s->str);
		if(!s->str) syslog(LOG_ERR,"cannot reallocate string");
	}

	s->str[s->len - 1] = c;
	s->str[s->len] = '\0';
}

void clear_string(struct string *s) {
	s->len = 0;
	if(s->cap) s->str[0] = '\0';
}

char tolower_ct(char c) {
	return c + (c >= 'A')*(c <= 'Z')*0x20;
}

// Constant-time algorithm to prevent timing attacks
int strieq_ct(char *a, char *b) {
	int equal = 0;

	// Sanity checks
	if(!a || !b || strlen(a) != strlen(b)) return 0;

	while(*a) {
		equal |= tolower_ct(*a) == tolower_ct(*b);
		a++, b++;
	}

	return equal;
}

void reset_datum(struct datum *datum) {
	datum->milliseconds = 0;
	datum->amperage = -1;
	datum->voltage = -1;
	datum->latitude = 0;
	datum->longitude = 0;
	datum->speed = -1;
}

void parse(char *block) {
	struct datum datum;
	enum { START, FIELD } state;

	reset_datum(&datum);

	for(state = START; block && *block; block++) {
		switch(state) {
		case START: if(*block == '{') state = FIELD; break;

		case FIELD:
			switch(*block) {
			case 'a': sscanf(block,"a:%f;",&datum.amperage); break;
			case 'g': sscanf(block,"g:%f,%f;",&datum.latitude,
				&datum.longitude); break;
			case 's': sscanf(block,"s:%f;",&datum.speed); break;
			case 't': sscanf(block,"t:%"PRIi64";",
				&datum.milliseconds); break;
			case 'v': sscanf(block,"v:%f;",&datum.voltage); break;

			case '}':
				store(&datum);
				reset_datum(&datum);
				state = START;
				break;

			default: break;
			}

			if(state == FIELD)
				block = strchr(block,';');
			break;

		default: state = START; break;
		}
	}
}

void monitor() {
	static const int bufsize = 1024;

	int nready;
	int reading;
	ssize_t nread;
	char buf[bufsize];
	struct pollfd pollfd;

	pollfd.fd = serialfd;
	pollfd.events = POLLIN | POLLHUP;

	while(1) {
		nready = poll(&pollfd,1,-1);
		if(nready < 0) syslog(LOG_WARNING,"poll error (%m)");

		if(pollfd.revents & POLLIN) {
			buf[0] = '\0';
			reading = nread = 0;
			while(nread >= 0
				&& (!strchr(buf,'{') || !strchr(buf,'}'))) {
				nread += read(serialfd,buf + nread,nready);

				if(!reading && !strchr(buf,'{')) break;
				else reading = 1;

				if(nread >= bufsize) {
					buf[bufsize - 1] = '\0';
					break;
				}
				if(nread >= 0) buf[nread] = '\0';

				nready = 1;
			}
			if(nread < 0) syslog(LOG_WARNING,"read error (%m)");

			parse(buf);
		}

		if(pollfd.revents & POLLHUP) {
			syslog(LOG_NOTICE,"serial device removed");

			close(serialfd);
			find_serial();
			pollfd.fd = serialfd;
		}
	}
}

int main(int argc, char **argv) {
	int alive, kill, live;

	openlog("angeld",LOG_PID | LOG_CONS,LOG_USER);

	/* Set defaults */
	kill = 0;
	live = 0;

	/* Parse arguments */
	if(argc <= 1) goto help;

	if(strcmp(argv[1],"start") == 0) live = 1;
	else if(strcmp(argv[1],"restart") == 0) kill = live = 1;
	else if(strcmp(argv[1],"stop") == 0) kill = 1;
	else { /* Fools! */
help:
		puts("usage: angeld (start|restart|stop)");
		exit(EXIT_SUCCESS);
	}

	/* Check PID file */
	if(kill || live) {
		alive = is_alive();

		if(kill && !alive) syslog(LOG_INFO,"nothing to kill");
		kill &= alive;

		if(!kill && live && alive)
			syslog(LOG_INFO,"angeld already alive");
		live &= kill || !alive;
	}

	/* To be or not to be... */
	if(kill) kill_daemon();

	if(live) {
		make_daemon();
		init_com();
		monitor();
	}

	closelog();

	return !(kill || live);
}

