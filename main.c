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

void timeStep(int time);
void processQuantum();
void roundRobin();
void request(int time, int jobNum, int deviceNum);
void release(int time, int jobNum, int requestNum);

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
    timeStep(currentTime);
    addToQueue(readyQueue, j);
    memAvailable = memAvailable - j->memUnits;
  }
}

void timeStep(int time){
  if(time <= currentTime){
    return;
  }
  int num_quantums = (int) ((time-currentTime)/quantum);
  for(int i = 0; i < num_quantums; i++){
    processQuantum(currentTime);
  }

  if(runningQueue->first == NULL){
    currentTime = time;
  }

  else{
    runningQueue->first->job->remainingTime = runningQueue->first->job->remainingTime - (time - currentTime);
    currentTime = time;
  }
}

void processQuantum(){
  roundRobin();
  if(runningQueue->first == NULL){
    currentTime += quantum;
  }
  if(runningQueue->first->job->remainingTime - quantum > 0){
    currentTime += quantum;
    runningQueue->first->job->remainingTime -= quantum;
  }
  else{
    currentTime += runningQueue->first->job->remainingTime;
    runningQueue->first->job->remainingTime = 0;
  }
}

void roundRobin(){
  if(runningQueue->first != NULL){
    if(runningQueue->first->job->remainingTime <= 0){
      devicesAvailable += runningQueue->first->job->devicesAllocated;
      runningQueue->first->job->devicesAllocated = 0;

      memAvailable += runningQueue->first->job->memAllocated;

      runningQueue->first->job->completionTime = currentTime;

      addToQueue(completeQueue, runningQueue->first->job);
    }

    else{
      addToQueue(readyQueue, runningQueue->first->job);
    }
  }
  if(readyQueue->first != NULL){
    runningQueue->first = readyQueue->first;
    removeHead(readyQueue);
  }
  else{
    runningQueue->first = NULL;
  }
}

void release(int time, int jobNum, int deviceNum){
  if(runningQueue->first != NULL && runningQueue->first->job->jobNumber == jobNum && runningQueue->first->job->devicesAllocated >= deviceNum){
    devicesAvailable += deviceNum;
    runningQueue->first->job->devicesAllocated -= deviceNum;
  }
  else if (runningQueue->first == NULL){
    printf("Invalid release. No job currently running. \n");
  }
  else if (runningQueue->first->job->devicesAllocated < deviceNum){
    printf("Invalid release. Job is not currently holding this many devices. \n");
  }
  else{
    printf("Invalid release. A different job is currently running. \n");
  }
}

bool bankersCheck(){
  int processes = readyQueue->size + waitQueue->size + runningQueue->size;
  int *need = (int *) malloc(processes  * sizeof(int));
  int *allocated = (int *) malloc(processes * sizeof(int));
  bool *finished = (bool *) malloc(processes * sizeof(bool));
  int tempAvailable = devicesAvailable;
  int i = 0;
  if(runningQueue->first != NULL){
    need[i] = runningQueue->first->job->devicesMax - runningQueue->first->job->devicesAllocated;
    allocated[i] = runningQueue->first->job->devicesAllocated;
    i++;
  }
  node *temp = newNode();
  for(temp = readyQueue->first; temp != NULL; temp = temp->next){
    need[i] = temp->job->devicesMax - temp->job->devicesAllocated;
    allocated[i] = temp->job->devicesAllocated;
    i++;
  }
  for(temp = waitQueue->first; temp != NULL; temp = temp->next){
    need[i] = temp->job->devicesMax - temp->job->devicesAllocated;
    allocated[i] = temp->job->devicesAllocated;
    i++;
  }
  int count = 0;
  while (count < processes){
    bool canComplete = false;
    for (int p = 0; p < processes; p++){
      if(finished[p] == 0){
        if(need[p] > work){ //work is undeclared?
          continue;
        }
        tempAvailable += allocated[p];
        finished[p] = 1;
        canComplete = true;
      }
    }
    if (canComplete == false){
      printf("Cannot complete request, system would be in unsafe state. \n");
      free(need);
      free(allocated);
      free(finished);
      free(temp);
      return false;
    }
  }
  printf("Request being filled, system is in a safe state. \n");
  free(need);
  free(allocated);
  free(finished);
  free(temp);
  return true;
}

