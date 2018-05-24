//Chris Bellotti and Ashley Gold

#include "types.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

//create a new job
job* newJob(){
  job *temp = (struct job*)malloc(sizeof(struct job));
  return temp;
}
//create a new node
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

//add a node with job j to queue q
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

//remove the first node from the queue
node *removeHead(queue *q){
  if(q->first == NULL)
    return NULL;
  node *temp = q->first;
  q->first = q->first->next;
  if(q->first == NULL)
    q->last = NULL;
  q->size--;
  return temp;
}

//remove a a node from the queue given its job
node *removeFromQueue(queue *q, job *j){
  if(q->first == NULL)
    return NULL;
  node *temp = q->first;
  if(temp->job->jobNumber == j->jobNumber){
    return (removeHead(q));
  }
  else{
    temp=temp->next;
    while(temp != NULL){
      if(temp->job->jobNumber == j->jobNumber){
        swap(temp, q->first);
        q->first = temp;
        removeHead(q);
        break;
      }
    temp = temp->next;
    }
  }
}

//helper function to swap the jobs of two nodes
void swap(node *a, node *b){
    job *j = a->job;
    a->job = b->job;
    b->job = j;
    node *temp = a->next;
    a->next = b->next;
    b->next = temp;
}

//sort the queue by runtime for SJF
void sortByRuntime(queue *q){
    node *start = q->first;
    job *j = start->job;

    int swapped, i;
    node *ptr1;
    node *lptr = NULL;

    //Checking for empty list
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
