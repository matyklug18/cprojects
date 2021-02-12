#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

//xlib, for interacting with the X server
#include <X11/Xatom.h>

//cairo, for drawing
#include <cairo.h>
#include <cairo-xlib.h>

#include "app.h"

//initialize the app, taking the initial size of the window as a parameter
main_window init(const app_params params) {
	//the display, representing the connection to X
	Display *dsp;
	//the window, representing the X window for this app
	Drawable da;
	//the screen, representing the monitor (?)
	int screen;
	//the cairo surface, used when drawing
	cairo_surface_t *sfc;

	//open the display, and in case it fails, exit
	if ((dsp = XOpenDisplay(NULL)) == ((void *)0))
		exit(1);

	//set the screen to the default screen (?)
	screen = DefaultScreen(dsp);

	//create the window, telling X it exists without showing it
	da = XCreateSimpleWindow(dsp, DefaultRootWindow(dsp), 0, 0, 1, params.height, 0, 0, 0);

	//tell xorg, that we want mouse button events, resize events and repaint needed events (?).
	XSelectInput(dsp, da, ButtonPressMask | StructureNotifyMask | ExposureMask);

	//make it into a bar/dock, by setting the "_NET_WM_WINDOW_TYPE" to "_NET_WM_WINDOW_TYPE_DOCK"
	Atom type = XInternAtom(dsp, "_NET_WM_WINDOW_TYPE", False);
	long value = XInternAtom(dsp, "_NET_WM_WINDOW_TYPE_DOCK", False);
	XChangeProperty(dsp, da, type,
			XA_ATOM, 32, PropModeReplace, (unsigned char *) &value, 1);

	//move the bar to the bottom of the screen
	XMoveWindow(dsp, da, 0, DisplayWidth(dsp, screen));

	//actually show the window
	XMapWindow(dsp, da);

	//create the cairo surface, so we can draw to the window
	sfc = cairo_xlib_surface_create(dsp, da, DefaultVisual(dsp, screen), 1, params.height);

	//resize the surface (?)
	cairo_xlib_surface_set_size(sfc, 1, params.height);

	//create the cairo type, which is usually used when drawing
	cairo_t* ctx = cairo_create(sfc);
	
	//make the struct
	main_window win_return = (main_window) {ctx, sfc, da, dsp};

	win_return.font_pos = params.font_pos;
	win_return.font_size = params.font_size;
	
	return win_return;
}

//the paint function, paints on the window using the cairo variables
void paint(main_window* app, const char* text_in) {
	// (?)
	cairo_push_group(app->cairo);

	//set the background color to a dark color (#282a36)
	cairo_set_source_rgb(app->cairo, 0.157, 0.165, 0.212);

	//paint the background with the color
	cairo_paint(app->cairo);

	//set the drawing location to (50, 32)
	cairo_move_to(app->cairo, app->font_size / 2, app->font_pos);

	//select the font
	cairo_select_font_face(app->cairo, "Cascadia Code", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

	//set the size of the font
	cairo_set_font_size(app->cairo, app->font_size);

	//change the color from black to a light color (#f8f8f2)
	cairo_set_source_rgb(app->cairo, 0.973, 0.973, 0.949);

	//render the text in the *text* variable
	cairo_show_text(app->cairo, text_in);

	// (?)
	cairo_pop_group_to_source(app->cairo);
	cairo_paint(app->cairo);

	//update the cairo surface of the window, then the window itself
	cairo_surface_flush(app->cairo_surface);
	XFlush(app->display);
}

//when the window is closed or the app crashed, destroy cairo and close the connection to X.
//if this isnt done, we get an error.
void exit_app(main_window* app) {
	cairo_destroy(app->cairo);
	XCloseDisplay(app->display);
}

//update the text && repaint 
void update_text(main_window* app) {
	char out[100];
	app->text_update(out);
	paint(app, out);
}

//a function that runs asynchronously from the main thread, managing the functionality of the app
//modifies the text painted in paint() to the output of the `date` command.
//used just for testing.
void* update(void* inp_app) {
	main_window* app = (main_window*) inp_app;
	while(True) {
		update_text(app);
		sleep(app->interval);
	}
	return NULL;
}

//simple function to resize the cairo surface && repaint it
void resize(main_window* app, const unsigned int width, const unsigned int height) {
	//resize the cairo surface
	cairo_xlib_surface_set_size(app->cairo_surface, width, height);

	//repaint the app
	update_text(app);
}

//event loop
void* event_loop(void* inp_app) {
	main_window* app = (main_window*) inp_app;
	//the event loop. when an event is recieved, its managed in one of the IFs inside
	for(;;) {
		//declare the event variable
		XEvent e;

		//fill the event variable with the next event
		//wait if there is no next event
		XNextEvent(app->display, &e);

		//if the event recieved was ButtonPress, which is recieved when a mouse button is pressed...
		if(e.type == ButtonPress) {
			//get the events data from xbutton,
			//which is the data for the event, since the type is ButtonPress.
			XButtonEvent ev = e.xbutton;
			printf("%d, %d\n\n", ev.x, ev.y);
			fflush(stdout);
		}

		//if the event was ConfigureNotify, which is usually reported when the window, for example, changes size...
		if(e.type == ConfigureNotify) {
			//call the resize() function
			resize(app, e.xconfigure.width, e.xconfigure.height);
		}

		//if the event was ConfigureNotify, which is usually reported, when the window needs redrawing, because, for example, new area was exposed.
		if(e.type == Expose) {
			update_text(app);
		}
  }

	//because the infinite loop exited, close the app
	exit_app(app);

	return NULL;
}

void start(main_window* app, void (*text_update)(char*), float interval) {
	//set few basic vars
	app->text_update = text_update;
	app->interval = interval;

	//paint the app before any events occur, so its not just empty (not sure if required)
	update_text(app);

	//start the update loop
	pthread_t update_thread_id;
	pthread_create(&update_thread_id, NULL, update, app);

	//start the event loop
	pthread_t event_thread_id;
	pthread_create(&event_thread_id, NULL, event_loop, app);
}
