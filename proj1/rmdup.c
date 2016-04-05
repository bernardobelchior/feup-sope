#include "lsdir.h"

#include<stdio.h>
#include<string.h>
#include<time.h>
#include<sys/stat.h>
#include<errno.h>
#include<unistd.h>

void print_file(const char* path, int output) {
	struct stat file_info;
	if(stat(path, &file_info) == 1) {
		perror(strerror(errno));
		perror("Error getting data about file.");
		return;
	}

	//Prepare file info
	char serial_no[20], size[10], modification_date[20];
	sprintf(serial_no, "%u",(unsigned int) file_info.st_ino);
	write(output, serial_no, strlen(serial_no));
	sprintf(size, "%u", (unsigned int) file_info.st_size);
	write(output, " | ", strlen(" | "));
	strftime(modification_date, 20, "%g %b %e %H:%M", localtime(&file_info.st_mtime));
	write(output, modification_date, strlen(modification_date));
	write(output, " | ", strlen(" | "));
	write(output, size, strlen(size));
	write(output, " | ", strlen(" | "));
}

int main(int argc, char* argv[]) {
	list_dir(argv[1]);
	return 0;
}
