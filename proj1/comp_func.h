#include<dirent.h>
#include<errno.h>
#include<sys/types.h>
#include<fcntl.h>
#include<stdlib.h>
#include<time.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>



/**
 *\brief Function used to compare two files
 *
 * any two regular files are considered equal if they have the same name, same size, same content and same permissions
 *
 * @arg file1 file to compare
 * @arg file2 file to compare
 * @ret 0 if the files are equal
 */

int comp_func(const void* file1, const void* file2);
