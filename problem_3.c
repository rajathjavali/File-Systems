/*
Author: Rajath P Javali
University: Utah
ID: u1140594
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>

#include "crc.h"

#define MAX_LENGTH 4096

struct dirTree
{
	struct dirent *file;
	struct dirTree *next;
	int checksum;

};

typedef struct dirTree dirList;
dirList *root, *sharedPtr;
int fileCount = 0;
char *name;
pthread_mutex_t lock;

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
	fileCount++;	
}

void *compute_checksum(void *arg) {

	__sync_synchronize();

	while (sharedPtr != NULL){
		int *i = (int *)arg;
		printf("%d\n", *i);
		
		dirList *temp;
		
		pthread_mutex_lock(&lock);
		temp = sharedPtr;
		sharedPtr = sharedPtr->next;
		pthread_mutex_unlock(&lock);


		char path[MAX_LENGTH];
		strcpy(path, name);
		strcat(path, temp->file->d_name);

		//printf("%s\n", path);

		FILE *subfile = fopen(path, "r");

		if(!subfile){
			printf("%s ACCESS ERROR\n", temp->file->d_name);
			return NULL;
		}

		// finding file size // referred stack overflow
		fseek(subfile, 0L, SEEK_END);
		int fileSize = ftell(subfile);
		rewind(subfile);

		char buf[fileSize];
		size_t readSize = fread(buf, 1, fileSize, subfile);

		uint32_t checksum = 0;
		checksum = crc32(checksum, buf, readSize);
		printf("%s %#.8X\n", temp->file->d_name, checksum);
		if(fclose(subfile))
			printf("Error closing file %s\n", temp->file->d_name);
	}

	return NULL;
}

int main (int argc, char* argv[])
{
	if ( argc < 2)
	{
		printf("Invalid execution call. \n./<executable> <directory path> <num of threads>;\n");
		return -1;
	}
	
	name = argv[1];
	int num_thread = atoi(argv[2]);
	if(num_thread < 1 || num_thread > 99)
	{
		printf("Num of threads exceeding the bounds (1-99)\n");
		return -1;
	}

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
	
	file = readdir(directory);
	while ((file = readdir(directory)) != NULL)
	{
		if(file->d_type == DT_DIR)
			continue;
		addNode(file);
	}

	/*
	// testing directory listings
	// prints the non sub directory files of the input Directory
	dirList *temp = root;
	while (temp!=NULL)
	{
		printf("%s\n", temp->file->d_name);
		temp = temp->next;
	}*/

	//printing checksum here
	
	sharedPtr = root;

	if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init has failed\n");
        return -1;
    }

	/* Multithreaded approach to computing checksum */
	if (fileCount < num_thread)
	{
		printf("Making use of just %d num of threads\n", fileCount);
	}

	int min = num_thread < fileCount ? num_thread : fileCount;
	pthread_t threads[min];
	
	for (int i = 0; i < min; i++) {
		//printf("%d\n", i);
		int tid = i;
		if(pthread_create(&threads[i], NULL, &compute_checksum, &tid))
		{
			printf("pthread_create failed !!!\n");
			return -1;
		}
	}

	for (int i = 0; i < min; i++) {
		pthread_join(threads[i], NULL);
	}


	int i = 0;
	while(closedir(directory) && i < 4) {
		i++;
		printf("Could not close directory!! Attempt %d\n", i);
	}
	return 0;

}