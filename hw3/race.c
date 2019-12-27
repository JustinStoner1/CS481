
/*=========================================================*/
/* race.c --- for playing with ECE437 */
/*=========================================================*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define SEMAPHORE "/transact"
#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

//struct {int balance[2];} Bank={{100,100}}; //global variable defined

struct BANK
{
  int balance[2];
};


void* MakeTransactions(struct BANK *Bank)
{ //routine for thread execution
  int i, j, tmp1, tmp2, rint; double dummy;
  for (i=0; i < 100; i++)
  {
    rint = (rand()%30)-15;
    if (((tmp1=Bank->balance[0])+rint)>=0 && ((tmp2=Bank->balance[1])-rint)>=0)
    {
      Bank->balance[0] = tmp1 + rint;
      for (j=0; j < rint*1000; j++) {dummy=2.345*8.765/1.234;} // spend time on purpose
      Bank->balance[1] = tmp2 - rint;
    }
  }
  return NULL;
}

int main(int argc, char **argv)
{
	//printf("%lu\n",(sizeof(struct BANK) + sizeof(sem_t)));

  int i;
  void* voidptr = NULL;
  srand(getpid());

  int shmid;
  char *shm;
	key_t key = ftok("shmfile",65);
//(sizeof(struct BANK) + sizeof(sem_t))
  if ((shmid = shmget(key, sizeof(struct BANK), IPC_CREAT | 0666)) < 0)
  {
    perror("shmget");
    exit(1);
  }

  struct BANK *Bank = shmat(shmid, NULL, 0);
  Bank->balance[0] = 100;
  Bank->balance[1] = 100;
  printf("\nInit balances A:%d + B:%d ==> %d!\n",Bank->balance[0],Bank->balance[1],Bank->balance[0]+Bank->balance[1]);


   sem_t *semaphore = sem_open(SEMAPHORE, O_CREAT | O_EXCL, (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP), 1);



  static pid_t pid;
  if ((pid = fork()) == 0)//make child
	{
		sem_t *semaphore = sem_open(SEMAPHORE, O_RDWR);
		//printf("child has been created\n");
		//printf("c%d, %d\n",Bank->balance[0],Bank->balance[1]);
		sem_wait(semaphore);
		printf("child is transacting\n");
    	MakeTransactions(Bank);
		sem_post(semaphore);
	}
	else if (pid < 0)//catch fork errors
	{
		printf("fork error");
    exit(1);
	}
	else
	{
		sem_t *semaphore = sem_open(SEMAPHORE, O_RDWR);
		sem_wait(semaphore);
		printf("Parent is transacting\n");
    	MakeTransactions(Bank);
		sem_post(semaphore);
    	wait(NULL);
		printf("Let's check the balances A:%d + B:%d ==> %d ?= 200\n\n",Bank->balance[0],Bank->balance[1],Bank->balance[0]+Bank->balance[1]);
	}
  
  return 0;
}

//Q3 Code
/*
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#define SHMSZ 27

//struct {int balance[2];} Bank={{100,100}}; //global variable defined

struct BANK
{
  int balance[2];
};


void* MakeTransactions(struct BANK *Bank)
{ //routine for thread execution
  int i, j, tmp1, tmp2, rint; double dummy;
  for (i=0; i < 100; i++)
  {
    rint = (rand()%30)-15;
    if (((tmp1=Bank->balance[0])+rint)>=0 && ((tmp2=Bank->balance[1])-rint)>=0)
    {
      Bank->balance[0] = tmp1 + rint;
      for (j=0; j < rint*1000; j++) {dummy=2.345*8.765/1.234;} // spend time on purpose
      Bank->balance[1] = tmp2 - rint;
    }
  }
  return NULL;
}

int main(int argc, char **argv)
{  
  int i;
  void* voidptr = NULL;
  srand(getpid());

  int shmid;
  char *shm;
  key_t key = ftok("shmfile",65);
  if ((shmid = shmget(key, sizeof(struct BANK), IPC_CREAT | 0666)) < 0)
  {
    perror("shmget");
    exit(1);
  }
  struct BANK *Bank = shmat(shmid, NULL, 0);
  Bank->balance[0] = 100;
  Bank->balance[1] = 100;
  printf("\nInit balances A:%d + B:%d ==> %d!\n",Bank->balance[0],Bank->balance[1],Bank->balance[0]+Bank->balance[1]);
  
  static pid_t pid;
  if ((pid = fork()) == 0)//make child
	{
		//printf("child has been created\n");
		//printf("c%d, %d\n",Bank->balance[0],Bank->balance[1]);
    	MakeTransactions(Bank);
	}
	else if (pid < 0)//catch fork errors
	{
		printf("fork error");
    exit(1);
	}
	else
	{
    	MakeTransactions(Bank);
    	wait(NULL);
		printf("Let's check the balances A:%d + B:%d ==> %d ?= 200\n\n",Bank->balance[0],Bank->balance[1],Bank->balance[0]+Bank->balance[1]);
	}
  
  return 0;
}
*/

