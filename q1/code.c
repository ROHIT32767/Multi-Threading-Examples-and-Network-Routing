#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
typedef long long int INT;
time_t start_time;
/**********color codes***************/
//Regular text
#define BLK "\e[0;30m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"
#define BLU "\e[0;34m"
#define MAG "\e[0;35m"
#define CYN "\e[0;36m"
#define WHT "\e[0;37m"

//Regular bold text
#define BBLK "\e[1;30m"
#define BRED "\e[1;31m"
#define BGRN "\e[1;32m"
#define BYEL "\e[1;33m"
#define BBLU "\e[1;34m"
#define BMAG "\e[1;35m"
#define BCYN "\e[1;36m"
#define BWHT "\e[1;37m"

//Regular underline text
#define UBLK "\e[4;30m"
#define URED "\e[4;31m"
#define UGRN "\e[4;32m"
#define UYEL "\e[4;33m"
#define UBLU "\e[4;34m"
#define UMAG "\e[4;35m"
#define UCYN "\e[4;36m"
#define UWHT "\e[4;37m"

//Regular background
#define BLKB "\e[40m"
#define REDB "\e[41m"
#define GRNB "\e[42m"
#define YELB "\e[43m"
#define BLUB "\e[44m"
#define MAGB "\e[45m"
#define CYNB "\e[46m"
#define WHTB "\e[47m"

//High intensty background 
#define BLKHB "\e[0;100m"
#define REDHB "\e[0;101m"
#define GRNHB "\e[0;102m"
#define YELHB "\e[0;103m"
#define BLUHB "\e[0;104m"
#define MAGHB "\e[0;105m"
#define CYNHB "\e[0;106m"
#define WHTHB "\e[0;107m"

//High intensty text
#define HBLK "\e[0;90m"
#define HRED "\e[0;91m"
#define HGRN "\e[0;92m"
#define HYEL "\e[0;93m"
#define HBLU "\e[0;94m"
#define HMAG "\e[0;95m"
#define HCYN "\e[0;96m"
#define HWHT "\e[0;97m"

//Bold high intensity text
#define BHBLK "\e[1;90m"
#define BHRED "\e[1;91m"
#define BHGRN "\e[1;92m"
#define BHYEL "\e[1;93m"
#define BHBLU "\e[1;94m"
#define BHMAG "\e[1;95m"
#define BHCYN "\e[1;96m"
#define BHWHT "\e[1;97m"

//Reset
#define reset "\e[0m"
#define CRESET "\e[0m"
#define COLOR_RESET "\e[0m"
/************************************/
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
    printf(BWHT);
    (4 * students_left_without_washing < num_students) ? printf("No\n") : printf("Yes\n");
    printf(reset);
    pthread_mutex_destroy(&cnt_lock);
    sem_destroy(&curr_sem);
    return 0;
}
