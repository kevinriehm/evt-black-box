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
 *  'h' string(HMAC-SHA256) ';' // Of all other bytes in the message
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
#include <unistd.h>

#include <sqlite3.h>

#include "hmac_sha256.h"


#define VARDIR "/var/opt/staevt.com/angel"

#define XBEEDEVBASE "ttyACM"

#define END_PARAM(param) { \
	nparam++; \
	type = PARAM_NONE; \
	(param) = NULL; \
	goto newparam; \
}


enum {
	FIELD_GPS,
	FIELD_HMAC,
	FIELD_TIME,

	FIELD_COUNT,
	FIELD_NONE
};

enum {
	PARAM_INTEGER,
	PARAM_REAL,
	PARAM_STRING,

	PARAM_COUNT,
	PARAM_NONE
};

struct string {
	int cap, len;
	char *str;
};

struct datum {
	struct string hmac; // HMAC-SHA256 (in hex)
	int64_t milliseconds; // As per Unix time
	double latitude, longitude;
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
	pidfp = fopen(VARDIR"/.angeld.pid","r+");
	if(!pidfp) pidfp = fopen(VARDIR"/.angeld.pid","w");
	pidfd = fileno(pidfp);

	if(pidfd < 0) syslog(LOG_WARNING,"cannot open PID file (%m)");

	return flock(pidfd,LOCK_EX|LOCK_NB) < 0;
}

int try_open(char *path, int flags) {
	int fd;

	fd = open(path,flags);

	if(fd < 0) syslog(LOG_WARNING,"cannot open '%s' (%m)",path);
	else syslog(LOG_NOTICE,"opened '%s'",path);

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

	for(i = 0; i < g.gl_pathc; i++)
		if((serialfd = try_open(g.gl_pathv[i],O_RDONLY)))
			break;

	globfree(&g);

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
			serialfd = try_open(path,O_RDONLY);
		}
	}

	inotify_rm_watch(inotifyfd,wd);
}

void handle_hup(int sig) {
	syslog(LOG_NOTICE,"received SIGHUP");
	syslog(LOG_NOTICE,"closing serial link");
	close(serialfd);

	find_serial();

	signal(SIGHUP,handle_hup); /* Just in case */
}

void kill_daemon() {
	pid_t pid;

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

	// XBee serial device
	inotifyfd = inotify_init();
	signal(SIGHUP,handle_hup);
	find_serial();

	// SQLite3 database
	dbname = getenv("ANGEL_DB");
	if(!dbname) dbname = "/var/opt/staevt.com/angel/angel.sqlite3";
	err = sqlite3_open(dbname,&db);
	if(err != SQLITE_OK) die("cannot open database '%s'",dbname);

	sqlite3_exec(db,"PRAGMA journal_mode=WAL",NULL,NULL,&msg);
	if(msg) die(msg);
	sqlite3_exec(db,"CREATE TABLE IF NOT EXISTS cars ("
		"car TEXT, time INTEGER, latitude REAL, longitude REAL,"
		"PRIMARY KEY (car, time)"
	")",NULL,NULL,&msg);
	if(msg) die(msg);

	// HMAC key
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

	syslog(LOG_INFO,"%s, %"PRIi64", %f, %f\n",datum->hmac.str,
		datum->milliseconds,datum->latitude,datum->longitude);

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

void parse(char *buf, int n) {
	static struct string hashable = {0,0,NULL};
	static struct datum datum = {{0,0,NULL},0,0,0};

	static double *real = NULL;
	static int64_t *integer = NULL;
	static struct string *string = NULL;

	static int indatum = 0,
		field, nparam, type;

	static int fraction = 0;

	int gpstypes[] = {PARAM_REAL,PARAM_REAL,PARAM_NONE};
	int hmactypes[] = {PARAM_STRING,PARAM_NONE};
	int timetypes[] = {PARAM_INTEGER,PARAM_NONE};
	int *typetable[] = {gpstypes,hmactypes,timetypes};

	void *gpsparams[] = {&datum.latitude,&datum.longitude,NULL};
	void *hmacparams[] = {&datum.hmac,NULL};
	void *timeparams[] = {&datum.milliseconds,NULL};
	void **paramtable[] = {gpsparams,hmacparams,timeparams};

	void **params[] = {(void **) &integer, (void **) &real,
		(void **) &string};

	char *fieldchars = "ght";

	int i;
	uint8_t hmac[SHA256_HASH_BYTES];
	char hmacstr[2*SHA256_HASH_BYTES + 1], str[2];

	str[1] = '\0';

	for(i = 0; i < n; i++) {
		if(!indatum) {
			if(buf[i] == '{') {
				indatum = 1;
				field = FIELD_NONE;
				nparam = 0;
				type = PARAM_NONE;
			} else continue;
		}

		if(field != FIELD_HMAC && !(field == FIELD_NONE
			&& buf[i] == fieldchars[FIELD_HMAC]))
			append_char(&hashable,buf[i]);

		if(buf[i] == '}') {
			hmac_sha256(hmac,hmackey,hmackeysize,hashable.str,
				hashable.len);
			sha256_str(hmacstr,hmac);

			if(strieq_ct(hmacstr,datum.hmac.str))
				store(&datum);
			else fprintf(stderr,"HMAC failure: %s (should be %s)\n",
				datum.hmac.str,hmacstr);

			clear_string(&hashable);

			clear_string(&datum.hmac);
			datum.milliseconds = 0;
			datum.latitude = 0;
			datum.longitude = 0;

			indatum = 0;
			continue;
		}

		if(buf[i] == ';') {
			type = PARAM_NONE;
			nparam = 0;
			field = FIELD_NONE;
		}

		switch(type) {
		case PARAM_NONE:
			switch(field) {
			case FIELD_NONE:
newfield:
				str[0] = buf[i];
				field = strcspn(fieldchars,str);

				if(field < FIELD_COUNT) goto newparam;

				field = FIELD_NONE;
				continue;

			default:
newparam:
				type = typetable[field][nparam];
				if(type == PARAM_NONE) {
					nparam = 0;
					goto newfield;
				}

				*params[type] = paramtable[field][nparam];
				continue;
			}
			continue;

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
			continue;

		case PARAM_STRING:
			if(!string) END_PARAM(string);

			append_char(string,buf[i]);
			continue;

		case PARAM_INTEGER:
			if(!integer) END_PARAM(integer);

			if('0' <= buf[i] && buf[i] <= '9')
				*integer = 10**integer + buf[i] - '0';
			else END_PARAM(integer);
			continue;
		}
	}
}

void monitor() {
	int nready;
	ssize_t nread;
	char buf[1024];
	struct pollfd pollfd;

	pollfd.fd = serialfd;
	pollfd.events = POLLIN;

	while(1) {
		nready = poll(&pollfd,1,-1);
		if(nready < 0) syslog(LOG_WARNING,"poll error (%m)");

		nread = read(serialfd,buf,1024);
		if(nread < 0) syslog(LOG_WARNING,"read error (%m)");
		parse(buf,nread);
	}
}

int main(int argc, char **argv) {
	int alive, kill, live;

	openlog("angeld",LOG_PID|LOG_CONS,LOG_USER);

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
