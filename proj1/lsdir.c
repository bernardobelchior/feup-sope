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

int main(int argc, char* argv[]){
	char* filepath = "./files.txt";
	int file = open(filepath, O_WRONLY | O_TRUNC | O_CREAT);

	if(file == -1) {
		perror(strerror(errno));
		perror("Redirecting to console...\n");
		file = STDOUT_FILENO;
	}

	char *header = "Serial number | File size | Modification Date | File name\n";
	write(file, header, strlen(header));
	print_directory(argv[1], file);
	return 0;
}
