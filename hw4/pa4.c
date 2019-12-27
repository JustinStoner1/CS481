/****************************************
 * Justin Stoner                        *
 * Steele Desmond                       *
 * CS 481-003                           *
 * Fall 2018, 12/6/2018                 *
 * Programming Assignment #4            *
 * Waiting Area with Concurrent Threads *
 ****************************************/
//----------------------------------------------Compling Command
//gcc pa4.c -lm -lpthread
//----------------------------------------------Important

//Out queue implmentation was heavly based on this design
//https://www.geeksforgeeks.org/queue-set-2-linked-list-implementation/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "random437.h"
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

//CS Protection
sem_t mutex;
sem_t full;
sem_t empty;

pthread_mutex_t mutexC;
pthread_cond_t ready;

// Used to tell the line of how many people are to be added to it
// Also outputs to the statusFile
struct ArrivalModule
{
  FILE *statusFile;
  int totalArrived;
  int totalRejected;
};

// Used to store how many people have ridden the ride (left the line)
struct Car
{
  int riders;
};

// Used to store the waiting time of each individual in the line
struct Human
{
  int timeWaiting;
};

// A linked list (LL) node to store a queue entry 
struct QNode 
{ 
    struct Human *human; 
    struct QNode *next; 
}; 
  
// The queue, front stores the front node of LL and rear stores ths 
// last node of LL 
struct Queue 
{ 
    struct QNode *front, *rear; 
}; 

// Used to store information about the line. Contains a queue of
// "Human" objects in the line that hold their current time waited
struct Line
{
  //Worst line tracking stats
  int max;
  int timeOfMaxHour;
  int timeOfMaxMinute;
  //Current Line
  int currentCount; // # of people that is updated by ArrivalModule
  int length; // Previous timestep's length of the line that is updated in manageLine
  //AVG wait time
  int totalTimeWaited; // Total time of all Human's wait time in the line
  //Queue of humans
  struct Queue *lineQueue;
};

//Constants
struct timespec interval; // Time interval for how often people enter the line
struct timespec loadInterval; // Time interval for how often people are loaded into cars
static struct ArrivalModule arrivalModule;
static struct Line line;
static struct Car car;

static const int TIMESTEPS = 599; //599 means 600 minutes or iterations
static const int MAXWAITPEOPLE = 800; // Line cap
static int CARNUM = 0; // # of cars, given at runtime
static int MAXPERCAR = 0; // # of people allowed in each car

//Time variables
static int iteration = -1; // 0 to 599 iterations
static int hour = 9; // Starting hour is 9am
static int minute = 0; // Used to output timestamps each iteration
static int second = 0; // Used to output timestamps each iteration

//----------------------------------------------Queue Stuff
  
//makes a new spot for a human to stand in
struct QNode* newNode(struct Human *human) 
{ 
    struct QNode *temp = (struct QNode*)malloc(sizeof(struct QNode)); 
    temp->human = human; 
    temp->next = NULL; 
    return temp;  
} 
  
//create the line
struct Queue *createQueue() 
{ 
    struct Queue *q = (struct Queue*)malloc(sizeof(struct Queue)); 
    q->front = q->rear = NULL; 
    return q; 
} 
  
//adds a human to the back of the line 
void push(struct Queue *q, struct Human *h) 
{ 
    // Create a new LL node 
    struct QNode *temp = newNode(h); 
    if (q->rear == NULL) 
    { 
       q->front = q->rear = temp; 
       return; 
    } 
    q->rear->next = temp; 
    q->rear = temp; 
} 
  
//pops a human from the front of the line
struct Human *pop(struct Queue *q) 
{ 
    if (q->front == NULL) 
       return NULL; 
    struct Human *temp = q->front->human; 
    q->front = q->front->next;  
    if (q->front == NULL) 
       q->rear = NULL; 
    return temp; 
} 

void waitingTimeUpdater(struct QNode *node)
{
    if (node != NULL)
	{
		node->human->timeWaiting = node->human->timeWaiting + 1;
    	waitingTimeUpdater(node->next);
	}
}

void updateWaitingTimes(struct Queue *q)
{
	waitingTimeUpdater(q->front);
}

int sumWaitTime(int x, struct QNode *node)
{
    if (x > 0)
	{
    	if (node != NULL)
		{
    		return sumWaitTime(x-1,node->next) + node->human->timeWaiting;
		}
	}
	else
    {
		return 0;
	}
}

int sumXWaitingTimes(int x, struct Queue *q)
{
  sumWaitTime(x,q->front);
}

