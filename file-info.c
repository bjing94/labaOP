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

long getFileSize(char *path)
{
	struct stat buffer;
	int status = stat(path, &buffer);

	if (status != 0)
	{
		perror("file size stat error: ");
		return 0;
	}
	else
	{
		return buffer.st_size;
	}
}

char *getFileContent(char *path, int desc)
{
	long size = getFileSize(path);
	char *buff = malloc(size);
	read(desc, buff, size);
	return buff;
}

char *getDateModified(char *path)
{
	char *res = malloc(200);
	struct stat *finfo = malloc(400); // file info
	stat(path, finfo);
	struct tm *localTime = localtime(&finfo->st_ctim.tv_sec);
	return asctime(localTime);
}

int getChmod(const char *path)
{
	struct stat ret;

	if (stat(path, &ret) == -1)
	{
		return -1;
	}

	return (ret.st_mode & S_IRUSR) | (ret.st_mode & S_IWUSR) | (ret.st_mode & S_IXUSR) | /*owner*/
		   (ret.st_mode & S_IRGRP) | (ret.st_mode & S_IWGRP) | (ret.st_mode & S_IXGRP) | /*group*/
		   (ret.st_mode & S_IROTH) | (ret.st_mode & S_IWOTH) | (ret.st_mode & S_IXOTH);	 /*other*/
}

char *getFilePermissions(char *path)
{
	struct stat *finfo = malloc(400); // file info
	int chmod = getChmod(path);
	char *res;
	sprintf(res, "%d", chmod);
	return res;
}

int isDirectory(char *path)
{
	struct stat path_stat;
	stat(path, &path_stat);
	return S_ISREG(path_stat.st_mode) == 0;
}

void copyInfo(int inDescriptor, long size, int outDescriptor)
{
	int sz = size;
	char buff[1];
	while (sz != 0)
	{
		read(inDescriptor, buff, 1);
		write(outDescriptor, buff, 1);
		sz -= 1;
	}
}

void createDirs(char *basePath, char *relativePath)
{
	char path[2048];
	char buff[256];
	buff[0] = '\0';
	strcpy(path, basePath);
	printf("path: %s\n", path);
	printf("relative: %s\n", relativePath);
	for (int i = 0; i < strlen(relativePath); i++)
	{
		if (relativePath[i] == '/')
		{
			strcat(path, buff);
			printf("Checking dir %s\n", path);
			DIR *dir = opendir(path);
			if (ENOENT == errno)
			{
				printf("Created dir: %s\n", path);
				mkdir(path, S_IRWXU);
			}
		}
		strncat(buff, &relativePath[i], 1);
	}
}
