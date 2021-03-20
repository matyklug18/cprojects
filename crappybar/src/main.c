#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../lib/app.h"

char buff[255] = "";

//function for getting the text. gets executed in the interval, plus every time it is needed. (resize, expose...)
void get_out(char* out) {
	fgets(out, 255, stdin);
	if(strlen(out) == 0) {
		strcpy(out, buff);
	} else {
		strcpy(buff, out);
	}
}

int main(int argc, char* argv[]) {
	//initialize the app (height, size, pos)
	main_window app = init((app_params){32, 16, 22});

	//start the app
	start(&app, get_out, 0.01);

	//block, because both functions are async
	while(True) {
	  //do nothing
	}
	//sleep(5);
}
