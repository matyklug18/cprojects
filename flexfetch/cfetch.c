//usr/bin/gcc -IX11 -lX11 -Ifontconfig -lfontconfig cfetch.c -o cfetch -Os; exec ./cfetch

				 /*-------------*\
				/ |  FLEXFETCH  | \
				\ |   1.0.0.1   | /
				 \*-------------*/

/*
	CHANGELOG

	v1.0.0.1

 - changed the version format
 - made FONTC the default
 - removed forgotten readproc header
*/
#define VERSION "v1.0.0.1"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

//ARCHITECTURE
#include <sys/utsname.h>

//WM and RESOLUTION
#include <X11/Xlib.h>

//for non-arch distros
//#define NO_ARCH

//for measuirng using perf
//#define PID3

//do not fetch the actual font name
//#define NO_FONTC

//hardcoded distro
//#define DISTRO "manjaro"

//FONT
#ifndef NO_FONTC
#include <fontconfig/fontconfig.h>
#endif

Display* dpy;

char* read_file(char* path) {
	FILE *fp;
  fp = fopen(path, "r");
	char* line = NULL;
	size_t len = 0;
	getline(&line, &len, fp);
	line[strlen(line)-1] = 0;
	return line;
}

char* read_data_line(char* file, char* pattern, int offset, int line_offset) {
	FILE *fp;
	fp = fopen(file, "r");
	char* line = "";
	char* cmp = pattern;
	size_t len = 0;
	while(strstr(line, cmp) == NULL) {
		if(getline(&line, &len, fp) == -1)
			break;
	}
	for(int i = 0; i < line_offset; i++)
		getline(&line, &len, fp);
	line += offset;
	line[strlen(line)-1] = 0;
	char* rtn = malloc (sizeof (char) * strlen(line));
	strcpy(rtn, line);
	return rtn;
}


char* read_data(char* file, char* pattern, int offset) {
	FILE *fp;
	fp = fopen(file, "r");
	char* line = "";
	char* cmp = pattern;
	size_t len = 0;
	while(strstr(line, cmp) == NULL) {
		if(getline(&line, &len, fp) == -1)
			break;
	}
	line += strlen(pattern) + offset;
	line[strlen(line)-1] = 0;
	char* rtn = malloc (sizeof (char) * strlen(line));
	strcpy(rtn, line);
	return rtn;
}

char* fetch_wmname() {
  Window window = DefaultRootWindow(dpy);

  Atom nameAtom = XInternAtom(dpy,"_NET_SUPPORTING_WM_CHECK",False);
  Atom utf8Atom = XInternAtom(dpy,"WINDOW",False);
  Atom type;
  int format;
  unsigned long nitems, after;
  unsigned char *data = 0;

  XGetWindowProperty(dpy, window, nameAtom, 0, sizeof(Window),
                                    False, utf8Atom, &type, &format,
                                    &nitems, &after, &data);
	Window* wnd_out = (Window*)data;

	Window ch_window = *wnd_out;
	Atom nameAtom_ch = XInternAtom(dpy,"_NET_WM_NAME",False);
	Atom utf8Atom_ch = XInternAtom(dpy,"UTF8_STRING",False);
	Atom type_ch;
	int format_ch;
	unsigned long nitems_ch, after_ch;
	unsigned char* data_ch = 0;

	XGetWindowProperty(dpy, ch_window, nameAtom_ch, 0, sizeof(char*),
																		False, utf8Atom_ch, &type_ch, &format_ch,
																		&nitems_ch, &after_ch, &data_ch);

	return (char*)data_ch;
}

char* fetch_hostname() {
	return read_file("/proc/sys/kernel/hostname");
}

int get_ppid_from_pid(int pid) {
	char path[255] = "";
	char pid_str[255] = "";
	sprintf(pid_str, "%d", pid);
	strcat(path, "/proc/");
	strcat(path, pid_str);
	strcat(path, "/stat");
	char str[255] = "";
	strcpy(str, read_file(path));
  const char s[255] = " ";
	char *token = strtok(str, s);

	for(int i = 0; i < 3; i++) {
		token = strtok(NULL, s);
	}
	return atoi(token);
}

