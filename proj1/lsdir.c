#include"lsdir.h"

#include<dirent.h>
#include<sys/wait.h>
#include<errno.h>
#include<sys/types.h>
#include<fcntl.h>
#include<stdlib.h>
#include<time.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>


int is_directory(const char* full_path) {
	struct stat file_info;
	if(stat(full_path, &file_info) == -1) {
		perror(strerror(errno));
		perror("Error getting data about directory.");
		return -1;
	}
	return S_ISDIR(file_info.st_mode);
}

void read_directory(int file, const char* dir_path) {
	DIR* directory;
	struct dirent* child;		

	if((directory = opendir(dir_path)) == NULL) {
		fprintf(stderr, "The directory %s could not be opened.\n", dir_path);
		exit(3);
	}

	while((child = readdir(directory)) != NULL) {
		int path_size = strlen(dir_path)+strlen(child->d_name)+2; 
		char path[path_size];
		strcpy(path, dir_path);
		strcat(path, child->d_name);

		int is_dir = is_directory(path);
		if(strcmp(child->d_name, ".") && strcmp(child->d_name, "..")) {
			if(is_dir == 1) {
				//fork
				int pid; 

				if((pid = fork()) == -1) {
					fprintf(stderr, "Error creating a child. (%s)\n", strerror(errno));
				}	else if (pid ==  0) {
					//son
					strcat(path, "/");
					read_directory(file, path);	
					exit(0);
				}
			} else if(is_dir == 0) {
				//write to file
				char file_line[255];
				sprintf(file_line, "%s %s\n", dir_path, child->d_name);
				write(file, file_line, strlen(file_line));
			}
		}
	}
}

void list_dir(const char* filepath, const char* dir_path){
	if(dir_path[strlen(dir_path) -1] != '/') {
		fprintf(stderr, "The directory provided is not a valid directory.\nDid you forget the \'/'\' at the end?\n");
		exit(2);
	}

	//Clears the file
	open(filepath, O_TRUNC);

	//Opens the file for writing.
	int file = open(filepath, O_WRONLY | O_APPEND | O_CREAT, S_IRWXG | S_IRWXU | S_IROTH);

	if(file == -1) {
		printf("Error opening file. Redirecting to console (%s)\n", strerror(errno));
		file = STDOUT_FILENO;
	}

	read_directory(file, dir_path); 
}
