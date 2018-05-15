#include <types.h>
#include <stdlib.h>
#include <stdio.h>

//create null job
job* newJob(){
  job *temp = (job*)malloc(sizeof(job));
  temp->arrivalTime = NULL;
  temp->jobNumber = NULL;
  temp->priority = NULL;
  temp->memUnits = NULL;
  temp->runTime = NULL;
  temp->maxDevices = NULL;
  temp->currDevices = NULL;
  return temp;
}
//create new nodes
node* newNode(job j){
  node *temp = (node*)malloc(sizeof(node));
  temp->job = j;
  temp->next = NULL;
  return temp;
}

//create an empty queue
queue *createQueue(){
  queue *q = (queue*)malloc(sizeof(struct queue));
  q->first = q->last = NULL;
  return q;
}

//add a job j to q
void addToQueue(queue *q, job j){
  node *temp = newNode(j);
  if (q->last == NULL){
    q->first = q->last = temp;
    return;
  }
  q->last->next = temp;
  q->last = temp;
}

//remove a job from the queue
node *removeFromQueue(queue *q){
  if(q->first == NULL)
    return NULL;
  node *temp = q->first;
  q->first = q->first->next;
  if(q->first == NULL)
    q->last = NULL;
  return temp;
}
