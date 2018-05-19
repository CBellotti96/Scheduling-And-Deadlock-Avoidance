#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "queues.c"
#include <stdbool.h>

#define SIZE 1024 //used for file reading buffer
#define CONFIG_ARGS 4
#define ARRIVAL_ARGS 6
#define REQUEST_ARGS 3
#define RELEASE_ARGS 3
#define DISPLAY_ARGS 1
#define FILE_NAME "test_input.txt" //must change based on input test

//global system vars
int currentTime;
int nextTime;
int memTotal;
int memAvailable;
int devicesTotal;
int devicesAvailable;
int quantum;

//initializing queues
queue *submitQueue;
queue *holdQueue1;
queue *holdQueue2;
queue *readyQueue;
queue *waitQueue;
queue *runningQueue;
queue *completeQueue;

void printArray(int *array, int size){
  for(int i = 0; i < size; i++){
    printf("%d ", array[i]);
  }
  printf("\n");
}


int getNum(char *line){
  int i;
  for(i = 0; i < strlen(line); i++){
    if isdigit(line[i]){
      return (atoi(&line[i]));
    }
  }
}
int getTime(char *line){
  char * charTime;
  char * temp = (char*)malloc(SIZE);
  strcpy(temp, line);
  charTime = strtok(temp, "C A Q L D ");
  int numTime = getNum(charTime);
  return(numTime);
}
void getValues (char *line, int *values){
  line = strtok(line, "C A Q L D "); //remove identifier
  int i = 0;
  while(line != NULL) {
    values[i] = getNum(line);
    i++;
    line = strtok(NULL, " ");
  }
}

bool hold_queue_1 (job *j1, job *j2){
  return j1->runTime < j2->runTime; //SJF Scheduling
}

bool hold_queue_2 (job *j1, job *j2){
  return j1->runTime < j2->runTime; //FIFO Scheduling
}

void submit_job(job *j){
  if (j->memUnits > memTotal || j->devicesMax > devicesTotal){
    printf("Job has been rejected due to insufficient memory or devices.");
  }
  else if(j->memUnits > memAvailable){
    if(j->priority == 1){
      addToQueue(holdQueue1, j);
      sortByRuntime(holdQueue1); //Sorting by runtime so SJF can be used
    }
    else{
      addToQueue(holdQueue2, j);
    }
  }
  else{
    j->processExists = true;
    //something with time?
    addToQueue(readyQueue, j);
    memAvailable = memAvailable - j->memUnits;
  }
}

void timeStep(int time){
  if(time <= currTime){
    return;
  }
  int num_quantums = (int) ((time-currTime)/quantum);
  for(int i = 0; i < num_quantums; i++){
    processQuantum();
  }

  if(runningQueue->first == NULL){
    currTime = time;
  }

  else{
    runningQueue->first->remainingTime = runningQueue->first->remainingTime - (time - currTime);
    currTime = time;
  }
}

void processQuantum(){
  roundRobin();
  if(readyQueue->first == NULL){

  }
}

int main(int argc, char ** argv){
  //creating queues
  submitQueue = createQueue();
  holdQueue1 = createQueue();
  holdQueue2 = createQueue();
  readyQueue = createQueue();
  waitQueue = createQueue();
  runningQueue = createQueue();
  completeQueue = createQueue();

  FILE * file = fopen(FILE_NAME, "r");
  FILE * file2 = fopen(FILE_NAME, "r");
  size_t fileBuffer = SIZE;
  char *currLine = (char *)malloc(SIZE * sizeof(char));
  char *nextLine = (char *)malloc(SIZE * sizeof(char));
  if(!file){
    printf("Error: input file could not be read. Exiting... \n");
    return(0);
  }
  else{
    getline(&nextLine,&fileBuffer,file2); //get initial line to look ahead for next task
    while(getline(&currLine,&fileBuffer,file) >= 0){ //current task
      getline(&nextLine,&fileBuffer,file2); //next task
      currentTime = getTime(currLine);
      nextTime = getTime(nextLine);
      int * values;

      if(currLine[0] == 'C'){
        values = (int *)malloc(sizeof(int)*CONFIG_ARGS);
        getValues(currLine,values);
        printArray(values, CONFIG_ARGS);
        currentTime = values[0];
        memTotal = memAvailable = values[1];
        devicesTotal = devicesAvailable = values[2];
        quantum = values[3];
      }

      else if(currLine[0] == 'A'){
        values = (int *)malloc(sizeof(int)*ARRIVAL_ARGS);
        getValues(currLine, values);
        printArray(values, ARRIVAL_ARGS);

        if(values[2] < memTotal || values[3] < devicesTotal){
          job *j = newJob();
          printf("new job!");
          j->arrivalTime = values[0];
          j->jobNumber = values[1];
          j->memUnits = values[2];
          j->devicesMax = values[3];
          j->remainingTime = j->runTime = values[4];
          j->priority = values[5];

          if(j->memUnits <= memTotal){
            //change something with time??
            submit_job(j);
          }

          //TODO
        }

        else{
          printf("job rejected, not enough total memory or devices.");
        }
      }

      else if(currLine[0] == 'Q'){
        values = (int *)malloc(sizeof(int)*REQUEST_ARGS);
        getValues(currLine, values);
        printArray(values, REQUEST_ARGS);

        //TODO
      }

      else if(currLine[0] == 'L'){
        values = (int *)malloc(sizeof(int)*RELEASE_ARGS);
        getValues(currLine, values);
        printArray(values, RELEASE_ARGS);

        //TODO
      }

      else if(currLine[0] == 'D'){
        values = (int *)malloc(sizeof(int)*DISPLAY_ARGS);
        getValues(currLine, values);
        printArray(values, DISPLAY_ARGS);

        //TODO
      }

      else{
        printf("Unrecognized character in file. Exiting...");
        return(0);
      }
    }
  }
  return(0);
}
