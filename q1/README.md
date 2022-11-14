# Assignment 5: Multi-Threading 
# Report for q1
# M22CS3.301: Operating Systems and Networks
# Question - 1: Washing Machines
# Gowlapalli Rohit
>##### All these commands are tested on Ubuntu Version 20.04.3 LTS (Focal Fossa) 
```
2021101113_assign5
├── q1
│   ├── q1.c
│   └── README
├── q2
│   ├── q2.c
│   └── README
└── q3
    ├── client.cpp
    ├── README
    └── server.cpp
```
---------------------------------------------------------------

* > Commands to be executed

     ``gcc q1.c -lpthread``

     ``./a.out``
### Threads are used in this problem to monitor the use of Washing machines in OBH ; to solve it , we classify the entities in the problem 
### An entity is an integral part of the problem and have an independent existence. There are single / multiple instances of each of these entities. 
### Here students are entities ; We have multiple instances of entities and we want them to use washing machines concurrently . Hence , we use threads for their execution.
### We use a common global semaphore for threads which share the same critical section 
### That way ,as soon as a thread posts a semaphore, any other thread in blocking can be signalled and can use up the variable.
### To initialise the global semaphore curr_sem we use sem_init(sem_t *sem, int pshared, unsigned int value) -  Here , sem_init() initializes the unnamed semaphore at the address pointed to by sem.  The value argument specifies the initial value for the semaphore representing the Total number of washing machines
```c
sem_t curr_sem;
sem_init(&curr_sem, 0, num_machines);
```
### We maintain a Data Structure (student) to track the status of each student 
```c
typedef struct student
{
    INT index; // index of the student in the data-input
    INT arrival_time; // the time at which the (index)th student comes after the execution of the program
    INT patience_time; // patience of (index)th student after which he leaves without getting his clothes washed
    INT washing_time; // time taken to wash the (index)th student clothes
    INT waited_for; // Seconds wasted by the (index)th student
    INT washed; // variable denoting the status of the clothes of the (index)th student 
} student;
```
### struct student[num_students] is declared to represent students and is sorted in order of their arrival times , if there exists a tie while comparing arrival times , we use index of the student as the tie-breaker using the following compare function ensuring FCFS Scheduling
```c
qsort(studenttrack, num_students, sizeof(studenttrack[0]), compare);
int compare(const void *arg1, const void *arg2)
{
    INT arrival1 = ((struct student *)arg1)->arrival_time;
    INT arrival2 = ((struct student *)arg2)->arrival_time;
    if ((arrival1 - arrival2) != 0)
    {
        return arrival1 - arrival2;
    }
    else
    {
        INT index1 = ((struct student *)arg1)->index;
        INT index2 = ((struct student *)arg2)->index;
        return index1 - index2;
    }
}
```
### Then, student_threads are created in the order of their indexes in the sorted array of struct student using pthread_create(pthread_t *restrict thread,const pthread_attr_t *restrict attr,void *(*wash_machine)(void *),void *restrict arg);pthread_create() function starts a new thread in the calling process.  The new thread starts execution by invoking wash_machine(); arg is passed as the sole argument of start_routine() and the gap between arrival time is simulated using sleep()
```c
int previous_time = 0;
    time(&start_time);
    for (int i = 0; i < num_students; i++)
    {
        int arrive_time = studenttrack[i].arrival_time;
        sleep(arrive_time - previous_time);
        pthread_create(&studentthreads[i], NULL, wash_machine, &studenttrack[i]);
        previous_time = arrive_time;
    }
    void *wash_machine(void *arg)
        {
            time_t arrive = time(NULL);
            INT index_current = ((struct student *)arg)->index;
            printf(BWHT);
            printf("%ld: Student %lld arrives\n", arrive - start_time, index_current);
            printf(reset);
            INT patience = ((struct student *)arg)->patience_time;
            INT wash_current = ((struct student *)arg)->washing_time;

            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += patience;
            int rc = sem_timedwait(&curr_sem, &ts);
            if (rc == -1)
            {
                // if (errno == ETIMEDOUT)
                time_t current_time;
                time(&current_time);
                printf(BRED);
                printf("%lld: Student %lld leaves without washing\n", arrive + patience - start_time, index_current);
                printf(reset);
                pthread_mutex_lock(&cnt_lock);
                seconds_wasted += current_time - arrive;
                pthread_mutex_unlock(&cnt_lock);
            }

            else
            {
                time_t current_time;
                time(&current_time);
                pthread_mutex_lock(&cnt_lock);
                seconds_wasted += current_time - arrive;
                students_left_without_washing--;
                pthread_mutex_unlock(&cnt_lock);
                printf(BGRN);
                printf("%ld: Student %lld starts washing\n", current_time - start_time, index_current);
                printf(reset);
                ((struct student *)arg)->washed = 1;
                sleep(wash_current);
                time(&current_time);
                printf(BYEL);
                printf("%ld: Student %lld leaves after washing\n", current_time - start_time, index_current);
                printf(reset);
                sem_post(&curr_sem);
            }
            return NULL;
        }

```
### sem_post() increments (unlocks) the semaphore pointed to by curr_sem.If the semaphore's value consequently becomes greater than zero,then another process or thread blocked in a sem_wait call will be woken up and proceed to lock the semaphore. 
### The sem_timedwait() function shall lock the semaphore referenced by curr_sem as in the sem_wait() function. However, if the semaphore cannot be locked without waiting for another process or thread to unlock the semaphore by performing a sem_post() function, this wait shall be terminated when the specified timeout expires.
### To attain mutual exclusion , we use a special lock (provided by pthread library) varibale cnt_lock to protect seconds_wasted and students_left_without_washing
### All threads accessing a critical section share a lock , one thread succeeds in locking -owner of lock ; Other threads that try to lock cannot proceed further until lock is released by the owner
### The mutex object referenced by mutex shall be locked by a call to pthread_mutex_lock() that returns zero or [EOWNERDEAD].  If the mutex is already locked by another thread, the calling thread shall block until the mutex becomes available. This operation shall return with the mutex object referenced by mutex in the locked state with the calling thread as its owner.
### The pthread_mutex_unlock() function shall release the mutex object referenced by mutex.  The manner in which a mutex is released is dependent upon the mutex's type attribute. If there are threads blocked on the mutex object referenced by mutex when pthread_mutex_unlock() is called, resulting in the mutex becoming available, the scheduling policy shall determine which thread shall acquire the mutex.
### After termination of each pthread ,we clean up the resources and merge it back with the parent thread using  pthread_join(pthread_t thread, void **retval); The pthread_join() function waits for the thread specified by thread to terminate.  If that thread has already terminated, then pthread_join() returns immediately.  The thread specified by thread must be joinable.
```c
for (INT i = 0; i < num_students; i++)
    {
        pthread_join(studentthreads[i], NULL);
    }
```
### Finally we destroy the lock variable using pthread_mutex_destroy(pthread_mutex_t *mutex);The pthread_mutex_destroy() function shall destroy the mutex object referenced by mutex; the mutex object becomes, in effect, uninitialized. An implementation may cause pthread_mutex_destroy() to set the object referenced by mutex to an invalid value.
```
 Example of Output
```
<span style="color: red"> Student 3 leaves without washing </span>
</br>
<span style="color: #DFFF00"> Student 1 leaves after washing </span>
</br>
<span style="color: green"> Student 2 starts washing </span>
</br>
<span style="color: white"> Student 1 arrives </span>
</br>
1
</br>
4
</br>
<span style="color: white"> No </span>
</br>
---------------------------------------------------------------