#include <Arduino.h>

#include "com.h"
#include "lights.h"


#define BLINK_DELAY 500

#define PIL_EL_WIRE 40
#define PIN_HEAD     3
#define PIN_FRONT_L  6
#define PIN_FRONT_R  5
#define PIN_BACK_L  10
#define PIN_BACK_R   9


static unsigned long blinktime;


static void update(struct light *);


void lights_init(struct lights *lights) {
	blinktime = millis();

	lights->el.pin = PINE_EL_WIRE;
	lights->head.pin = PIN_HEAD;
	lights->front.l.pin = PIN_FRONT_L;
	lights->front.r.pin = PIN_FRONT_R;
	lights->back.l.pin = PIN_BACK_L;
	lights->back.r.pin = PIN_BACK_R;
}

void lights_read(struct lights *lights) {
	struct light *light;

	switch(com_read_cmd()) {
	case 'b':
		switch(com_read_cmd()) {
		case 'l': light = &lights->back.l; break;
		case 'r': light = &lights->back.r; break;

		case '\0':
		default: break;
		}
		break;

	case 'e': light = &lights->el; break;

	case 'f':
		switch(com_read_cmd()) {
		case 'l': light = &lights->front.l; break;
		case 'r': light = &lights->front.r; break;

		case '\0':
		default: break;
		}
		break;

	case 'h': light = &lights->head; break;

	case '\0':
	default: break;
	}

	switch(com_read_cmd()) {
	case 'p': light->power = com_read_int(); break;
	case 'b': light->blinking = com_read_int() != 0; break;
	default: break;
	}
}

void lights_update(struct lights *lights) {
	update(&lights->el);
	update(&lights->head);
	update(&lights->front.l);
	update(&lights->front.r);
	update(&lights->back.l);
	update(&lights->back.r);
}

static void update(struct light *light) {
	int blinking = (millis() - blinktime)/BLINK_DELAY & 1;

	analogWrite(light->pin,blinking && light->blinking ? 0 : light->power);
}

