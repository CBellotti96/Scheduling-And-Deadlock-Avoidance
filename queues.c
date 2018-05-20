#include "types.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

//create null job
job* newJob(){
  job *temp = (struct job*)malloc(sizeof(struct job));
  return temp;
}
//create new nodes
node* newNode(){
  node *temp = (struct node*)malloc(sizeof(struct node));
  return temp;
}

//create an empty queue
queue *createQueue(){
  queue *q = (struct queue*)malloc(sizeof(struct queue));
  q->first = q->last = NULL;
  return q;
}

//add a job j to q
void addToQueue(queue *q, job *j){
  node *temp = newNode();
  temp->job = j;
  q->size++;
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
  q->size--;
  return temp;
}

/* function to swap data of two nodes a and b*/
void swap(node *a, node *b)
{
    int temp = a->job->runTime;
    a->job->runTime = b->job->runTime;
    b->job->runTime = temp;
    int temp2 = a->job->arrivalTime;
    a->job->arrivalTime = b->job->arrivalTime;
    b->job->arrivalTime = temp2;
    int temp3 = a->job->jobNumber;
    a->job->jobNumber = b->job->jobNumber;
    b->job->jobNumber = temp3;
    int temp4 = a->job->memUnits;
    a->job->memUnits = b->job->memUnits;
    b->job->memUnits = temp4;
    int temp5 = a->job->devicesMax;
    a->job->devicesMax = b->job->devicesMax;
    b->job->devicesMax = temp5;
    int temp6 = a->job->remainingTime;
    a->job->remainingTime = b->job->remainingTime;
    b->job->remainingTime = temp6;
    int temp7 = a->job->priority;
    a->job->priority = b->job->priority;
    b->job->priority = temp7;
}

//sort the queue by runtime
void sortByRuntime(queue *q){
    node *start = q->first;
    job *j = start->job;

    int swapped, i;
    node *ptr1;
    node *lptr = NULL;

    /* Checking for empty list */
    if (start == NULL)
        return;

    do{
        swapped = 0;
        ptr1 = start;
        while (ptr1->next != lptr){
            if (ptr1->job->runTime > ptr1->next->job->runTime){
                swap(ptr1, ptr1->next);
                swapped = 1;
            }
            ptr1 = ptr1->next;
        }
        lptr = ptr1;
    }
    while (swapped);
}


int main(int argc, char **argv);
