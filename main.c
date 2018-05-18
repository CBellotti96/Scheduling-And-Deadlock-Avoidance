#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "queues.c"

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

void printArray(int *array, int size){
  for(int i = 0; i < size; i++){
    printf("%d ", array[i]);
  }
  printf("\n");
}

char * charAppend(char *source, char c){
  int len = strlen(source);
  s[len] = c;
  s[len+1] = '\0';
  return(source);
}

int getNum(char *line);
  char *num[SIZE];
  int i;
  for(i = 0; i < strlen(line); i++){
    if isdigit(line[i]){
      while(isdigit(line[i])){
        charAppend(num, line[i]);
        i++;
      }
      return atoi(&num);
    }
  }

int getTime(char *line);
  char *charTime;
  char *temp = (char*)malloc(SIZE);
  strcpy(temp, line);
  charTime = strtok(temp, "C A Q L D ");
  int time = getNum(charTime);
  return(time);

void getValues (char *line, int *values){
  line = strtok(line, "C A Q L D "); //remove identifier
  int i = 0;
  while(line != NULL) {
    values[i] = getNum(line);
    i++;
    line = strtok(NULL, " ");
  }
}

int main(int argc, char ** argv){
  //initializing queues
  queue *submitQueue = createQueue();
  queue *holdQueue1 = createQueue();
  queue *holdQueue2 = createQueue();
  queue *readyQueue = createQueue();
  queue *waitQueue = createQueue();
  queue *runningQueue = createQueue();
  queue *completeQueue = createQueue();

  FILE file = fopen(FILE_NAME, "r");
  size_t fileBuffer = SIZE;
  char *currLine = (char *)malloc(SIZE * sizeof(char));
  char *nextLine = (char *)malloc(SIZE * sizeof(char));
  if(!file){
    printf("Error: input file could not be read. Exiting... \n");
    return;
  }
  else{
    getline(&nextLine,&fileBuffer,file); //get initial line
    while(getline(&line,&fileBuffer,file) >= 0){
      currLine = nextLine; //current task
      getline(&nextLine,&fileBuffer,file); //next task
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
          Job *j = newJob();
          j->arrivalTime = values[0];
          j->jobNumber = values[1];
          j->memUnits = values[2];
          j->maxDevices = values[3];
          j->remainingTime = j->runTime = values[4];
          j->priority = values[5];

          //TODO
        }
      }

      else if(currLine[0] == "Q"){
        values = (int *)malloc(sizeof(int)*REQUEST_ARGS);
        getValues(currLine, values);
        printArray(values, REQUEST_ARGS);

        //TODO
      }

      else if(currLine[0] == "L"){
        values = (int *)malloc(sizeof(int)*RELEASE_ARGS);
        getValues(currLine, values);
        printArray(values, RELEASE_ARGS);

        //TODO
      }

      else if(currLine[0] == "D"){
        values = (int *)malloc(sizeof(int)*DISPLAY_ARGS);
        getValues(currLine, values);
        printArray(values, DISPLAY_ARGS);

        //TODO
      }

      else{
        printf("Unrecognized character in file. Exiting...");
        return();
      }
    }
  }
}
