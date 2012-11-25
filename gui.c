#include <EGL/egl.h>
#include <VG/openvg.h>

#include "pil.h"


// Initializes OpenVG on top of EGL and sets up the GUI from a PIL file
void gui_init() {
	pilin = fopen("angel.pil","r");
	pilparse();

	eglBindAPI(EGL_OPENVG_API);
}

void gui_stop() {
}

