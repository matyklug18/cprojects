#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "app.h"

char cmd[100] = "";

//function for getting the text. gets executed in the interval, plus every time it is needed. (resize, expose...)
void get_out(char* out) {
	char text[100] = "";
	FILE *p;
	char ch;
	p = popen(cmd,"r");
	while( (ch=fgetc(p)) != EOF)
		strncat(text, &ch, 1);
	text[strlen(text)-1] = 0;
	pclose(p);
	strcpy(out, text);
}

int main(int argc, char* argv[]) {
	strcpy(cmd, "date");

	//initialize the app (height, size, pos)
	main_window app = init((app_params){16, 8, 11});

	//start the app
	start(&app, get_out, 1.0);

	//block, because both functions are async
	while(True) {
	  //do nothing
	}
	//sleep(5);
}
