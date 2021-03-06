/*
Author: Rajath P Javali
University: Utah
ID: u1140594
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/mman.h>

#include "crc.h"

#define MAX_LENGTH 4096

struct dirTree
{
	struct dirent *file;
	struct dirTree *next;

};
char directoryList[256][4096];
typedef struct dirTree dirList;
dirList *root;

void addNode(struct dirent* newNode)
{
	dirList* temp = (dirList *)malloc(sizeof(dirList));
	temp->file = newNode;
	temp->next = NULL;

	if(root == NULL)
		root = temp;

	else if(strcmp((root)->file->d_name, temp->file->d_name) >= 0)
	{
		temp->next = root;
		root = temp;
	}
	else
	{
		dirList *ittr = root;
		while(ittr->next != NULL && (strcmp(ittr->next->file->d_name, temp->file->d_name) <= 0))
			ittr = ittr->next;

		temp->next = ittr->next;
		ittr->next = temp;
	}	
}

int main (int argc, char* argv[])
{
	char *name = argv[1];
	size_t nameLength = strlen(name);
	if(name[nameLength - 1] != '/')
		name[nameLength] = '/', name[nameLength+1] = '\0';
		/*//alternately can use this
		name = strcat(name, "/");
		*/

	/*// Checking directory path string
	printf("directory %s\n", name);
	*/

	DIR *directory;
	directory = opendir(name);
	if (!directory) {
		printf ("Error opening directory %s\n", name);
		return -1;
	}

	struct dirent *file;
	int index = 0;
	file = readdir(directory);
	while ((file = readdir(directory)) != NULL)
	{
		if(file->d_type != DT_DIR)
		{
			strcpy(directoryList[index], name);
			//strcpy(directoryList[index++], file->d_name);
			addNode(file);
		}
	}

	/*for(int i= 0; i< index;i++)
	{
		printf("%s\n", directoryList[i]);
	}*/

	/*
 	// testing directory listings
	// prints the non sub directory files of the input Directory

	while (temp!=NULL)
	{
		printf("%s\n", temp->file->d_name);
		temp = temp->next;
	}
	*/
	//printing checksum here
	//printf("%d\n", index);
	/*for(int i = 0 ; i < index; i++)
	{
		char path[MAX_LENGTH];
		strcpy(path, name);
		strcat(path, directoryList[i]);
		
		FILE *subfile = fopen(path, "r");
		if(!subfile){
			printf("%s ACCESS ERROR\n", directoryList[i]);
			continue;
		}
		printf("%s ", directoryList[i]);
		// finding file size // referred stack overflow
		fseek(subfile, 0L, SEEK_END);
		int fileSize = ftell(subfile);
		rewind(subfile);

		//printf("%d\n", fileSize);

		char *buf = (char *)mmap(0, fileSize, PROT_READ, MAP_SHARED, fileno(subfile), 0);

		//printf("%s\n", buf);
		//long size = fread(buf, 1, fileSize, subfile);

		uint32_t checksum = 0;
		checksum = crc32(checksum, buf, fileSize);
		printf("%#.8X\n", checksum);

		if(fclose(subfile))
			printf("Error closing file %s %s\n", directoryList[i], strerror (errno));
		
	}*/
	dirList *temp = root;
	while (temp != NULL)
	{
		char path[MAX_LENGTH];
		strcpy(path, name);
		strcat(path, temp->file->d_name);

		//printf("Path %s, ", path);
		FILE *subfile = fopen(path, "r");
		if(!subfile){
			printf("%s ACCESS ERROR\n", temp->file->d_name);
			temp = temp->next;
			continue;
		}
		// finding file size // referred stack overflow
		if(fseek(subfile, 0L, SEEK_END) == -1)
		{
			printf("fseek error on file %s\n", temp->file->d_name);
			if(fclose(subfile))
				printf("fclose error on file %s\n", temp->file->d_name);
			continue;
		}
		
		int fileSize = ftell(subfile);
		rewind(subfile);
		
		char *buf = (char *)mmap(0, fileSize, PROT_READ, MAP_SHARED, fileno(subfile), 0);
		
		uint32_t checksum = 0;
		printf("%s ", temp->file->d_name);
		checksum = crc32(checksum, buf, fileSize);
		printf("%#.8X\n", checksum);

		if(fclose(subfile))
			printf("Error closing file %s %s\n", temp->file->d_name, strerror (errno));
		temp = temp->next;
	}
	

	int i = 0;
	while(closedir(directory) && i < 4) {
		i++;
		printf("Could not close directory!! Attempt %d\n", i);
	}
	return 0;

}
