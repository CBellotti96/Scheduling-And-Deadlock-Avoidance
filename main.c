#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "queues.c"
#include <stdbool.h>
//#include <json/json.h>

#define SIZE 1024 //used for file reading buffer
#define FILE_NAME "test_input1.txt" //must change based on input test

//global system vars
int currentTime;
int nextTime;
int memTotal;
int memAvailable;
int devicesTotal;
int devicesAvailable;
int quantum;
int quantumSlice;
int turnaroundTime;
int weightedTurnaroundTime;

//initializing queues
queue *submitQueue;
queue *holdQueue1;
queue *holdQueue2;
queue *readyQueue;
queue *waitQueue;
queue *runningQueue;
queue *completeQueue;
queue *acceptedJobs;


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


bool bankersCheck(){
  int processes = readyQueue->size + waitQueue->size;
  if(runningQueue->first != NULL){
    processes++;
  }
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
        if(need[p] > tempAvailable){
          continue;
        }
        tempAvailable += allocated[p];
        finished[p] = 1;
        canComplete = true;
        count++;
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

void submitJob(job *j){
  if (j->memUnits > memTotal || j->devicesMax > devicesTotal){
    removeFromQueue(acceptedJobs, j);
    printf("\nJob %d has been rejected due to insufficient memory or devices.\n", j->jobNumber);
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
    //timeStep();
    addToQueue(readyQueue, j);
    memAvailable = memAvailable - j->memUnits;
    j->memAllocated = j->memUnits;
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
    runningQueue->first->job->turnaroundTime = runningQueue->first->job->completionTime - runningQueue->first->job->arrivalTime;
    runningQueue->first->job->weightedTurnaroundTime = runningQueue->first->job->turnaroundTime / runningQueue->first->job->runTime;
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

void roundRobin(){
  if(runningQueue->first != NULL){
    if(runningQueue->first->job->remainingTime <= 0){
      completeJob(currentTime, runningQueue->first->job->jobNumber);
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

void finishQuantum(){
  if(runningQueue->first == NULL){
    quantumSlice = 0;
    return;
  }

  if(runningQueue->first->job->remainingTime < quantumSlice){
    currentTime += runningQueue->first->job->remainingTime;
    runningQueue->first->job->remainingTime = 0;
  }
  else{
    currentTime += quantumSlice;
    runningQueue->first->job->remainingTime -= quantumSlice;
  }
  quantumSlice = quantum;
}

void beginQuantum(int step){
  if(runningQueue->first != NULL){
    runningQueue->first->job->remainingTime -= step;
  }

  currentTime += step;
  quantumSlice = quantum - (step % quantum);
}

void processQuantum(){
  if(runningQueue->first == NULL){
    currentTime += quantum;
    return;
  }
  finishQuantum();
}

void resumeQuantum(int step){
  if(runningQueue->first != NULL){
    runningQueue->first->job->remainingTime -= step;
  }
  currentTime += step;
  quantumSlice -= step;
  return;
}

void timeStep(int time){

  int step = time - currentTime;

  if(step < quantumSlice && runningQueue->first != NULL && runningQueue->first->job->remainingTime > step){
      resumeQuantum(step);
      return;
  }
  finishQuantum();

  step = time - currentTime;

  roundRobin();

  while(step > quantum || (runningQueue->first != NULL && runningQueue->first->job->remainingTime < step)){
    processQuantum();
    step = time - currentTime;
    roundRobin();
  }
  beginQuantum(step);

  return;
}

void release(int time, int jobNum, int deviceNum){
  if(runningQueue->first != NULL && runningQueue->first->job->jobNumber == jobNum && runningQueue->first->job->devicesAllocated >= deviceNum){
    devicesAvailable += deviceNum;
    runningQueue->first->job->devicesAllocated -= deviceNum;

    addToQueue(readyQueue, runningQueue->first->job);
    removeHead(runningQueue);
    quantumSlice = 0;

    node *temp = newNode();
    for(temp = waitQueue->first; temp != NULL; temp = temp->next){
      if(temp->job->devicesRequested <= devicesAvailable){
        temp->job->devicesAllocated += temp->job->devicesRequested;
        devicesAvailable -= temp->job->devicesRequested;

        if(!bankersCheck()){
          temp->job->devicesAllocated -= temp->job->devicesRequested;
          devicesAvailable += temp->job->devicesRequested;
          continue;
        }
        else{
          temp->job->devicesRequested = 0;
          addToQueue(readyQueue, temp->job);
          removeFromQueue(waitQueue, temp->job);
        }
      }
    }
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

float averageTurnaroundTime(){
  int sum, numElements = 0;
  node *temp = completeQueue->first;
  while(temp!=NULL){
    sum += temp->job->turnaroundTime;
    temp = temp->next;
    numElements += 1;
  }
  return((float)sum/(float)numElements);
}

float averageWeightedTTime(){
  int sum, numElements = 0;
  node *temp = completeQueue->first;
  while(temp!=NULL){
    sum += temp->job->weightedTurnaroundTime;
    temp = temp->next;
    numElements += 1;
  }
  return((float)sum/(float)numElements);
}

void request(int time, int jobNum, int deviceNum){
  if(runningQueue->first != NULL && runningQueue->first->job->jobNumber == jobNum){
    if(devicesAvailable < deviceNum){
      printf("Cannot complete request at this time, not enough devices available \n");
      runningQueue->first->job->devicesRequested += deviceNum;
      addToQueue(waitQueue, runningQueue->first->job);
      removeHead(runningQueue);
      quantumSlice = 0;
    }
    else if(runningQueue->first->job->devicesMax < runningQueue->first->job->devicesAllocated + deviceNum){
      printf("Process requesting too many devices. \n");
    }
    else{
      devicesAvailable -= deviceNum;
      runningQueue->first->job->devicesAllocated += deviceNum;
      if(bankersCheck()){
        addToQueue(readyQueue, runningQueue->first->job);
        removeHead(runningQueue);
        quantumSlice = 0;
        return;
      }
      else{
        devicesAvailable += deviceNum;
        runningQueue->first->job->devicesAllocated -= deviceNum;
        runningQueue->first->job->devicesRequested += deviceNum;
        addToQueue(waitQueue, runningQueue->first->job);
        removeHead(runningQueue);
        quantumSlice = 0;
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

char * printQueue(queue *q, char *buffer){
  int offset = 0;
  node *temp = q->first;
  offset += snprintf(buffer+offset, SIZE-offset, "[");
  while(temp != NULL){
    offset += snprintf(buffer+offset, SIZE-offset, "%d,", temp->job->jobNumber);
    temp = temp->next;
  }
  offset += snprintf(buffer+offset, SIZE-offset, "]");
  return buffer;
}

char * printJobs(queue *q, char *buffer){
  int offset = 0;
  node *temp = q->first;
  offset += snprintf(buffer+offset, SIZE-offset, "[");
  while(temp != NULL){
    offset += snprintf(buffer+offset, SIZE-offset, "{\"arrival time\": %d,\"devices allocated\": %d,\"id\":%d,\"remaining time\": %d",
    temp->job->arrivalTime, temp->job->devicesAllocated, temp->job->jobNumber, temp->job->remainingTime);
    if(temp->job->completionTime != 0){
      offset += snprintf(buffer+offset, SIZE-offset, "\"completion time\": %d", temp->job->completionTime);
    }
    offset += snprintf(buffer+offset, SIZE-offset, "}");
    temp = temp->next;
  }
  offset += snprintf(buffer+offset, SIZE-offset, "]");
  return buffer;
}

void generateJSON(){
  printf("generating json output \n");
  char fileOutName[SIZE];
  snprintf(fileOutName, SIZE, "D%d.json",currentTime);
  FILE *fileOut;
  fileOut = fopen(fileOutName, "w");
  char line[SIZE];
  fprintf(fileOut, "{");
  fprintf(fileOut, "\"Current Time\": %d", currentTime);
  fprintf(fileOut, "\"Total Memory\": %d", memTotal);
  fprintf(fileOut, "\"Available Memory\": %d", memAvailable);
  fprintf(fileOut, "\"Total Devices\": %d", devicesTotal);
  fprintf(fileOut, "\"Available Devices\": %d", devicesAvailable);
  fprintf(fileOut, "\"Quantum\": %d", quantum);
  fprintf(fileOut, "\"Ready Queue\": %s", printQueue(readyQueue, line));
  fprintf(fileOut, "\"Running\": %d", runningQueue->first->job->jobNumber);
  fprintf(fileOut, "\"Submit Queue\": %s", printQueue(submitQueue, line));
  fprintf(fileOut, "\"Hold Queue 2\": %s", printQueue(holdQueue2, line));
  fprintf(fileOut, "\"Hold Queue 1\": %s", printQueue(holdQueue1, line));
  fprintf(fileOut, "\"Complete Queue\": %s", printQueue(completeQueue, line));
  fprintf(fileOut, "\"Wait Queue\": %s", printQueue(waitQueue, line));
  fprintf(fileOut, "\"Jobs\": %s", printJobs(acceptedJobs, line));
  fprintf(fileOut, "}");
  fclose(fileOut);
}

void output(){
  printf("\n");
  printf("******System Information****** \n");
  printf("\tCurrent Time: %d \n", currentTime);
  printf("\tTotal Memory: %d \n", memTotal);
  printf("\tAvailable Memory: %d \n", memAvailable);
  printf("\tTotal Devices: %d \n", devicesTotal);
  printf("\tAvailable Devices: %d \n", devicesAvailable);
  printf("\tQuantum: %d \n", quantum);
  if(completeQueue->first != NULL){ //only print turnaround times if there are completed jobs
    printf("\tTurnaround Time: %d \n", turnaroundTime);
    printf("\tWeighted Turnaround Time: %d \n", weightedTurnaroundTime);
  }

  printf("******ReadyQueue****** \n");
  printf("\t[");
  node *temp = newNode();
  for(temp = readyQueue->first; temp != NULL; temp = temp->next){
    printf("%d, ", temp->job->jobNumber);
  }
  printf("] \n");

  printf("******RunningQueue****** \n");
  if(runningQueue->first != NULL){
    printf("\t%d ", runningQueue->first->job->jobNumber);
  }
  printf("\n");

  printf("******SubmitQueue****** \n");
  printf("\t[");
  for(temp = submitQueue->first; temp != NULL; temp = temp->next){
    printf("%d, ", temp->job->jobNumber);
  }
  printf("] \n");

  printf("******HoldQueue2****** \n");
  printf("\t[");
  for(temp = holdQueue2->first; temp != NULL; temp = temp->next){
    printf("%d, ", temp->job->jobNumber);
  }
  printf("] \n");

  printf("******HoldQueue1****** \n");
  printf("\t[");
  for(temp = holdQueue1->first; temp != NULL; temp = temp->next){
    printf("%d, ", temp->job->jobNumber);
  }
  printf("] \n");

  printf("******CompleteQueue****** \n");
  printf("\t[");
  for(temp = completeQueue->first; temp != NULL; temp = temp->next){
    printf("%d, ", temp->job->jobNumber);
  }
  printf("] \n");

  printf("******WaitQueue****** \n");
  printf("\t[");
  for(temp = waitQueue->first; temp != NULL; temp = temp->next){
    printf("%d, ", temp->job->jobNumber);
  }
  printf("] \n");

  printf("******Jobs****** \n");
  printf("\t[ \n");
  for(temp = acceptedJobs->first; temp != NULL; temp = temp->next){
    printf("\t\t{ \n");
    printf("\t\t\tArrival Time: %d, \n", temp->job->arrivalTime);
    if(temp->job->devicesRequested != 0 || temp->job->devicesAllocated != 0){
      printf("\t\t\tDevices Allocated: %d, \n", temp->job->devicesAllocated);
    }
    printf("\t\t\tJob Number: %d, \n", temp->job->jobNumber);
    printf("\t\t\tRemaining Time: %d, \n", temp->job->remainingTime);
    if(temp->job->completionTime != 0){
      printf("\t\t\tCompletion Time: %d, \n", temp->job->completionTime);
    }
    printf("\t\t}, \n");
  }
  printf("\t] \n");
  printf("} \n");

  generateJSON();
}

void readByLineNum(int lineNum, char c){
  FILE * file = fopen(FILE_NAME, "r");
  char ignore[SIZE];
  for (int i=0; i<lineNum-1; i++){
    fgets(ignore, sizeof(ignore), file);
  }
  if(c=='C'){
    fscanf(file, "C %d M=%d S=%d Q=%d", &currentTime, &memTotal, &devicesTotal, &quantum);
    memAvailable = memTotal;
    devicesAvailable = devicesTotal;
  }
  else if(c=='A'){
    int tempArrival, tempJob, tempMem, tempDevices, tempRemaining, tempPriority;
    fscanf(file, "A %d J=%d M=%d S=%d R=%d P=%d", &tempArrival, &tempJob, &tempMem, &tempDevices, &tempRemaining, &tempPriority);
    if(tempMem < memTotal || tempDevices < devicesTotal){
      job *j = newJob();
      j->arrivalTime = tempArrival;
      j->jobNumber = tempJob;
      j->memUnits = tempMem;
      j->devicesMax = tempDevices;
      j->remainingTime = j->runTime = tempRemaining;
      j->priority = tempPriority;
      addToQueue(acceptedJobs, j);
      timeStep(j->arrivalTime);
      submitJob(j);
    }
    else{
      printf("job rejected, not enough total memory or devices.");
    }
  }
  else if(c=='Q'){
    int time, jobNum, devices;
    fscanf(file, "Q %d J=%d D=%d", &time, &jobNum, &devices);
    timeStep(time);
    request(time, jobNum, devices);
  }
  else if(c=='L'){
    int time, jobNum, devices;
    fscanf(file, "L %d, J=%d, D=%d", &time, &jobNum, &devices);
    timeStep(time);
    release(time, jobNum, devices);
  }
  else if(c=='D'){
    int time;
    fscanf(file, "D %d", &time);
    if(time == 9999){
      timeStep(time);
      printf("End of input file. Dumping final state.\n");
    }
    else{
      timeStep(time);
    }
    turnaroundTime = averageTurnaroundTime();
    weightedTurnaroundTime = averageWeightedTTime();
    output();
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
  acceptedJobs = createQueue();

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
    char c;
    int lineNum=0;
    while(fgets(currLine, SIZE, file) != NULL){
      int jj = -1;
      while(++jj < strlen(currLine)){
        lineNum++;
        if ((c=currLine[jj]) != -1) break;
      }
      readByLineNum(lineNum, c);
      //printf("%c %d \n",c, lineNum);
    }
  }
  return(0);
}