//----------------------------------------------Update Time
//Helper function to timeStep --> Converts digital time to real time
void updateTime(int timeStep)
{
  int hours = timeStep / 60;// + (timeStep % 60)/60;
  int minutes = (timeStep % 60);
  //int seconds = (timeStep%100)%60;
  hour = 9+hours;
  minute = minutes;
  //second = seconds;
}

//----------------------------------------------Time Step
//Helper function to getArrivals (arrivalModule thread)
//Updates the current time
void timeStep()
{
  iteration++;
  updateTime(iteration);
}

//----------------------------------------------Get Arrivals
// ArrivalModule thread started in main
// Controls how many people on average arrive in the line
// Rejects people from entering the line if the line is full
// Updates final stats and writes output status to status file
void* getArrivals()
{
  // Add arrivals to the line for the given number of TIMESTEPS
  while(iteration < TIMESTEPS)
  {
    // Wait until the line is done managing itself
    sem_wait(&empty);
    sem_wait(&mutex);
    // Step forward in time
    timeStep();
    //printf("A: %d\n",iteration); // Testing

    // Check how many people need to arrive
    int mean = 0;
    if (hour < 11)
    {
      mean = 25;
    }
    else if (hour >= 11 && hour < 14)
    {
      mean = 45;
    }
    else if (hour >= 14 && hour < 16)
    {
      mean = 35;
    }
    else
    {
      mean = 25;
    }
    // Update the total arrivals stat
    int arrivals = poissonRandom(mean);
    arrivalModule.totalArrived += arrivals;
    
    // Reject people if the line is full, update stat
    int rejected = 0;
    if (line.currentCount + arrivals > MAXWAITPEOPLE)
    {
      rejected = line.currentCount + arrivals - MAXWAITPEOPLE;
      arrivalModule.totalRejected += rejected;
      line.currentCount += arrivals - rejected;
    }
    else
    {
      line.currentCount += arrivals;
    }

    //Write status line to file
    fprintf(arrivalModule.statusFile, "%d arrive %d reject %d wait-line %d at %d:%d:%d\n",iteration,arrivals,rejected,line.currentCount,hour,minute,second);

    // Unlock and allow manageLine to do its thing, sleep for 1 virtual minute
    sem_post(&mutex);
    sem_post(&full);
    nanosleep(&interval,NULL);
  }
  //printf("ending arrivals\n"); // Testing
  fclose(arrivalModule.statusFile); // If the line is closed close the file
}

// Helper function used by the manageLine thread
// Used to remove people from the line (Add them to cars)
// Removes 1 car worth of people from the line
void loadPeople()
{
  if (line.length > MAXPERCAR)
  {
    line.length -= MAXPERCAR;
    line.currentCount -= MAXPERCAR;
    car.riders += MAXPERCAR;
  }
  else
  {
    car.riders += line.length;
    line.length = 0;
    line.currentCount = 0;
  }
}

//----------------------------------------------Manage Line
// Line thread that is started in main
// When the line struct is finished being updated by ArrivalModule, it is unlocked
// Creates humans and adds the new arrivals (updated by the arrivalModule) to the queue
void* manageLine()
{
  int newArrivals;
  // Add arrivals to the line for the given number of TIMESTEPS
  while(iteration < TIMESTEPS)
  {
    //Wait for the arrivalModule to do its thing
    sem_wait(&full);
    sem_wait(&mutex);
    //printf(" L: %d\n",iteration); // Testing

    // See how many people were added to the line
    // Set length's new length 
    if (line.currentCount > line.length)
    {
      newArrivals = line.currentCount - line.length;
      line.length = line.currentCount;
    }

    // If the line's size is a new high, update the max size stats
    if (line.length > line.max)
    {
      line.max = line.length;
      line.timeOfMaxHour = hour;
      line.timeOfMaxMinute = minute;
    }

    // Remove Riders from the line/queue and add their time waited to the total time waited
    line.totalTimeWaited = line.totalTimeWaited +sumXWaitingTimes(CARNUM*MAXPERCAR,line.lineQueue);
    for (int i = 0; i < CARNUM*MAXPERCAR; i++)
    {
      pop(line.lineQueue);
    }

    // Load Riders (Remove people from the line)
    for (int i = 0; i < CARNUM; i++)
    {
      loadPeople();
    }

    // Update the waiting time of all humans by 1 minute
    updateWaitingTimes(line.lineQueue);

    // Add new humans
    for (int i = 0; i < newArrivals; i++)
    {
      //printf("adding person\n");
	  struct Human *t1 = (struct Human*)malloc(sizeof(struct Human));
      push(line.lineQueue, t1);
    }

    // Tell cars to leave
    pthread_cond_broadcast(&ready); // Unlocks loadCar threads
    // Unlock arrivalModule thread
    sem_post(&mutex);
    sem_post(&empty);
  }
  //printf("ending line\n");
}

