#include "lsdir.h"

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

	while((child = readdir(directory)) != NULL) { 												//for every child in a directory
		int path_size = strlen(dir_path)+strlen(child->d_name)+2; 
		char path[path_size]; 																			//creates a string with the child path
		strcpy(path, dir_path);
		strcat(path, child->d_name);

		int is_dir = is_directory(path); 															//checks if the child is a dir
		if((strcmp(child->d_name, ".")!=0) && (strcmp(child->d_name, "..")!=0)) { 		//excludes the current and father dirs, to avoid endless cycles
			if(is_dir) { 									
				//fork
				int pid= fork();

				if(pid < 0) {
					fprintf(stderr, "Error creating a child. (%s)\n", strerror(errno));
				}	
				
				else if (pid ==  0) { //son
					strcat(path, "/");
					read_directory(file, path);
					exit(0);
				}

				else{ //father
					
					waitpid(pid,NULL,0);
					char file_line[255];
					sprintf(file_line, "%s %s\n", dir_path, child->d_name);//FIXME try to find a way to do this without the space
					write(file, file_line, strlen(file_line));
				}
			}
		  
			else{ //not a directory
					char file_line[255];
					sprintf(file_line, "%s %s\n", dir_path, child->d_name);  //FIXME try to find a way to do this without the space
					write(file, file_line, strlen(file_line));
			}
		}
	}
}

int list_dir(const char* dir_path, const char* filepath){
	if(dir_path[strlen(dir_path) -1] != '/') {
		fprintf(stderr, "The directory provided is not a valid directory.\nDid you forget the \'/'\' at the end?\n");
		return 2;
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
	return 0;
}

int main(int argc, char* argv[]) {
	if(argc < 2 || argc > 3) {
		fprintf(stderr, "Invalid number of arguments. \nProgram must be called as:\n./lsdir <directory>\nWhere <directory> is a valid directory that ends in \'/\'.\n");
		return 1;
	} else if (argc == 2){
		return list_dir(argv[1], "./files.txt");
	} else {
		return list_dir(argv[1], argv[2]);
	}

	return 0;
}
