#ifndef RMDUP_H
#define RMDUP_H

#include<stdio.h>
#include<sys/wait.h>
#include<string.h>
#include<time.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#include<stdlib.h>
#include<unistd.h>


/**
 * struct used to store the file path for a file
 */
typedef struct {
	char* path;
	char* name;
} file_path; 

/**
 * struct used to store duplicate files
 * fp is the file path for the file
 * num_dups is the number of duplicates this file has (used to iterate the array)
 */
typedef struct{
	file_path* fp;
	int num_dups;
}dup_file;

/**
 * \brief function that checks if two given files are the same
 *
 * two regular files are the same when they have the same name, same size, same content and same permissions
 * @ret 0 if the files are the not the same
 * @ret 1 if the files are the same
 */
int same_files(const file_path file1,const file_path file2); 

/**
 * \brief checks for duplicates in an array of pointers to files
 *
 * @ret array of arrays of file_paths storing all the instances of equal files found
 * 
 * the bidimensional arrays will be sorted by oldest modification so that the first element of an array (v[i][0]) will be the oldest
 */
dup_file **check_duplicate_files(const char* filepath, file_path *files, int files_size, int *n_dupicates); 


/**
 *  \brief prints the information about a file
 */
void print_file(const char* path, int output);

/**
 *  \brief reads a file and returns a pointer to a file_path struct that represents it
 */
file_path* read_from_file(const char* filepath, int* size); 

/**
 * \brief function used to order files by date of modification
 * 
 * @ret 1 if file1 is younger, 0 if equal, -1 if older
 * */
int comp_func(const void* file1, const void*  file2);

/**
 * \brief function used to create the links between the duplicate files
 */
void create_links(dup_file** duplicates, int n_duplicates); 

#endif
