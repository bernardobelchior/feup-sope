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

	else if(time_dif == 0) //same modify date (should never happen), does not matter even if it happens...
		return 0;

	else if(time_dif < 0) //file 1 is older
		return -1;

	return 0;
} 


int same_files(const file_path file1,const file_path file2) {
	//TODO check permissions and content	

	struct stat file_info1, file_info2;
	char path1[255],path2[255];

	if(strcmp(file1.name,file2.name) != 0)
		return 0;

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

	//checking permissions
	if(file_info1.st_mode != file_info2.st_mode)
		return 0;

	//checking content
	pid_t pid;

	pid = fork();

	if(pid < 0){
		fprintf(stderr, "rmdup:same_files:fork() error\n");
		return -1;
	}	

	else if(pid == 0){ //child
		if(execlp("diff","diff",path1,path2,NULL) == -1){
			fprintf(stderr,"rmdup:same_files:execlp failed\n");
			return -1;
		}
	}

	else { //father
		int status;
		waitpid(pid,&status,0);
		if(status != 0) //files are equal, diff returned 0
			return 0;
	}


	return 1;
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
			if (same_files(files[i], *duplicates[j][0].fp)){
				skip = 1;
				break;
			}
		}

		if(skip)
			continue;


		for(j=i+1;j < files_size; j++){
			
			if(same_files(files[i], files[j])){

				if(new_dup){ 			//first duplicate of a file
					curr_size = 2;
					(*n_duplicates)++; //increments the size of the duplicates array
					duplicates = realloc(duplicates, *n_duplicates * sizeof(dup_file *));
					new_dup = 0; //resets boolean

					duplicates[*n_duplicates - 1] = (dup_file *)malloc(curr_size * sizeof(dup_file));

					duplicates[*n_duplicates - 1][0].fp = &files[i];
					duplicates[*n_duplicates - 1][0].num_dups = 2;

					duplicates[*n_duplicates - 1][1].fp = &files[j];
					duplicates[*n_duplicates - 1][1].num_dups = 2;
				}

				else{ 
					curr_size++;
					duplicates[*n_duplicates - 1] = realloc(duplicates[*n_duplicates - 1],curr_size * sizeof(dup_file));
					duplicates[*n_duplicates - 1][curr_size - 1].fp = &files[j];
					duplicates[*n_duplicates - 1][curr_size - 1].num_dups = duplicates[*n_duplicates - 1][0].num_dups;
					
					int k = 0;
					for(k = 0; k < curr_size; k++){ 	//increments the duplicate count on every duplicate file
						duplicates[*n_duplicates - 1][k].num_dups++;
					}

				}
			}
		}
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

	while( fscanf(file, "%s\n%s",path_buffer, name_buffer) != EOF){
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
void create_links(char* links_file_path, dup_file** duplicates, int n_duplicates) {
	FILE* links_file = fopen(links_file_path, "w");

	int i;
	for(i = 0; i < n_duplicates; i++) {
		int j;
		char src_full_path[strlen(duplicates[i][0].fp->path) + strlen(duplicates[i][0].fp->name) + 1];
			strcpy(src_full_path, duplicates[i][0].fp->path);
			strcat(src_full_path, duplicates[i][0].fp->name);
		for(j = 1; j < duplicates[i]->num_dups; j++) {
			//Creates links
			char dest_full_path[strlen(duplicates[i][j].fp->path) + strlen(duplicates[i][j].fp->name) + 1];
			strcpy(dest_full_path, duplicates[i][j].fp->path);
			strcat(dest_full_path, duplicates[i][j].fp->name);

			struct stat file_info;
			ino_t orig_inode, new_inode;

			if(stat(dest_full_path,&file_info) == 1){
				fprintf(stderr, "Error getting data about a file. (%s)\n", strerror(errno));
				return;
			}

			orig_inode = file_info.st_ino;

			if(unlink(dest_full_path) == -1) {
				fprintf(stderr, "Error unlinking file at %s. (%s)\n", dest_full_path, strerror(errno));
			} else {
				if(link(src_full_path, dest_full_path) == -1) {
					fprintf(stderr, "Error linking file at %s to %s. (%s)\n", dest_full_path, src_full_path, strerror(errno));
				} else {

					if(stat(dest_full_path,&file_info) == 1){
						fprintf(stderr,"Error getting data about a file. (%s)\n",strerror(errno));
					}

					new_inode = file_info.st_ino;

					fprintf(links_file, "%s linked to %s :: old inode = %li ,  new inode = %li\n", dest_full_path, src_full_path,orig_inode,new_inode);
					fprintf(stdout, "%s linked to %s\n", dest_full_path, src_full_path);
				}
			}
		}
	}
} 


int main(int argc, char* argv[]) {

	if(argc != 2) {
		fprintf(stderr, "Invalid number of arguments. \nProgram must be called as:\n./rmdup <directory>\nWhere <directory> is a valid directory that ends in \'/\'.\n");
		return 1;
	}

	char* file_input_name = "files.txt";
	char* link_file_name = "hlinks.txt";
	char filepath[strlen(argv[1]) + strlen(file_input_name) + 1];
	strcpy(filepath, argv[1]);
	strcat(filepath, file_input_name);
	char linkspath[strlen(argv[1]) + strlen(link_file_name) + 1];
	strcpy(linkspath, argv[1]);
	strcat(linkspath, link_file_name);
	
	int pid = fork();
	int files_size = 0;

	if(pid < 0){ //error
		fprintf(stderr,"main: fork() failed!\n");
	}

	if(pid == 0) { //child uses lsdir to list all the files in the directory and in its subdirectories and stores the list in a text file
		execlp("./lsdir", "lsdir", argv[1], NULL);
		exit(0);
	} 

	else {  //father waits for lsdir to finish
		waitpid(pid, NULL, 0);
	}

	file_path* files = read_from_file(filepath, &files_size);
	qsort(files, files_size, sizeof(file_path), comp_func);

	int n_duplicates;
	dup_file** duplicates =	check_duplicate_files(filepath, files, files_size, &n_duplicates);

	create_links(linkspath, duplicates, n_duplicates);
	
	return 0;
}
