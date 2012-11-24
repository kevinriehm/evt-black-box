// period is in milliseconds
extern pthread_t *schedule(int period, void (*callback)(void *), void *arg);

