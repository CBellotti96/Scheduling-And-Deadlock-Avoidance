#ifndef TYPES_H
#define TYPES_H

typedef struct job {
  int arrivalTime;
  int jobNumber;
  int priority;
  int memUnits;
  int runTime;
  int maxDevices;
  int currDevices;
} job;

typedef struct node {
  job job;
  node *ptr;
} node;

typedef struct queue {
  node *first, *last;
} queue;
#endif /*TYPES_H*/