//----------------------------------------------Load Car
/* Lock and unlock the car threads to simulate cars leaving with humans*/
void* loadCar()
{
  while(iteration < TIMESTEPS)
  {
    pthread_mutex_lock(&mutexC);
    pthread_cond_wait(&ready,&mutexC);
    //printf("  C: %d\n",iteration); // Testing
    //Wait 3 seconds to load car then leave
    pthread_mutex_unlock(&mutexC);
    nanosleep(&loadInterval,NULL);
  }
//printf("ending car\n");
}

//----------------------------------------------Main
/* Main initializes the semaphores and threads, and joins the threads together. 
 * The threads call functions getArrivals, manageLine, and loadCar */
int main(int argc, char **argv)
{
  /*Setup*/
  /*Semaphores used by the ArrivalModule and the Line (producer/consumer relationship)*/
  sem_init(&mutex,0,1);
  sem_init(&empty,0,1); //1 is empty
  sem_init(&full,0,0); //1 is full

  pthread_mutex_init(&mutexC,NULL); // Used to make sure cars don't load at the same time
  pthread_cond_init(&ready,NULL); // Used by the line to tell cars they can load

  // Iterate every 10 milliseconds. 1 virtual second = 10 milliseconds
  int milliseconds = 10;
  interval.tv_sec = 0;
  interval.tv_nsec = milliseconds * 1000000;

  // Time set to load people into cars
  milliseconds = 1;
  loadInterval.tv_sec = 0;
  loadInterval.tv_nsec = milliseconds * 100000;

  // File to write status updates to every minute
  arrivalModule.statusFile = fopen("statusFile", "w");

  // Initialize max line size (longest the line has been)
  line.max = 0;
  //struct Queue *q = createQueue();
  line.lineQueue = createQueue();
  
  //Get command line inputs for CARNUM and MAXPERCAR
  for (int i = 1; i < argc; i += 2)
  {
    if (strcmp(argv[i],"-N") == 0)
    {
      CARNUM = atoi(argv[i+1]);;
    }
    else if (strcmp(argv[i],"-M") == 0)
    {
      MAXPERCAR = atoi(argv[i+1]);
    }
  }
  printf("CARNUM = %d, MAXPERCAR = %d\n",CARNUM,MAXPERCAR); // Testing

  //Make arrival module thread which calls getArrivals
  void* voidptr = NULL;
  pthread_t arrivalThread;
  if (pthread_create(&arrivalThread, NULL, getArrivals, NULL))
  {
    perror("Error in thread creating\n");
  }

  //Make line thread which calls manageLine
  pthread_t lineThread;
  if (pthread_create(&lineThread, NULL, manageLine, NULL))
  {
    perror("Error in thread creating\n");
  }

  //Make car threads which each call loadCar
  int i;
  pthread_t *cars = malloc(CARNUM * sizeof(pthread_t));
  for (int i = 0; i < CARNUM; i++)
  {
    if (pthread_create(&cars[i],NULL,loadCar, NULL))
    {
      perror("Error in thread creating\n");
    }
  }

  //Join arrivalModule thread, line thread, and car thread(s)
  //Arrivals
  if (pthread_join(arrivalThread, (void*)&voidptr)) 
  {
    perror("Error in thread joining\n");
  }
  //Line
  if (pthread_join(lineThread, (void*)&voidptr)) 
  {
    perror("Error in thread joining\n");
  }

  //Cars
  for (int i = 0; i < CARNUM; i++)
  {
    //Make threads
    if (pthread_join(cars[i], (void*)&voidptr))
    {
      perror("Error in thread joining\n");
    }
  }

  /*Destroy semaphores and the conditional variable used by the cars*/
  sem_destroy(&mutex);
  sem_destroy(&full);
  sem_destroy(&empty);
  pthread_cond_destroy(&ready);

  /*Output final stats*/
  printf("Arrived: %d, Riders: %d, Rejected: %d, AVG waiting time: %d, Longest Line: %d at %d:%d\n"
         ,arrivalModule.totalArrived
         ,car.riders
         ,arrivalModule.totalRejected
         ,(line.totalTimeWaited/car.riders)
         ,line.max
         ,line.timeOfMaxHour
         ,line.timeOfMaxMinute);
  return 0;
}

