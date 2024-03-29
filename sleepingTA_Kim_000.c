#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<semaphore.h>

#define MAX_SLEEP_TIME	3
#define NUM_OF_STUDENTS	4
#define NUM_OF_HELPS	2
#define NUM_OF_SEAT		2

int waiting_students;
pthread_mutex_t mutex_lock;
sem_t students_sem; //ta waits for student to show up, student notifies ta of their arrival
sem_t ta_sem; //student wait for ta to help, ta notify student he is ready to help

void* student_thread(void* param){
	int order = (int)param;
	int seed = (int)param*1000;
	int sleeptime = (rand_r(&seed)%3)+1;
	int helptime = NUM_OF_HELPS;
	printf("	Student %d is programming for %d seconds.\n", order, sleeptime);
	sleep(sleeptime);

	while(helptime > 0){
		if(waiting_students < 2){
			if(waiting_students==0){
				//critical section
				pthread_mutex_lock(&mutex_lock);
				waiting_students++;
				pthread_mutex_unlock(&mutex_lock);

				//wake TA up
				printf("		Student %d takes a seat, # of waiting studentsA = %d\n", order, waiting_students);
				sem_post(&students_sem);

				//sleep for a while
				sleep(sleeptime);

				//wait for TA
				sem_wait(&ta_sem);
				printf("Student %d receiving help\n", order);
				helptime--;
			}else{
				//critical section
				pthread_mutex_lock(&mutex_lock);
				waiting_students++;
				pthread_mutex_unlock(&mutex_lock);

				//signaling ta
				sem_post(&students_sem);
				printf("		Student %d takes a seat, # of waiting students = %d\n", order, waiting_students);

				//wait for ta
				sem_wait(&ta_sem);
				printf("Student %d receiving help\n", order);
				sleep(sleeptime);
				helptime--;
			}
		}else{ //go back to sleep
			printf("			Student %d will try later.\n", order);
			sleeptime = (rand_r(&seed)%3)+1;
			printf("	Student %d is programming for %d seconds.\n", order, sleeptime);
			sleep(sleeptime);
		}
	}

	pthread_exit(NULL);
}

void* ta_thread(void* param){
	while(0==0){
		sem_wait(&students_sem);

		while(waiting_students > 0){
			pthread_mutex_lock(&mutex_lock);
			waiting_students--;
			pthread_mutex_unlock(&mutex_lock);
		    sem_post(&ta_sem);	

			printf("Helping a student for 3 seconds, # of waiting students = %d\n", waiting_students);
			sleep(MAX_SLEEP_TIME);
		}
	}

}

int main(){
	printf("CS149 Sleeping TA from Kim Pham\n");

	//studen & ta thread  thread
	pthread_t student[NUM_OF_STUDENTS];
	pthread_attr_t attr;
	pthread_t ta;


	//init mutex, semaphore, attribute
	pthread_mutex_init(&mutex_lock, NULL);
	pthread_attr_init(&attr);
	sem_init(&students_sem, 0, 1);
	sem_init(&ta_sem, 0, 1);


	long seeds[4] = {1,2,3,4};

	//create thread
	for(int i = 0; i < NUM_OF_STUDENTS+1; i++ ){
		if(i < NUM_OF_STUDENTS)
			pthread_create(&student[i], &attr, student_thread, (void*)seeds[i]);
		else pthread_create(&ta, &attr, ta_thread, NULL);
	}

	//joining all exit student thread
	for(int i = 0;i < NUM_OF_STUDENTS; i++){
		pthread_join(student[i], NULL);
	}

	pthread_cancel(ta);
	printf("program terminated");
	pthread_mutex_destroy(&mutex_lock);
}
