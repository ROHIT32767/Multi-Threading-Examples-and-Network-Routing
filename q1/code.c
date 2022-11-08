#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
typedef long long int INT;
time_t start_time;
/*********** entities****************/
typedef struct student
{
    INT index;
    INT arrival_time; // when he arrives
    INT patience_time;
    INT washing_time; // time for which a student washes in machine
    INT waited_for;
    INT washed;
} student;
/*************************************/
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
/************semaphores**************/
sem_t curr_sem;
/************************************/
INT num_students, num_machines;
INT student_count;
pthread_mutex_t cnt_lock;
INT seconds_wasted = 0;
INT students_left_without_washing = 0;
void *wash_machine(void *arg)
{
    time_t arrive = time(NULL);
    INT index_current = ((struct student *)arg)->index;
    printf("%ld: Student %lld arrives\n", arrive - start_time, index_current);
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
        printf("%lld: Student %lld leaves without washing\n", arrive + patience - start_time, index_current);
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
        printf("%ld: Student %lld starts washing\n", current_time - start_time, index_current);
        ((struct student *)arg)->washed = 1;
        sleep(wash_current);
        time(&current_time);
        printf("%ld: Student %lld leaves after washing\n", current_time - start_time, index_current);
        sem_post(&curr_sem);
    }
    return NULL;
}

int main(void)
{
    scanf("%lld %lld", &num_students, &num_machines);
    students_left_without_washing = num_students;
    student studenttrack[num_students];
    pthread_t studentthreads[num_students];
    pthread_mutex_init(&cnt_lock, NULL);
    sem_init(&curr_sem, 0, num_machines);
    for (INT i = 0; i < num_students; i++)
    {
        studenttrack[i].index = i + 1;
        INT arrive, wash_for, patience;
        scanf("%lld %lld %lld", &arrive, &wash_for, &patience);
        studenttrack[i].arrival_time = arrive;
        studenttrack[i].washing_time = wash_for;
        studenttrack[i].patience_time = patience;
        studenttrack[i].washed = -1;
    }
    qsort(studenttrack, num_students, sizeof(studenttrack[0]), compare);

    int previous_time = 0;
    time(&start_time);
    for (int i = 0; i < num_students; i++)
    {
        int arrive_time = studenttrack[i].arrival_time;
        sleep(arrive_time - previous_time);
        pthread_create(&studentthreads[i], NULL, wash_machine, &studenttrack[i]);
        previous_time = arrive_time;
    }
    for (INT i = 0; i < num_students; i++)
    {
        pthread_join(studentthreads[i], NULL);
    }
    printf("%lld\n", students_left_without_washing);
    printf("%lld\n", seconds_wasted);
    (4 * students_left_without_washing < num_students) ? printf("No\n") : printf("Yes\n");
    pthread_mutex_destroy(&cnt_lock);
    sem_destroy(&curr_sem);
    return 0;
}