char* get_pid_name(int pid) {
	char path[255] = "";
	char pid_str[255] = "";
	sprintf(pid_str, "%d", pid);
	strcat(path, "/proc/");
	strcat(path, pid_str);
	strcat(path, "/comm");
	return read_file(path);
}

char* fetch_terminal() {
	return get_pid_name(get_ppid_from_pid(getppid()));
}

char* fetch_system_name() {
	char* name = read_file("/sys/class/dmi/id/product_name");
	char* ver = read_file("/sys/class/dmi/id/product_version");
	char buff[255];

	sprintf(buff, "%s %s", name, ver);
	char* rtn = malloc (sizeof (char) * strlen(buff));
	strcpy(rtn, buff);
	return rtn;
}

char* fetch_kernel() {
	return read_file("/proc/sys/kernel/osrelease");
}

char* fetch_arch() {
	struct utsname buff;
	uname(&buff);
	char* text = buff.machine;
  char* rtn = malloc (sizeof (char) * strlen(text));
  strcpy(rtn, text);
  return rtn;
}

char* fetch_os() {
	char* full_line = read_file("/etc/os-release");
	full_line += 6;
	full_line[strlen(full_line)-1] = 0;
	char buff[255];
	sprintf(buff, "%s (%s)", full_line, fetch_arch());
  char* rtn = malloc (sizeof (char) * strlen(buff));
  strcpy(rtn, buff);
  return rtn;
}

char* fetch_uptime() {
	char delim[] = " ";
	char* full_name = read_file("/proc/uptime");
	char buff[255] = "";
	strcpy(buff, full_name);
	char* rtnp = strtok(buff, delim);
	float uptime_f = strtof(rtnp, NULL);
	char* rtn = malloc (sizeof (char) * strlen(rtnp));
	sprintf(rtn, "%.6f Hours", uptime_f/3600);
	return rtn;
}

char* fetch_shell() {
#ifdef NO_ARCH
	return getenv("SHELL");
#endif
	struct dirent *de;
	DIR *dir = opendir("/var/lib/pacman/local");
	char* shell = getenv("SHELL");
	char* shell_n = strrchr(shell, '/');
	shell_n += 1;
	char out[255] = "";
	while((de = readdir(dir))) {
		if(strstr(de->d_name, shell_n) != NULL) {
				FILE *fp;
				char buff[255] = "/var/lib/pacman/local/";
				strcat(buff, de->d_name);
				strcat(buff, "/files");
				fp = fopen(buff, "r");
				char* line = "";
				char* cmp = shell;
				size_t len = 0;
				while(strstr(line, cmp) == NULL) {
					if(getline(&line, &len, fp) == -1)
						goto finish;
				}
				char buff_d[255] = "/var/lib/pacman/local/";
				strcat(buff_d, de->d_name);
				strcat(buff_d, "/desc");
				char* name = read_data_line(buff_d, "%NAME%", 0, 1);
				char* ver = read_data_line(buff_d, "%VERSION%", 0, 1);
				sprintf(out, "%s v%s", name, ver);
				break;
		}
	}
finish:
	closedir(dir);
  char buff[255] = "";
	strcpy(buff, out);
  char* rtn = malloc (sizeof (char) * strlen(buff));
  strcpy(rtn, buff);
  return rtn;
}

char* fetch_packages() {
	struct dirent *de;
	DIR *dir = opendir("/var/lib/pacman/local");
	unsigned long count = 0;
	while((de = readdir(dir))) {
		++count;
	}
	closedir(dir);
	char buff[255] = "";
	sprintf(buff, "%lu (pacman)", count);
	char* rtn = malloc (sizeof (char) * strlen(buff));
	strcpy(rtn, buff);
	return rtn;
}

