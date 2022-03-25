#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <libgen.h>
#include <errno.h>
#include <fcntl.h>
#include "file-info.c"

void dirToArchive(char *dirPath, char *archPath, int level, char *basePath)
{
	struct stat *finfo = malloc(800); // file info
	int outDescriptor = open(archPath, O_CREAT | O_RDWR | O_APPEND, S_IRWXU);
	if (outDescriptor == -1)
	{
		printf("File didn't open!\n");
	}
	// FILE *out = fopen(archPath, "a");
	//  fseek(out, 0, SEEK_SET);
	char buff[256];
	char path[32256];
	// fputs(basename(dirPath), out);
	if (isDirectory(dirPath))
	{
		struct dirent *de; // iterator through folder
		DIR *dr = opendir(dirPath);
		// fclose(out);

		while ((de = readdir(dr)) != NULL)
		{
			if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0 && ((de->d_type == DT_REG) || (de->d_type == DT_DIR)))
			{

				strcpy(path, dirPath);
				strcat(path, "/");
				strcat(path, de->d_name);
				printf("Inspecting file: %s\n", path);
				dirToArchive(path, archPath, level + 1, basePath);
			}
			// closedir(dr);
		}
	}
	else
	{
		int inDescriptor = open(dirPath, O_RDWR);
		long size = getFileSize(dirPath);
		char sizeStr[255];
		sprintf(sizeStr, "%li", size);
		char *fileName = basename(dirPath);
		printf("Dir name: %s\n", basename(dirname(dirPath)));
		printf("File name: %s\n", fileName);
		char *dateModified = getDateModified(dirPath);
		char *filePermissions = getFilePermissions(dirPath);
		// char *fileContent = getFileContent(dirPath, inDescriptor);

		printf("File size is %li bytes\n", size);
		printf("Date modified: %s\n", dateModified);
		printf("File permissions: %s(%ld)\n", filePermissions, strlen(filePermissions));
		for (int i = strlen(basePath); i < strlen(dirPath); i++)
		{
			write(outDescriptor, &dirPath[i], 1);
		}
		write(outDescriptor, "/", 1);
		write(outDescriptor, fileName, strlen(fileName));
		write(outDescriptor, ";", 1);
		write(outDescriptor, fileName, strlen(fileName));
		write(outDescriptor, ";", 1);
		write(outDescriptor, &sizeStr, strlen(sizeStr));
		write(outDescriptor, ";", 1);
		write(outDescriptor, dateModified, strlen(dateModified));
		write(outDescriptor, ";", 1);
		copyInfo(inDescriptor, size, outDescriptor);
		write(outDescriptor, ";", 1);
		write(outDescriptor, filePermissions, 3);
		write(outDescriptor, "!", 1);
		write(outDescriptor, "\n", 1);
		// close(outDescriptor);
		// fclose(out);
	}
	free(finfo);
}

void archiveToDir(char *dirPath, char *archPath)
{
	enum fileInfo
	{
		filePath,
		fileName,
		fileSize,
		fileDateModified,
		content,
		permissions,
		finished
	};
	enum fileInfo myInfo = filePath;

	struct stat *finfo = malloc(800); // file info
	char buff[256];
	char path[2048];
	int inDescriptor = open(archPath, O_RDWR);
	char currentPath[2048];
	char *fileContent = getFileContent(archPath, inDescriptor);
	int isReadingContent = 0;
	long contentSize = 0;
	mode_t newPermissions;
	int outDescriptor;
	char *outContent;
	for (int i = 0; i < strlen(fileContent); i++)
	{
		if ((fileContent[i] == ';' | fileContent[i] == '!') && isReadingContent == 0)
		{
			// printf("%i\n", myInfo);
			switch (myInfo)
			{
			case filePath:
				strcpy(path, dirPath);
				strcat(path, buff);
				printf("trying to create: %s\n", path);
				createDirs(dirPath, buff);
				strcpy(currentPath, path);
				break;
			case fileName:
				printf("File name: %s\n", buff);
				break;
			case fileSize:
				contentSize = atoi(buff);
				printf("File size: %li\n", contentSize);
				break;
			case fileDateModified:
				printf("Date modifed: %s\n", buff);
				isReadingContent = 1;
				break;
			case content:
				printf("Reading content: \n%s\n", buff);
				outContent = malloc(strlen(buff) + 1);
				strcpy(outContent, buff);
				break;
			case permissions:
				newPermissions = atoi(buff);
				printf("Reading permissions: %o\n", newPermissions);
				break;
			default:
				break;
			}
			buff[0] = '\0';
			if (myInfo < finished)
			{
				myInfo = myInfo + 1;
				printf("Current step: %d\n", myInfo);
			}
		}
		if (isReadingContent == 0)
		{
			if (fileContent[i] != '\n' && fileContent[i] != '!' && fileContent[i] != ';')
			{ // do not ignore \n in content of files
				strncat(buff, &fileContent[i], 1);
			}
			if (fileContent[i] == '!')
			{ // file read

				buff[0] = '\0';
			}
		}
		if (isReadingContent == 1 && fileContent[i] != ';')
		{

			strncat(buff, &fileContent[i], 1);
			contentSize -= 1;
			if (contentSize == 0)
			{
				isReadingContent = 0;
			}
		}

		if (myInfo == 6)
		{
			outDescriptor = open(currentPath, O_CREAT | O_WRONLY, newPermissions);
			perror("open status");
			printf("path: %s\n", currentPath);
			printf("content: \n%s\n", outContent);
			write(outDescriptor, outContent, strlen(outContent));
			perror("write status");
			close(outDescriptor);
			printf("\n=========\n");
			myInfo = filePath;
		}
	}
	int status = close(inDescriptor);
	if (status == -1)
	{
		perror("close error");
	}
}

int main(int argc, char *argv[])
{
	FILE *fp;

	printf("Program: %s\n", argv[0]);

	char *pathToFile = "";
	char *pathToArchive = "";
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-d") == 0)
		{
			pathToFile = argv[i + 1];
		}

		if (strcmp(argv[i], "-o") == 0)
		{
			pathToArchive = argv[i + 1];
		}
	}
	char *finalPath = strcat(pathToArchive, "lab1.txt");
	printf("Path to file: %s\n", pathToFile);
	printf("Path to archive: %s\n", finalPath);
	int len = 0;
	int *sz = &len;
	remove(finalPath);
	dirToArchive(pathToFile, finalPath, 0, pathToFile);
	archiveToDir("/home/bjing/output", finalPath);
	return 0;
}
