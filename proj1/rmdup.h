#include<stdio.h>
#include<sys/wait.h>
#include<string.h>
#include<time.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#include<stdlib.h>
#include<unistd.h>


typedef struct {
	char* path;
	char* name;
} file_path; 

/**
 * \brief function that checks if two given files are the same
 *
 * @ret 0 if the files are the not the same
 * @ret 1 if the files are the same
 */
int same_files(file_path* file1, file_path* file2); 

/**
 * \brief checks for duplicates in an array of pointers to files
 *
 */
void check_duplicate_files(const char* filepath, file_path* *files, int files_size); 


/**
 *  \brief prints the information about a file
 */
void print_file(const char* path, int output);

/**
 *  \brief reads a file and returns a pointer to a file_path struct that represents it
 */
file_path* read_from_file(const char* filepath, int* size); 

/**
 * \brief function used to order files by alphabetical order
 * 
 * @ret 1 if file1 > file2, 0 if equal, -1 if lower
 * */
int comp_func(const void* file1, const void*  file2);
