#ifndef LSDIR_H
#define LSDIR_H

typedef struct {
	char* name;
	char* path;
} file_path; 

void print_file(const char* path, int output);

int is_dirent_a_directory(const char* full_path);

void print_directory(const char* name, int output);

void read_directory(int file, const char* dir_path);

void list_dir(const char* dir_path);

#endif
