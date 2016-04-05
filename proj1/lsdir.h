#ifndef LSDIR_H
#define LSDIR_H

typedef struct {
	char* name;
	char* path;
} file_path; 

int is_directory(const char* full_path);

void read_directory(int file, const char* dir_path);

void list_dir(const char* dir_path);

#endif
