struct light {
	int pin;
	int power;
	int blinking;
};

struct lights {
	struct light el;
	struct light head;
	struct { struct light l, r; } front, back;
};

extern void lights_init(struct lights *);
extern void lights_read(struct lights *);
extern void lights_update(struct lights *);

