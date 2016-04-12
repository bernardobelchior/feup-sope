#include "rmdup.h"

int comp_func(const void* file1, const void* file2){

	//checking case
	
	char name1[50],name2[50];
	strcpy(name1, ((file_path*) file1)->name);
	strcpy(name2, ((file_path*) file2)->name);

	if(name1[0] > 'A' && name1[0] < 'Z')
		name1[0] += 20;
	
	if(name2[0] > 'A' && name2[0] < 'Z')
		name2[0] += 20;

	int ret = strcmp(name1,name2);
	return ret;
}

int same_files(file_path* file1, file_path* file2) {
	//TODO check permissions and content	
	return 0;
}

void check_duplicate_files(const char* filepath, file_path* *files, int files_size) {
	int i;	
	for(i = 0; i < files_size; i++) {
		//TODO do the actual check
	}

}

void print_file(const char* path, int output) {
	struct stat file_info;
	if(stat(path, &file_info) == 1) {
		fprintf(stderr, "Error getting data about a file. (%s)\n", strerror(errno));
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

file_path* read_from_file(const char* filepath, int* size) {

	file_path* files = (file_path*) malloc(0);

	struct stat file_info;

	if(stat(filepath, &file_info) == -1) {
		fprintf(stderr, "The file could not be open. Exiting... (%s)\n", strerror(errno));
		exit(2);
	}

	FILE* file = fopen(filepath, "r");

	char path_buffer[200], name_buffer[100];
	int i = 0;

	while( fscanf(file, "%s %s",path_buffer, name_buffer) != EOF){
		
		files = realloc(files, (i+1) * sizeof(file_path));
		files[i].name = (char *)malloc(50*sizeof(char));
		files[i].path = (char *)malloc(200*sizeof(char));
		
		strcpy(files[i].name,name_buffer);
		strcpy(files[i].path,path_buffer);
		i++;

	}

	*size = i;

	return files;
}

int main(int argc, char* argv[]) {

	if(argc != 2) {
		fprintf(stderr, "Invalid number of arguments. \nProgram must be called as:\n./rmdup <directory>\nWhere <directory> is a valid directory that ends in \'/\'.\n");
		return 1;
	}

	const char* filepath = "./files.txt";
	int pid = fork();
	int files_size = 0;
	int i = 0;

	if(pid < 0){ //error
		fprintf(stderr,"main: fork() failed!\n");
	}

	if(pid == 0) { //child uses lsdir to list all the files in the directory and in its subdirectories and stores the list in a text file
		execlp("./lsdir", "lsdir", argv[1], filepath, NULL);
		exit(0);
	} 
	
	else {  //father waits for lsdir to finish
		waitpid(pid, NULL, 0);
	}
	
	file_path* files =	read_from_file(filepath, &files_size);
	qsort(files, files_size, sizeof(file_path), comp_func);

	//FIXME for debug purposes, delete when done
	for(i = 0; i < files_size; i++)
		printf("%s%s\n",files[i].path, files[i].name);
	
	//check_duplicate_files(filepath, files, files_size); TODO

	return 0;
}