char* fetch_xres() {
	Screen* scr = DefaultScreenOfDisplay(dpy);
	int width = WidthOfScreen(scr);
	int height = HeightOfScreen(scr);
	char buff[255] = "";
	sprintf(buff, "%dx%d", width, height);
	char* rtn = malloc (sizeof (char) * strlen(buff));
	strcpy(rtn, buff);
	return rtn;
}

char* fetch_gtk_widget() {
	char file[255];
	strcat(strcpy(file, getenv("HOME")), "/.config/gtk-3.0/settings.ini");
	return read_data(file, "gtk-theme-name", 1);
}

char* fetch_gtk_icon() {
	char file[255];
	strcat(strcpy(file, getenv("HOME")), "/.config/gtk-3.0/settings.ini");
	return read_data(file, "gtk-icon-theme-name", 1);
}

char* fetch_user() {
	return getenv("USER");
}

char* fetch_cpu() {
	char* name = read_data("/proc/cpuinfo", "model name", 3);
	char* cores = read_data("/proc/cpuinfo", "cpu cores", 3);
	char buff[255] = "";
	sprintf(buff, "%s (%s)", name, cores);
	char* rtn = malloc (sizeof (char) * strlen(buff));
	strcpy(rtn, buff);
	return rtn;
}

char* fetch_memtotal() {
	char mem_kb_str[255] = "";
	strcpy(mem_kb_str, read_data("/proc/meminfo", "MemTotal", 9));
	mem_kb_str[strlen(mem_kb_str)-3] = 0;
	int mem = atoi(mem_kb_str)/1024;
	char buff[255] = "";
	sprintf(buff, "%d mB", mem);
	char* rtn = malloc (sizeof (char) * strlen(buff));
	strcpy(rtn, buff);
	return rtn;
}

char* fetch_memavail() {
	char mem_kb_str[255] = "";
	strcpy(mem_kb_str, read_data("/proc/meminfo", "MemAvailable", 5));
	mem_kb_str[strlen(mem_kb_str)-3] = 0;
	int mem = atoi(mem_kb_str)/1024;
	char buff[255] = "";
	sprintf(buff, "%d mB", mem);
	char* rtn = malloc (sizeof (char) * strlen(buff));
	strcpy(rtn, buff);
	return rtn;
}

char* fetch_mem() {
	char buff[255] = "";
	sprintf(buff, "%s/%s", fetch_memavail(), fetch_memtotal());
	char* rtn = malloc (sizeof (char) * strlen(buff));
	strcpy(rtn, buff);
	return rtn;
}

void print_free(const char *fmt, const char *inp) {
  printf(fmt, inp);
  free((void*)inp);
}

char* fetch_font() {
	char path[255] = "";
	char pid_str[255] = "";
#ifdef PID3
	sprintf(pid_str, "%d", get_ppid_from_pid(get_ppid_from_pid(getppid())));
#else
	sprintf(pid_str, "%d", get_ppid_from_pid(getppid()));
#endif
	strcat(path, "/proc/");
	strcat(path, pid_str);
	strcat(path, "/maps");
	char str[255] = "";
	strcpy(str, read_data_line(path, "/usr/share/fonts", 0, 0));
#ifndef NO_FONTC
	FcInit();
  FcFontSet* fs = FcFontSetCreate();
	char* pth = malloc(sizeof(char) * strlen(str));
	strcpy(pth, str);
	pth += (int)(strchr(str, '/')-str);
	const FcChar8 *file = (FcChar8*)pth;
	FcFileScan(fs, NULL, NULL, NULL, file, FcTrue);
	FcPattern *pat = fs->fonts[0];
  FcChar8* family;
  FcPatternGetString(pat, FC_FAMILY, 0, &family);
  char* family_ch = (char*) family;
  char* rtn = malloc (sizeof (char) * strlen(family_ch));
  strcpy(rtn, family_ch);
  return rtn;
#else
	char* res = strrchr(str, '/');
	res += 1;
	res[strlen(res)-4] = 0;
  char* rtn = malloc (sizeof (char) * strlen(res));
  strcpy(rtn, res);
  return rtn;
#endif
}

