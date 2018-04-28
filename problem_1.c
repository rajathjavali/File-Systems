#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>

#define CAT 0
#define DOG 1
#define BIRD 2

typedef struct 
{
	int type;
	int count;
}threadData;

volatile int forceStop = 0;
volatile int cats = 0, dogs = 0, birds = 0, n_cats = 0, n_dogs = 0, n_birds = 0;

pthread_mutex_t mutex;
pthread_cond_t cond_cat, cond_bird_dog;// cond_dog, cond_bird;

void cat_enter(threadData *arg)
{
	pthread_mutex_lock(&mutex);
	while (birds != 0 || dogs != 0)
		pthread_cond_wait(&cond_cat, &mutex);
	cats++; 
	arg->count++;
	pthread_mutex_unlock(&mutex);
}

void cat_exit(threadData *arg)
{
	pthread_mutex_lock(&mutex);
	cats--;
	if (cats == 0)
	{
		pthread_cond_broadcast(&cond_bird_dog);
		//pthread_cond_broadcast(&cond_bird);
		//pthread_cond_broadcast(&cond_dog);
	}
	pthread_mutex_unlock(&mutex);
}

void dog_enter(threadData *arg)
{
	pthread_mutex_lock(&mutex);
	while (cats != 0)
		pthread_cond_wait(&cond_bird_dog, &mutex);
		//pthread_cond_wait(&cond_dog, &mutex);
	dogs++;
	arg->count++;
	pthread_mutex_unlock(&mutex);
}

void dog_exit(threadData *arg)
{
	pthread_mutex_lock(&mutex);
	dogs--;
	if(dogs == 0 && birds == 0)
		pthread_cond_broadcast(&cond_cat);
	pthread_mutex_unlock(&mutex);
}

void bird_enter(threadData *arg)
{
	pthread_mutex_lock(&mutex);
	while (cats != 0)
		pthread_cond_wait(&cond_bird_dog, &mutex);
	birds++;
	arg->count++;
	pthread_mutex_unlock(&mutex);
}

void bird_exit(threadData *arg)
{
	pthread_mutex_lock(&mutex);
	birds--;
	if (birds == 0 && dogs == 0)
		pthread_cond_broadcast(&cond_cat);
	
	pthread_mutex_unlock(&mutex);
}

void play(void) {
  for (int i=0; i<10; i++) {
    assert(cats >= 0 && cats <= n_cats);
    assert(dogs >= 0 && dogs <= n_dogs);
    assert(birds >= 0 && birds <= n_birds);
    assert(cats == 0 || dogs == 0);
    assert(cats == 0 || birds == 0);
   }
}

