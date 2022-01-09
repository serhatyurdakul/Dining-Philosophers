#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <time.h>
#include <sched.h>

#define N 8
#define THINKING 2
#define HUNGRY 1
#define EATING 0
#define LEFT (phnum + (N - 1)) % N
#define RIGHT (phnum + 1) % N
#define PILAVGR 2000
void test(int phnum);
void take_fork(int phnum);
void put_fork(int phnum);
void *philospher(void *num);
void init();
void printInfo(int);

float total_cost;
int group_number;
int eatenCount;
int totalPilav;
int usedPilav = 0;
int refresh_count;
int state[N];
int isEaten[N];
int phil[N];

sem_t mutex;
sem_t mutex2;
sem_t S[N];

int main(int argc, char *argv[])
{

    group_number = 5;
    if (argc >= 2)
        /* gets number of groups from command line*/
        group_number = atoi(argv[1]);

    srand((unsigned int)time(NULL));
    /* if arguments is invalid */
    if (group_number < 2 || group_number > 1000)
        /* replace with 5 */
        group_number = 5;
    for (int j = group_number; j > 0; j--)
    {
        init();
        int i;
        pthread_t thread_id[N];
        // initialize thread priorities
        struct sched_param param;
        pthread_attr_t attr;
        if (pthread_attr_init(&attr) == -1)
        {
            perror("init");
            exit(1);
        }

        // initialize the semaphores
        sem_init(&mutex, 0, 1);
        sem_init(&mutex2, 0, 1);

        for (i = 0; i < N; i++)

            sem_init(&S[i], 0, 0);

        for (i = 0; i < N; i++)
        {

            // create philosopher processes

            if (pthread_attr_getschedparam(&attr, &param) == -1)
            {
                perror("get");
                exit(1);
            }

            param.sched_priority = (rand() % 5);
            printf("Thread %d has priority %d\n", i, param.sched_priority);

            if (pthread_attr_setschedparam(&attr, &param) == -1)
            {
                perror("set");
                exit(1);
            }
            pthread_create(&thread_id[i], &attr,
                           philospher, &phil[i]);
            //printf("Philosopher %d is thinking\n", i + 1);
        }
        printf("\n");
        for (i = 0; i < N; i++)
            pthread_join(thread_id[i], NULL);
        printInfo(j);
    }
}
void test(int phnum)
{
    if (state[phnum] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING)
    {
        // state that eating
        state[phnum] = EATING;
        usleep((rand() % 5) + 1);
        sem_wait(&mutex2);
        if (isEaten[phnum] == 0)
        {
            isEaten[phnum] = 1;
            eatenCount += 1;
        }
        else
        {
            isEaten[phnum] += 1;
        }
        if (totalPilav == 0)
        {
            refresh_count += 1;
            totalPilav = PILAVGR;
            total_cost += 19.90;
            usedPilav += PILAVGR;
            total_cost += ((PILAVGR / 1000.0) * 20 /*rice kg price*/);
        }
        else
        {
            totalPilav -= 100;
        }
        sem_post(&mutex2);
        //printf("Philosopher %d takes fork %d and %d\n",
        //             phnum + 1, LEFT + 1, phnum + 1);

        //printf("Philosopher %d is Eating\n", phnum + 1);

        // sem_post(&S[phnum]) has no effect
        // during takefork
        // used to wake up hungry philosophers
        // during putfork
        sem_post(&S[phnum]);
    }
}

// take up chopsticks
void take_fork(int phnum)
{

    sem_wait(&mutex);

    // state that hungry
    state[phnum] = HUNGRY;

    //printf("Philosopher %d is Hungry\n", phnum + 1);

    // eat if neighbours are not eating
    test(phnum);

    sem_post(&mutex);

    // if unable to eat wait to be signalled
    sem_wait(&S[phnum]);

    usleep((rand() % 5) + 1);
}

// put down chopsticks
void put_fork(int phnum)
{

    sem_wait(&mutex);

    // state that thinking
    state[phnum] = THINKING;

    //printf("Philosopher %d putting fork %d and %d down\n",
    //      phnum + 1, LEFT + 1, phnum + 1);
    //printf("Philosopher %d is thinking\n", phnum + 1);

    test(LEFT);
    test(RIGHT);

    sem_post(&mutex);
}

void *philospher(void *num)
{

    while (1)
    {

        if (eatenCount == N)
        {
            break;
        }

        int *i = num;

        usleep((rand() % 5) + 1);

        take_fork(*i);
        usleep((rand() % 5) + 1);

        put_fork(*i);
    }
    pthread_exit(NULL);
}
void init()
{
    eatenCount = 0;
    usedPilav = PILAVGR;
    totalPilav = PILAVGR;
    refresh_count = 0;
    total_cost = 99.90;
    total_cost += ((usedPilav / 1000.0) * 20);
    for (int i = 0; i < N; i++)
    {
        phil[i] = i;
        isEaten[i] = 0;
        state[i] = HUNGRY;
    }
}
void printInfo(int j)
{
    printf("Group %d is Finished\nReceipt :", j);
    printf("\nTable opening fee: %.2lf tl\nTable Refresh fee: %.2lf tl", 99.90, refresh_count * 19.90);
    printf("\nTable Refreshed  %d times ", refresh_count);
    printf("\nTotaly %d gram Rice used which cost : %.1f tl ", usedPilav,
           (usedPilav / 1000.0) * 20);
    printf("\nTotal Cost is : %.1f tl", total_cost);
    for (int i = 0; i < N; i++)
    {
        printf("\nphilosopher %d eaten %d times ", i, isEaten[i]);
    }
    printf("\n-------------\n");
}
