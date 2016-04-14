#include "rmdup.h"

int comp_func(const void* file1, const void* file2){
	
	struct stat file_info1, file_info2;

	char path1[255], path2[255];

	strcpy (path1,((file_path *)file1)->path);
	strcat (path1,((file_path *)file1)->name);

	strcpy (path2,((file_path *)file2)->path);
	strcat (path2,((file_path *)file2)->name);


	if(stat(path1, &file_info1) == 1) {
		fprintf(stderr, "Error getting data about a file. (%s)\n", strerror(errno));
		return 0;
	}

	if(stat(path2, &file_info2) == 1){
		fprintf(stderr, "Error getting data about a file. (%s)\n",strerror(errno));
		return 0;
	}

	// = file_info1.st_time - file_info2.st_time
	double time_dif = difftime (file_info1.st_mtime,file_info2.st_mtime);

	if(time_dif > 0) // file 1 was modified more recently
		return 1;

	else if(time_dif == 0) //same modify date (should never happen)
		return 0;

	else if(time_dif < 0) //file 1 is older
		return -1;

	return 0;
} 


int same_files(const file_path file1,const file_path file2) {
	//TODO check permissions and content	
	
	struct stat file_info1, file_info2;
	char path1[255],path2[255];

	strcpy(path1,file1.path);
	strcat(path1,file1.name);

	strcpy(path2,file2.path);
	strcat(path2,file2.name);

	if(stat(path1, &file_info1) == 1) {
		fprintf(stderr, "Error getting data about a file. (%s)\n", strerror(errno));
		return -1;
	}

	if(stat(path2, &file_info2) == 1){
		fprintf(stderr, "Error getting data about a file. (%s)\n",strerror(errno));
		return -1;
	}

	//


	return 0;
}

dup_file** check_duplicate_files(const char* filepath, file_path *files, int files_size, int *n_duplicates) {

	dup_file  **duplicates = (dup_file  **)malloc(0);
	*n_duplicates = 0;


	int i,j;	
	int new_dup = 1; //used as a boolean to check if this is the first duplicate a file has 
	int skip = 0;
	int curr_size;

	for(i = 0; i < files_size; i++) {
		new_dup = 1;
		curr_size = 0;
		skip = 0;


		for(j = 0; j < *n_duplicates; j++){ //check if a file is already in the duplicates array
			if (same_files(files[i], *duplicates[j][0].fp))
				skip = 1;
			break;
		}

		if(skip)
			continue;


		for(j=i+1;j < files_size; j++){
			if(same_files(files[i], files[j])){
				printf("butarde\n"); //FIXME remove
			if(new_dup){ 			//first duplicate of a file
				curr_size = 2;
				(*n_duplicates)++; //increments the size of the duplicates array
				duplicates = realloc(duplicates, *n_duplicates * sizeof(dup_file *));
				new_dup = 0; //resets boolean
				duplicates[i] = (dup_file *)malloc(curr_size * sizeof(dup_file));

				duplicates[i][0].fp = &files[i];
				duplicates[i][0].num_dups = 1;

				duplicates[i][1].fp = &files[j];
				duplicates[i][1].num_dups = 1;
			}

			else{ 
				curr_size ++;
				duplicates[i] = realloc(duplicates[i],curr_size * sizeof(dup_file *));
				duplicates[i][curr_size - 1].fp = &files[j];
				duplicates[i][curr_size - 1].num_dups = duplicates[i][0].num_dups;

				int k = 0;
				for(k = 0; k < curr_size; k++){ 	//increments the duplicate count on every duplicate file
					duplicates[i][k].num_dups++;
				}
			}
		}
		}
	}
	
	//TODO remove -- for debug purposes only
	for(i = 0; i < *n_duplicates; i++){
		for(j = 0; j<duplicates[i][0].num_dups; j++){
			printf("%s%s = ", duplicates[i][j].fp->path, duplicates[i][j].fp->name);
		}
		printf("\n\n");
	}
	return duplicates;
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

	int n_duplicates;
	check_duplicate_files(filepath, files, files_size, &n_duplicates); //TODO

	return 0;
}
