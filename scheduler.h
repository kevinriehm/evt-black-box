typedef void *schedule_t;
typedef void (*callback_t)(void *);

extern schedule_t schedule(int periodms, callback_t callback, void *arg);
extern void unschedule(schedule_t schedule);

