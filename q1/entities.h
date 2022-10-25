#ifndef _ENTITY
#define _ENTITY
#include <headers.h>

typedef struct student
{
    INT waiting;       // 0 means not arrived , 1 means waiting to wash , 2 means machine assigned , 3 means washed and left
    INT arrival_time;  // when he arrives
    INT patience_time; // waiting time <= patience_time , else leave
    INT waiting_time;  // time for which a student waits for machine
    INT machine_index; // which machine is assigned to the student, -1 if not assigned
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} student;
typedef struct machine
{
    INT index;
    INT working; // 0 if not assigned , 1 if assigned
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} machine;

#endif