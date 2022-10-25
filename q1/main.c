#include "headers.h"
machine *machinetrack[100];
student *studenttrack[100];
pthread_t studentthreads[100];
pthread_t machinethreads[100];
INT num_students, num_machines;
INT student_count;
int start_time;

void *thread_student(void *arg)
{
    int num = *(int *)arg;
    sleep(studenttrack[num]->arrival_time);
    pthread_mutex_lock(&studenttrack[num]->mutex);
    studenttrack[num]->waiting = 1;
    pthread_mutex_unlock(&studenttrack[num]->mutex);
    printf("Student %d arrived", num);
    wait_for_slot(num);
    printf("Student %d allocated a slot at machine %d", num, studenttrack[num]->machine_index); // this is the student in slot fuction
    // starts washing in the machine thread
    return NULL;
}
int wait_for_slot(int num)
{
    pthread_cond_wait(&studenttrack[num]->cond, &studenttrack[num]->mutex);
    
    pthread_mutex_unlock(&studenttrack[num]->mutex);
}
void *thread_table(void *arg)
{
    int input = *(int *)arg;
    while (num_students > 0)
    {
        ready_to_serve_table(input);
    }
    return NULL;
}
int ready_to_serve_table(int num)
{
    int slots = 1;
    printf("Table %d made %d slots available", num, slots);
    int flag = 1; // if flag is k_students that means no student is waiting
    while (slots > 0 && flag < num_students)
    {
        flag = 0;
        for (int i = 0; i < num_students; i++)
        {
            if (studenttrack[i]->waiting == 1)
            {
                int s = pthread_mutex_trylock(&studenttrack[i]->mutex);
                if (s == 0)
                {
                    if (studenttrack[i]->waiting == 1)
                    {
                        studenttrack[i]->waiting = 2;
                        student_count--;
                        pthread_mutex_unlock(&studenttrack[i]->mutex);
                        pthread_cond_signal(&studenttrack[num]->cond);
                        slots--;
                    }
                }
            }
            else
            {
                flag++;
            }
        }
    }
    for (int i = 0; i < num_students; i++)
    {
        if (studenttrack[i]->machine_index == num)
        {
            printf("Student %d eating at machine %d\n", i, num);
        }
    }
    return 0;
}

int main(void)
{
    scanf("%lld %lld", num_students, num_machines);
    for (INT i = 0; i < num_machines; i++)
    {
        machinetrack[i] = (struct machine *)malloc(sizeof(struct machine));
        machinetrack[i]->index = i;
        machinetrack[i]->working = 0;
    }
    for (INT i = 0; i < num_students; i++)
    {
        studenttrack[i] = (struct student *)malloc(sizeof(struct student));
        studenttrack[i]->machine_index = -1;
        INT arrive, wait_for, patience;
        scanf("%lld %lld %lld", arrive, wait_for, patience);
        studenttrack[i]->arrival_time = arrive;
        studenttrack[i]->waiting_time = wait_for;
        studenttrack[i]->patience_time = patience;
        studenttrack[i]->waiting = 0;
    }
    for (int i = 0; i < num_students; i++)
    {
        pthread_create(&studentthreads[i], NULL, thread_student, (void *)(i));
    }
}