void request(int time, int jobNum, int deviceNum){
  if(runningQueue->first != NULL && runningQueue->first->job->jobNumber == jobNum){
    if(devicesAvailable < deviceNum){
      printf("Cannot complete request, not enough devices available \n");
    }
    else{
      devicesAvailable -= deviceNum;
      runningQueue->first->job->devicesAllocated += deviceNum;
      if(bankersCheck()){
        return;
      }
      else{
        devicesAvailable += deviceNum;
        runningQueue->first->job->devicesAllocated -= deviceNum;
        runningQueue->first->job->devicesRequested += deviceNum;
        addToQueue(readyQueue, runningQueue->first->job);
        removeHead(runningQueue);
      }
    }
  }
  else if(runningQueue->first == NULL){
    printf("Invalid request. No job currently running. \n");
  }
  else {
    printf("Invalid request. A different job is currently running. \n");
  }
}

void completeJob(int time, int jobNum){
  if(runningQueue->first->job->jobNumber == jobNum){
    //make devices and memory available
    devicesAvailable += runningQueue->first->job->devicesAllocated;
    runningQueue->first->job->devicesAllocated = 0;
    memAvailable += runningQueue->first->job->memAllocated;
    runningQueue->first->job->memAllocated = 0;
    runningQueue->first->job->completionTime = currentTime;
    addToQueue(completeQueue, runningQueue->first->job);
    removeHead(runningQueue);

    //check waitQueue first
    node *temp = newNode();
    for(temp = waitQueue->first; temp != NULL; temp = temp->next){
      if(temp->job->devicesRequested <= devicesAvailable){
        //allocate and call bankersCheck()
        devicesAvailable -= temp->job->devicesRequested;
        temp->job->devicesAllocated += temp->job->devicesRequested;
        if(!bankersCheck()){
          devicesAvailable += temp->job->devicesRequested;
          temp->job->devicesAllocated -= temp->job->devicesRequested;
          continue;
        }
        else{
          temp->job->devicesRequested = 0;
          addToQueue(readyQueue, temp->job);
          removeFromQueue(waitQueue, temp->job);
        }
      }
    }
    if(holdQueue1->first != NULL){
      for(temp = holdQueue1->first; temp != NULL; temp = temp->next){
        if(memAvailable >= temp->job->memUnits){
          addToQueue(readyQueue, temp->job);
          memAvailable -= temp->job->memUnits;
          temp->job->memAllocated = temp->job->memUnits;
          removeFromQueue(holdQueue1, temp->job);
        }
      }
    }
    if(holdQueue2->first != NULL){
      for(temp = holdQueue2->first; temp != NULL; temp = temp->next){
        if(memAvailable >= temp->job->memUnits){
          addToQueue(readyQueue, temp->job);
          memAvailable -= temp->job->memUnits;
          temp->job->memAllocated = temp->job->memUnits;
          removeFromQueue(holdQueue2, temp->job);
        }
      }
    }
  }
}

void output(){
  printf("\n");
  printf("******System Information****** \n");
  printf("Current Time: %d \n", currentTime);
  printf("Total Memory: %d \n", memTotal);
  printf("Available Memory: %d \n", memAvailable);
  printf("Total Devices: %d \n", devicesTotal);
  printf("Available Devices: %d \n", devicesAvailable);
  printf("Quantum: %d \n", quantum);
  //implement turnaroundtime and weightedTurnaroundTime!!!

  printf("******RunningQueue****** \n");
  if(runningQueue->first != NULL){
    printf("%d \n", runningQueue->first->job->jobNumber);
  }
  else{
    printf("empty \n");
  }

  printf("******SubmitQueue****** \n");
  printf("[");
  node *temp = submitQueue->first;
  for(temp = submitQueue->first; temp != NULL; temp = temp->next){
    printf("%d, ", temp->job->jobNumber);
  }
  printf("] \n");

  printf("******HoldQueue2****** \n");
  printf("[");
  for(temp = holdQueue2->first; temp != NULL; temp = temp->next){
    printf("%d, ", temp->job->jobNumber);
  }
  printf("] \n");

  printf("******HoldQueue1****** \n");
  printf("[");
  for(temp = holdQueue1->first; temp != NULL; temp = temp->next){
    printf("%d, ", temp->job->jobNumber);
  }
  printf("] \n");

  printf("******CompleteQueue****** \n");
  printf("[");
  for(temp = completeQueue->first; temp != NULL; temp = temp->next){
    printf("%d, ", temp->job->jobNumber);
  }
  printf("] \n");

  printf("******WaitQueue****** \n");
  printf("[");
  for(temp = waitQueue->first; temp != NULL; temp = temp->next){
    printf("%d, ", temp->job->jobNumber);
  }
  printf("] \n");
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