void *playArea(void *arg)
{
	__sync_synchronize();

	threadData *thread = (threadData *)arg;
	while(!forceStop)
	{
		if(thread->type == CAT)
		{
			cat_enter(thread);
			play();
			cat_exit(thread);
		}
		else if(thread->type == DOG)
		{
			dog_enter(thread);
			play();
			dog_exit(thread);
		}
		else if(thread->type == BIRD)
		{
			bird_enter(thread);
			play();
			bird_exit(thread);
		}
	}
	return NULL;
}
int main (int argc, char* argv[]) 
{
	if(argc < 4){
		printf("Arguments missing. Expected 3 parameters indicating:\n<executable> num_cats num_dogs num_birds\n");
		return -1;
	}
	
	n_cats = atoi(argv[1]);
	if (n_cats < 0 || n_cats > 99)
	{
		printf("Number of cats exceeded the boundary 0-99\n");
		return -1;
	}	
	
	n_dogs = atoi(argv[2]);
	if (n_dogs < 0 || n_dogs > 99)
	{
		printf("Number of dogs exceeded the boundary 0-99\n");
		return -1;
	}

	n_birds = atoi(argv[3]);
	if (n_birds < 0 || n_birds > 99)
	{
		printf("Number of birds exceeded the boundary 0-99\n");
		return -1;
	}

	//n_cats = 2, n_birds = 2, n_dogs = 2;
	
	if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        printf("\n mutex init has failed\n");
        return 1;
    }

    if (pthread_cond_init(&cond_cat, NULL) != 0)
    {
        printf("\n condition variable init has failed\n");
        return 1;
    }

    if (pthread_cond_init(&cond_bird_dog, NULL) != 0)
    {
        printf("\n condition variable init has failed\n");
        return 1;
    }

    /*if (pthread_cond_init(&cond_dog, NULL) != 0)
    {
        printf("\n condition variable init has failed\n");
        return 1;
    }

    if (pthread_cond_init(&cond_bird, NULL) != 0)
    {
        printf("\n condition variable init has failed\n");
        return 1;
    }
	*/
	int totalThreads = n_cats + n_birds + n_dogs;
	pthread_t threadPool[totalThreads];
	threadData thData[totalThreads];

	for (int i = 0; i < totalThreads; i++)
	{
		thData[i].count = 0;
		if (i < n_cats)
			thData[i].type = CAT;
		else if (i < n_cats + n_dogs)
			thData[i].type = DOG;
		else
			thData[i].type = BIRD;
		if(pthread_create(&threadPool[i], NULL, playArea, &thData[i]))
		{
			fprintf(stderr, "Error on pthread create\n");
		}
	}
	
	/*pthread_t catThreadPool[n_cats], dogThreadPool[n_dogs], birdThreadPool[n_birds];
	threadData catPoolData[n_cats], dogPoolData[n_dogs], birdPoolData[n_birds];

	for (int i = 0; i < n_cats; i++)
	{
		catPoolData[i].count = 0;
		if(pthread_create(&catThreadPool[i], NULL, playArea, catPoolData[i]))
		{
			fprintf(stderr, "Error on pthread create\n");
		}

	}
	
	for (int i = 0; i < n_dogs; i++)
	{
		dogPoolData[i].count = 0;
		if(pthread_create(&dogThreadPool[i], NULL, playArea, dogPoolData[i]))
		{
			fprintf(stderr, "Error on pthread create\n");
		}

	}

	for (int i = 0; i < n_birds; i++)
	{
		birdPoolData[i].count = 0;
		if(pthread_create(&birdThreadPool[i], NULL, playArea, birdPoolData[i]))
		{
			fprintf(stderr, "Error on pthread create\n");
		}

	}*/

	sleep(10);
	forceStop = 1;
	
	for(int i = 0; i < totalThreads; i++)
		pthread_join(threadPool[i], NULL);
	
	/*for (int i = 0; i < n_cats; i++)
		pthread_join(catThreadPool[i], NULL);

	for (int i = 0; i < n_dogs; i++)
		pthread_join(dogThreadPool[i], NULL);

	for (int i = 0; i < n_birds; i++)
		pthread_join(birdThreadPool[i], NULL);
	*/
	int totalCatPlayTime = 0, totalDogPlayTime = 0, totalBirdPlayTime = 0;
	
	for(int i = 0; i < totalThreads; i++)
	{
		if(i < n_cats)
			totalCatPlayTime += thData[i].count;
		else if(i < n_cats + n_dogs)
			totalDogPlayTime += thData[i].count;
		else
			totalBirdPlayTime += thData[i].count;
	}
	
	/*for (int i = 0; i < n_cats; i++)
		totalCatPlayTime += catPoolData[i].count;
	
	for (int i = 0; i < n_cats; i++)
		totalDogPlayTime += dogPoolData[i].count;
 
	for (int i = 0; i < n_cats; i++)
		totalBirdPlayTime += birdPoolData[i].count; */
	
	printf("cat play = %i, dog play = %i, bird play = %i\n", totalCatPlayTime, totalDogPlayTime, totalBirdPlayTime);

	return 0;
}