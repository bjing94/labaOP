#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int isDirectory(char* path){
	struct stat path_stat;
	stat(path, &path_stat);
	return S_ISREG(path_stat.st_mode) == 0;
}

char* archiveDirectory(char* path, int level){
	struct stat finfo; // file info
	struct dirent *de; // iterator through folder
	char* res = malloc(1000);
	//strcat(res,"FOLDER:\n");
	
	DIR *dr = opendir(path);
	
	if(dr == NULL){
		return "";
	}
	
	while((de = readdir(dr)) != NULL){
		if(strcmp(de->d_name,".")!=0 
		&& strcmp(de->d_name,"..")!=0 
		&& ((de->d_type == DT_REG) 
		|| (de->d_type == DT_DIR))){
			printf("%s\n", de->d_name);
			
			for(int i=0; i<level; i++){
				strcat(res,"-");
			}
			
			strcat(res,de->d_name);
			strcat(res,"\n");
			char* testPath = malloc(200);
			strcpy(testPath, path);
			strcat(testPath,"/");
			strcat(testPath, de->d_name);
			printf("%s file info: ", stat(testPath, &finfo));
			if(isDirectory(testPath)){
				char* tempRes = malloc(200);
				tempRes = archiveDirectory(testPath, level+1);
				strcat(res,tempRes);
			}
		}
		
	}
	
	closedir(dr);

	return res;
}
int main(int argc, char *argv[]){
	FILE *fp;
	
	printf("Program:%s\n", argv[0]);
	
	char* pathToFile = "";
	char* pathToArchive = "";
	for(int i=1; i < argc; i++){
		if(strcmp(argv[i], "-d") == 0){
			pathToFile = argv[i+1];
		}
		
		if(strcmp(argv[i], "-o") == 0){
			pathToArchive = argv[i+1];
		}
	}
	char* finalPath = strcat(pathToArchive, "lab1.arch");
	
	printf("Path to file:%s\n", pathToFile);
	printf("Path to archive:%s\n", finalPath);
	char* result = archiveDirectory(pathToFile, 1);
	fp = fopen(finalPath, "w+");
	fputs(result, fp);
	fclose(fp);
	printf("Result:\n %s", result);
	free(result);
	return 0;
}