int main() {
	dpy = XOpenDisplay(NULL);

	char* distro_arch = "\n\
\033[38;2;23;147;209m                   ▄\n\
                  ▟█▙\n\
                 ▟███▙\n\
                ▟█████▙\n\
               ▟███████▙\n\
              ▂▔▀▜██████▙\n\
             ▟██▅▂▝▜█████▙\n\
            ▟█████████████▙\n\
           ▟███████████████▙\n\
          ▟█████████████████▙\n\
         ▟███████████████████▙\n\
        ▟█████████▛▀▀▜████████▙\n\
       ▟████████▛      ▜███████▙\n\
      ▟█████████        ████████▙\n\
     ▟██████████        █████▆▅▄▃▂\n\
    ▟██████████▛        ▜█████████▙\n\
   ▟██████▀▀▀              ▀▀██████▙\n\
  ▟███▀▘                       ▝▀███▙\n\
 ▟▛▀                               ▀▜▙\n\
\n\
";

	char* distro_manjaro = "\n\
\033[0;32m██████████████████  ████████\n\
██████████████████  ████████\n\
██████████████████  ████████\n\
██████████████████  ████████\n\
████████            ████████\n\
████████  ████████  ████████\n\
████████  ████████  ████████\n\
████████  ████████  ████████\n\
████████  ████████  ████████\n\
████████  ████████  ████████\n\
████████  ████████  ████████\n\
████████  ████████  ████████\n\
████████  ████████  ████████\n\
████████  ████████  ████████\n\
\n\
";
#ifdef DISTRO
	char* distro_id = DISTRO;
#else
	char* distro_id = read_data("/etc/os-release", "ID=", 0);
#endif
	if(strcmp(distro_id, "arch") == 0)
		printf("%s", distro_arch);

	if(strcmp(distro_id, "manjaro") == 0)
		printf("%s", distro_manjaro);
	
  printf(    "\033[1;34mWM              \033[0;36m%s\n", fetch_wmname());
	printf(    "\033[1;34mHOSTNAME        \033[0;36m%s\n", fetch_hostname());
  printf(    "\033[1;34mTERMINAL        \033[0;36m%s\n", fetch_terminal());
  print_free("\033[1;34mSYSTEM NAME     \033[0;36m%s\n", fetch_system_name());
	printf(    "\033[1;34mKERNEL          \033[0;36m%s\n", fetch_kernel());
  print_free("\033[1;34mDISTRO          \033[0;36m%s\n", fetch_os());
  print_free("\033[1;34mUPTIME          \033[0;36m%s\n", fetch_uptime());
#ifdef NO_ARCH
  printf(    "\033[1;34mSHELL           \033[0;36m%s\n", fetch_shell());
#else
  print_free("\033[1;34mSHELL           \033[0;36m%s\n", fetch_shell());
  print_free("\033[1;34mPACKAGES        \033[0;36m%s\n", fetch_packages());
#endif
  print_free("\033[1;34mRESOLUTION      \033[0;36m%s\n", fetch_xres());
  printf(    "\033[1;34mGTK WIDGET      \033[0;36m%s\n", fetch_gtk_widget());
  printf(    "\033[1;34mGTK ICON        \033[0;36m%s\n", fetch_gtk_icon());
  printf(    "\033[1;34mUSERNAME        \033[0;36m%s\n", fetch_user());
  printf(    "\033[1;34mCPU             \033[0;36m%s\n", fetch_cpu());
  print_free("\033[1;34mMEMORY          \033[0;36m%s\n", fetch_mem());
	print_free("\033[1;34mFONT            \033[0;36m%s\n", fetch_font());
	printf(    "\033[1;34mFLEXFETCH       \033[0;36m%s\n", VERSION);
  printf(    "\033[0m");
}
