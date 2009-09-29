#include <pspkernel.h>
#include <pspdebug.h>
#include <stdio.h>
#include <string.h>

#include "sml.h"

#define MAX_FILES 1024

char files_list[MAX_FILES][256];
char current_dir[1024];
char file_name[1024];

void one_dir_up() {
	char tmp[1024];
	char *p;
	int n = 0;
	int i = 0;

	// Gets number of slashes (directories)
	while(current_dir[i]) {
		if(current_dir[i] == '/') {
			n++;
		}

		i++;
	}

	if(n <= 1)
		return;

	strcpy(tmp, current_dir);
	memset(current_dir, 0, 1024);

	i = 0;
	p = strtok(tmp, "/");
	while (p != NULL && i < (n-1)) {
		strcat(current_dir, p);
		strcat(current_dir, "/");
		p = strtok(NULL, "/");
		i++;
	}
}

int is_file( const char *filename) {
	SceIoStat stats;

	sceIoGetstat(filename, &stats);

	if (stats.st_mode & FIO_S_IFDIR)
		return 0;
	else
		return 1;
}

int read_directory(char *directory) {
	SceIoDirent dir;
	int dfd;
	int i = 0;

	dfd = sceIoDopen(directory);

	if (dfd <= 0) {
		printf("Could not open directory\n");
		return 0;
	}

	if(strcmp(directory, "ms0:/") != 0) {
		strcpy(files_list[i++], "..");
	}

	memset(&dir, 0, sizeof(SceIoDirent));

	while (sceIoDread(dfd, &dir) > 0) {
		if ((dir.d_stat.st_attr & FIO_SO_IFDIR) && (dir.d_name[0] != '.')) {
			sprintf(files_list[i++], "%s/", dir.d_name);
		} else {
			strcpy(files_list[i++], dir.d_name);
		}

		if(i > MAX_FILES - 1)
			break;
	}

	sceIoDclose(dfd);

	return i;
}

char *filebrowser(char *initial_dir) {
	int i, opt;

	strcpy(current_dir, initial_dir);

	memset(file_name, 0, 1024);

	for(;;) {
		i = read_directory(current_dir);
		opt = sml_menubox(1, 2, 66, 31, &files_list[0][0], i, 256, 0, 0xFFFFFFFF, 0x00000000, 0x00000000, 0xFFFFFFFF);

		if(opt < 0)
			continue;


		if(strcmp(files_list[opt], "..") == 0) {
			one_dir_up();
			continue;
		}

		sprintf(file_name, "%s%s", current_dir, files_list[opt]);

		if(!is_file(file_name)) {
			strcpy(current_dir, file_name);
			continue;
		}

		break;
	}

	return file_name;
}
