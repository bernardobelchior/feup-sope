#include<dirent.h>
#include<errno.h>
#include<sys/types.h>
#include<fcntl.h>
#include<stdlib.h>
#include<time.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>

typedef struct {
	char* name;
	char* path;
} file_path; 

file_path* (*files);
int file_no;

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

int is_dirent_a_directory(const char* full_path) {
	struct stat file_info;
	if(stat(full_path, &file_info) == -1) {
		perror(strerror(errno));
		perror("Error getting data about directory.");
		return -1;
	}
	return S_ISDIR(file_info.st_mode);
}

void print_directory(const char* name, int output) {

	DIR* directory = opendir(name);
	struct dirent* child;

	while((child = readdir(directory)) != NULL) {
		if(strcmp(child->d_name, ".") && strcmp(child->d_name, "..")) { //directories must not be this or this parent
			char path[strlen(name)+strlen(child->d_name)+2];
			strcpy(path, name);
			strcat(path, child->d_name);
			int is_dir = is_dirent_a_directory(path); 
			if (is_dir == 1) { //If it is a directory
				strcat(path, "/");
				write(output, path, strlen(path));
				write(output, "\n", strlen("\n"));
				print_directory(path, output);
			} else if(is_dir == 0) { //If it is a file
				print_file(path, output);
				write(output, child->d_name, strlen(child->d_name));
				write(output, "\n", strlen("\n"));
			} //If there was an error, skip to the next node.
		}
	}

}

void read_directory(int pipe, const char* dir_path) {
	DIR* directory;
	struct dirent* child;		

	if((directory = opendir(dir_path)) == NULL) {
		perror(strerror(errno));
		perror("The directory ");
		perror(dir_path);
		perror(" could not be open.\n");
	}

	while((child = readdir(directory)) != NULL) {
		int is_dir = is_dirent_a_directory(dir_path);
		if(is_dir == 1) {
			//fork
			int pid, fd[2];
			char path[strlen(dir_path)+strlen(child->d_name)+1];
			strcpy(path, dir_path);

			if(pipe(fd) < 0) {
				perror("Error creating pipe.\n");
			}

			if((pid = fork()) == -1) {
				//in case of error
				perror(strerror(errno));
				perror("Error creating a child.\n");
			}	else if (pid >  0) {
				//father
				close(fd[1]); //close the sender
				if((waitpid(pid, NULL, 0)) < 0) {
					perror("waitpid error.\n");
				}
				char temp[200]; //dynamic allocate memory afterwards
				//read path
				read(fd[0], temp, 200);
				write(pipe, temp, 200);
				//read name
				read(fd[0], temp, 200);
				write(pipe, temp, 200);
				//TODO: i was her
			} else {
				//son
				strcat(path, child->d_name);
				close(fd[0]); //close the receiver
				read_directory(path, fd[1]);	
			}
		} else if(is_dir == 0) {
			//send to father.
			write(pipe, path, strlen(path) + 1);
			write(pipe, name, strlen(name) + 1);
		}
	}
}

int main(int argc, char* argv[]){
	char* filepath = "./files.txt";
	int file = open(filepath, O_WRONLY | O_TRUNC | O_CREAT);
	int pid;
	int fd[2];
	
	if(file == -1) {
		perror(strerror(errno));
		perror("Redirecting to console...\n");
		file = STDOUT_FILENO;
	}

	char *header = "Serial number | File size | Modification Date | File name\n";
	write(file, header, strlen(header));

	files = (file_path**) malloc(10*sizeof(file_path*));
	file_no = 0;

	if(pipe(fd) < 0) {
		perror("Error creating main pipe.\n");
		exit(1);
	}
		
	
	if((pid = fork()) < 0) {
		perror("Error creating main fork.\n");
	} else if (pid > 0) {
		//father
		//do the sorting and whatever.
		close(fd[1]);			
	} else {
		//son
		close(fd[0]);
		read_directory(argv[1], fd[1]);
	}
	return 0;
}
