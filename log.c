#include "angel.h"

#include <inttypes.h>

#include <curl/curl.h>


#define LOCAL_POST	"http://laptop/staevtangel/data"
#define REMOTE_POST	"http://staevt.com/angel/data"

#define LOG_DELAY_MS 1000

#define KEYSIZE 32


static CURLM *curlm;
static FILE *logfile;

static int maxpostlen;
static char *postformat = NULL;

static uint8_t key[KEYSIZE];


static Uint32 log_callback(Uint32 interval, void *param)
{
	CURL* curl;
	CURLMsg *msg;
	long respcode;
	datum_t datum;
	int activehandles, msgs;
	char *authheader, *postfields;
	uint8_t hmac[SHA256_HASH_BYTES];
	char hmacstr[2*SHA256_HASH_BYTES];
	struct curl_slist *headers = NULL;
	
	data_get(&datum);
	
	// Local file
	fprintf(logfile,"%"PRIi64",%i,%f,%f\n",
		datum.time,datum.potentiometer,datum.latitude,datum.longitude);
	
	// POST data
	postfields = calloc(maxpostlen,sizeof *postfields);
	sprintf(postfields,postformat,datum.time,datum.potentiometer,
		datum.latitude,datum.longitude);
	
	// HMAC
	authheader = calloc(15 + 2*SHA256_HASH_BYTES + 1,sizeof *authheader);
	hmac_sha256(hmac,key,KEYSIZE,postfields,strlen(postfields));
	sha256_str(hmacstr,hmac);
	sprintf(authheader,"Authorization: %s",hmacstr);
	headers = curl_slist_append(headers,authheader);
	
	// Local server
	if(!(curl = curl_easy_init()))
		die("cannot create cURL easy handle");
	curl_easy_setopt(curl,CURLOPT_URL,LOCAL_POST);
	curl_easy_setopt(curl,CURLOPT_HTTPHEADER,headers);
	curl_easy_setopt(curl,CURLOPT_POSTFIELDS,postfields);
	curl_multi_add_handle(curlm,curl);
	
	// Remote server
	if(!(curl = curl_easy_init()))
		die("cannot create cURL easy handle");
	curl_easy_setopt(curl,CURLOPT_URL,REMOTE_POST);
	curl_easy_setopt(curl,CURLOPT_HTTPHEADER,headers);
	curl_easy_setopt(curl,CURLOPT_POSTFIELDS,postfields);
	curl_multi_add_handle(curlm,curl);
	
	// Actually do the transfers
	curl_multi_perform(curlm,&activehandles);
	
	// Clean up old transfers
	while(msg = curl_multi_info_read(curlm,&msgs))
	{
		if(msg->msg != CURLMSG_DONE) // Impossible with current version of libcurl, but...
			continue;
		
		curl_easy_getinfo(msg->easy_handle,CURLINFO_RESPONSE_CODE,&respcode);
		if(respcode != 200) // Problem?
			fprintf(stderr,"HTTP %li\n",respcode);
		
		curl_multi_remove_handle(curlm,msg->easy_handle);
		curl_easy_cleanup(msg->easy_handle);
	}
	
	return interval;
}

void log_init()
{
	int i;
	FILE *f;
	time_t now;
	char logname[20];
	
	struct {
		char *name;
		char *format;
		int maxlen;
	} datumformats[] = {
		{"car",CAR,strlen(CAR)},
		{"time","%"PRIi64,20},
		{"potentiometer","%i",4},
		{"latitude","%f",11},
		{"longitude","%f",11},
		{NULL}
	};
	
	// Read the HMAC-SHA256 key
	f = fopen("key","rb");
	fread(key,1,KEYSIZE,f);
	fclose(f);
	
	// Remote tracking
	if(!(curlm = curl_multi_init()))
		die("cannot create cURL multi handle");
	
	// Local file
	now = time(NULL);
	strftime(logname,20,"%Y-%m-%d.csv",localtime(&now));
	if(!(logfile = fopen(logname,"a")))
		die("cannot open log file");
	if(ftell(logfile) == 0) // New file?
		fputs(DATUM_HEADER "\n",logfile);
	
	// Server POSTs
	for(i = 0; datumformats[i].name != NULL; i++) {
		maxpostlen += i > 0 ? strlen("&") : 0;
		maxpostlen += strlen(datumformats[i].name);
		maxpostlen += strlen("=");
		maxpostlen += datumformats[i].maxlen;
		
		postformat = realloc(postformat,maxpostlen);
		strcpy(postformat + strlen(postformat),i > 0 ? "&" : "");
		strcpy(postformat + strlen(postformat),datumformats[i].name);
		strcpy(postformat + strlen(postformat),"=");
		strcpy(postformat + strlen(postformat),datumformats[i].format);
	}
	
	// Use a callback to keep stuff as in sync as possible
	SDL_AddTimer(LOG_DELAY_MS,log_callback,NULL);
}

void log_stop()
{
	fclose(logfile);
	curl_multi_cleanup(curlm);
}
