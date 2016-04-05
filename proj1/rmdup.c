#include "lsdir.h"

#include<stdio.h>
#include<string.h>
#include<time.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#include<stdlib.h>
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

file_path *read_from_file(const char* filepath, int* size) {
	int cur_size = 20;
	file_path (*files) = (file_path*) malloc(cur_size*sizeof(file_path));

	struct stat file_info;
	if(stat(filepath, &file_info) == -1) {
		fprintf(stderr, "The file could not be open. Exiting... (%s)\n", strerror(errno));
		exit(0);
	}

	FILE* file = fopen(filepath, "r");
//	int file = open(filepath, O_RDONLY);
	

	int i = -1;
//	while(!feof(file)) {
	do {
		i++;
		if(i == cur_size -1) {
			cur_size += 20;
			files = (file_path*) realloc(files, cur_size*sizeof(file_path));
		}
		
		files[i].path = (char*) malloc(200*sizeof(char));
		files[i].name = (char*) malloc(50*sizeof(char));				
	} while(fscanf(file, "%s %s\n", files[i].path, files[i].name) != EOF);

	if(feof(file))
		printf("Acabou o ficheiro! Li %i cenas.\n", i);

	files = (file_path*) realloc(files, i*sizeof(file_path));
	*size = i;

	return files;
}

int main(int argc, char* argv[]) {
	if(argc != 2) {
		fprintf(stderr, "Invalid number of arguments. \nProgram must be called as:\n./rmdup <directory>\nWhere <directory> is a valid directory that ends in \'/\'.\n");
		exit(1);
	}


	const char* filepath = "./files.txt";
	list_dir(filepath, argv[1]);
	int files_size = 0;
   file_path (*files) =	read_from_file(filepath, &files_size);

	printf("The array has size of %u.\n", files_size);

	int i;
	for(i = 0; i < files_size; i++) {
		printf("%s %s\n", files[i].path, files[i].name);
	}



	return 0;
}
