#include <stdbool.h>
#ifndef TYPES_H
#define TYPES_H

typedef struct job {
  int arrivalTime;
  bool processExists;
  int processArrival;
  int completionTime;
  int turnaroundTime;
  int weightedTurnaroundTime;
  int jobNumber;
  int priority;
  int memUnits;
  int memAllocated;
  int runTime;
  int remainingTime;
  int devicesMax;
  int devicesAllocated;
  int devicesRequested;
} job;


typedef struct node {
  job *job;
  struct node *next;
} node;

typedef struct queue {
  node *first, *last;
  int size;
} queue;

job *newJob();
node *newNode();
queue *createQueue();
void addToQueue(queue *q, job *j);
node *removeFromQueue(queue *q);
#endif /*TYPES_H*/
