#ifndef LSDIR_H
#define LSDIR_H

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

/**
 * checks if a file given by a certain filepath is a directory
 */
int is_directory(const char* full_path); 

/**
 * \brief checks if a file given by a certain filepath is a regular file
 *
 * @ret non zero if the file is regular
 */
int is_regular_file(const char* full_path);

/**
 * \brief recursively reads a directory and its children and writes their info to a file
 */ 
void read_directory(int file, const char* dir_path);

/**
 * \brief calls read_directory
 * FIXME is this really needed??
 */
int list_dir(const char* dir_path, const char* filepath);

#endif
