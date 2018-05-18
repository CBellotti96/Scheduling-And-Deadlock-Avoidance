#ifndef TYPES_H
#define TYPES_H

typedef struct job {
  int arrivalTime;
  int completionTime;
  int turnaroundTime;
  int weightedTurnaroundTime;
  int jobNumber;
  int priority;
  int memUnits;
  int runTime;
  int remainingTime;
  int maxDevices;
  int currDevices;
  int devicesRequested;
} job;

typedef struct node {
  job job;
  node *next;
} node;

typedef struct queue {
  node *first, *last;
} queue;

job *newJob();
node *newNode(job j);
queue *createQueue();
void addToQueue(queue *q, job *j);
node *removeFromQueue(queue *q);
#endif /*TYPES_H*/
