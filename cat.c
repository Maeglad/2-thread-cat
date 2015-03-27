#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFSIZE 300000 

char buffer[BUFFSIZE];

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t readCond = PTHREAD_COND_INITIALIZER; 
pthread_cond_t writeCond = PTHREAD_COND_INITIALIZER; 

int val; 

void *reader(){

	while((val = read(0, buffer, BUFFSIZE)) > 0 ){
		pthread_mutex_lock(&lock);
		
		pthread_cond_signal(&writeCond);

		while(val > 0){
			pthread_cond_wait(&readCond, &lock);
		}

		pthread_mutex_unlock(&lock);
	}

	if(val != 0){
		perror("read error");
		exit(-1);
	}

	pthread_cond_signal(&writeCond);

	pthread_exit(NULL);
}

void *writer(){

	int writeVal = 0;


	while(1){
		pthread_mutex_lock(&lock);

		while(val == 0){

			pthread_cond_wait(&writeCond, &lock);
		}
		writeVal += write(1, buffer+writeVal, val-writeVal);
		if(writeVal == -1){
			perror("Write error");
			exit(-1);
		}
		if(writeVal == val){
			writeVal = 0;
			val = 0;
		    pthread_cond_signal(&readCond);		
		}

		
		pthread_mutex_unlock(&lock);
	}
	
//	printf("Writer konci"); fflush(stdout);
	pthread_exit(NULL);
}


int main(){
	pthread_t readThread, writeThread;

	val = 0;

	if(pthread_create(&readThread, NULL, reader, NULL) != 0){
		perror("reader thread not started");
		exit(-1);
	}
	if(pthread_create(&writeThread, NULL, writer, NULL) != 0){
		perror("writer thread not started");
		exit(-1);
	}
	pthread_join(readThread, NULL);
    if(pthread_cancel(writeThread) != 0){
        perror("pthread_cancel failed");
        exit(-1);
    }
	pthread_join(writeThread, NULL);
	pthread_mutex_destroy(&lock);
	return 0;
}
