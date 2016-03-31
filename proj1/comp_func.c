#include "comp_func.h"

int comp_func(const void* file1, const void* file2){

	//check if the  files have the same name	
	if(strcmp(((file_path) file1)->name, ((file_path) file2)->name) != 0)
		return 1;

	//create a stat struct to store the info for each file
	
	struct stat file1_info, file2_info;
	char *file1_complete_path, *file2_complete_path;

	file1_complete_path = (char *)malloc(sizeof( ((file_path) file1)->path) + sizeof( ((file_path)file1)->path));
	file2_complete_path = (char *)malloc(sizeof( ((file_path) file2)->path) + sizeof( ((file_path)file2)->path));


	strcpy(file1_complete_path, ((file_path) file1)->path);
	strcpy(file2_complete_path, ((file_path) file2)->path);

	strcat(file1_complete_path, ((file_path) file1)->name);
	strcat(file2_complete_path, ((file_path) file2)->name);

	//pass the info to the stat structs	
	if(stat(file1_complete_path,&file1_info) < 0){
		write(STDOUT_FILENO,"comp_func: stat1 returned error\n", sizeof("comp_func: stat1 returned error\n"));
		return 2;
	}	

	if(stat(file2_complete_path,&file2_info) < 0){
		write(STDOUT_FILENO,"comp_func: stat2 returned error\n", sizeof("comp_func: stat2 returned error\n"));
		return 2;
	}

	//compare the file size
	if(file1_info.st_size != file2_info.st_size)
		return 1;

	//compare the content
	int pid;
	if((pid = fork()) < 0)
		write(STDOUT_FILENO,"comp_func: fork() failed\n",sizeof("comp_func: fork() failed\n"));

	if(pid == 0){
		execlp("diff", file1_complete_path, file2_complete_path);
	}

	else{
//TODO
	}

	//compare the file modes
	if(file1_info.st_mode != file2_info.st_mode)
		return 1;
}
