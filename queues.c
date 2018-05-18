#include "types.h"
#include <stdlib.h>
#include <stdio.h>

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

int main(int argc, char **argv);