//Q2 Code
/*
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct {int balance[2];} Bank={{100,100}}; //global variable defined
pthread_mutex_t mutex_lock;

void* MakeTransactions()
{ //routine for thread execution
  int i, j, tmp1, tmp2, rint; double dummy;
  pthread_mutex_lock(&mutex_lock);
  for (i=0; i < 100; i++)
  {
    rint = (rand()%30)-15;
    if (((tmp1=Bank.balance[0])+rint)>=0 && ((tmp2=Bank.balance[1])-rint)>=0)
    {
      Bank.balance[0] = tmp1 + rint;
      for (j=0; j < rint*1000; j++) {dummy=2.345*8.765/1.234;} // spend time on purpose
      Bank.balance[1] = tmp2 - rint;
    }
  }
  pthread_mutex_unlock(&mutex_lock);
  return NULL;
}

int main(int argc, char **argv)
{
  int i;
  void* voidptr = NULL;
  
  pthread_mutex_init(&mutex_lock, NULL);
  
  pthread_t tid[2];
  srand(getpid());
  printf("\nInit balances A:%d + B:%d ==> %d!\n",Bank.balance[0],Bank.balance[1],Bank.balance[0]+Bank.balance[1]);
  for (i=0; i<2; i++) if (pthread_create(&tid[i],NULL,MakeTransactions, NULL))
  {
    perror("Error in thread creating\n"); return(1);
  }
  for (i=0; i<2; i++) if (pthread_join(tid[i], (void*)&voidptr)) 
  {
    perror("Error in thread joining\n"); return(1);
  }
  printf("Let's check the balances A:%d + B:%d ==> %d ?= 200\n\n",Bank.balance[0],Bank.balance[1],Bank.balance[0]+Bank.balance[1]);
  return 0;
}
*/

//Q1 Code
/*
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct {int balance[2];} Bank={{100,100}}; //global variable defined

void* MakeTransactions()
{ //routine for thread execution
  int i, j, tmp1, tmp2, rint; double dummy;
  for (i=0; i < 100; i++)
  {
    rint = (rand()%30)-15;
    if (((tmp1=Bank.balance[0])+rint)>=0 && ((tmp2=Bank.balance[1])-rint)>=0)
    {
      Bank.balance[0] = tmp1 + rint;
      for (j=0; j < rint*1000; j++) {dummy=2.345*8.765/1.234;} // spend time on purpose
      Bank.balance[1] = tmp2 - rint;
    }
  }
  return NULL;
}

int main(int argc, char **argv)
{
  int i;
  void* voidptr = NULL;
  pthread_t tid[2];
  srand(getpid());
  printf("\nInit balances A:%d + B:%d ==> %d!\n",Bank.balance[0],Bank.balance[1],Bank.balance[0]+Bank.balance[1]);
  for (i=0; i<2; i++) if (pthread_create(&tid[i],NULL,MakeTransactions, NULL))
  {
    perror("Error in thread creating\n"); return(1);
  }
  for (i=0; i<2; i++) if (pthread_join(tid[i], (void*)&voidptr)) 
  {
    perror("Error in thread joining\n"); return(1);
  }
  printf("Let's check the balances A:%d + B:%d ==> %d ?= 200\n\n",Bank.balance[0],Bank.balance[1],Bank.balance[0]+Bank.balance[1]);
  return 0;
}
*/
