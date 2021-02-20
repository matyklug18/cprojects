//xlib, for interacting with the X server
#include <X11/Xlib.h>

//cairo, for drawing
#include <cairo.h>

//the struct used for storing info about the app
typedef struct main_window {
	//the cairo type and surface, used when drawing
	cairo_t* cairo;
	cairo_surface_t* cairo_surface;

	//the window and the display, used when calling xlib functions.
	Window window;
	Display* display;

	//function for getting the text
	void (*text_update)(char*);

	//interval (in seconds) in which to execute the text_update function
	float interval;

	int height;
	float font_size;
	float font_pos;
} main_window;

//the struct used for storing parameters about the app
typedef struct app_params {
	int height;
	int font_size;
	int font_pos;
} app_params;

//initialize
main_window init(const app_params params);

//start
void start(main_window* app, void (*text_update)(char*), float interval);

