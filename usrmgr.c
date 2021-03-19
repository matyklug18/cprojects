#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <shadow.h>
#include <crypt.h>
#include <string.h>
#include <grp.h>

int main(int argc, char* argv[]) {
	struct passwd* passwdEntry = getpwuid(getuid());
	struct spwd* shadowEntry = getspnam(passwdEntry->pw_name);
	struct group* wheel = getgrnam("wheel");
	int ngroups = 0;
	getgrouplist(passwdEntry->pw_name, passwdEntry->pw_gid, NULL, &ngroups);
	__gid_t groups[ngroups];
	getgrouplist(passwdEntry->pw_name, passwdEntry->pw_gid, groups, &ngroups);
	for (int i = 0; i < ngroups; i++) {
		struct group* gr = getgrgid(groups[i]);
		if(gr->gr_gid == wheel->gr_gid) {
			char* password = getpass("#: ");
			if(!strcmp(shadowEntry->sp_pwdp, crypt(password, shadowEntry->sp_pwdp))) {
				int len = (argc-1);
				for(int i = 1; i < argc; i++) {
					len += strlen(argv[i]);
				}
				char* cmd = malloc(len);
				strcpy(cmd, "");
				for(int i = 1; i < argc; i++) {
					strcat(cmd, argv[i]);
					if(i < argc - 1)
						strcat(cmd, " ");
				}
				setuid(0);
				system(cmd);
				free(cmd);
			} else
				return 1;
			break;
		} else
			return 1;
	}
}
