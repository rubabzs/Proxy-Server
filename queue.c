/*
 * C Program to Implement Queue Data Structure using Linked List
 */
#include <stdio.h>
#include <stdlib.h>
 
struct node
{
  int info;
  struct node *ptr;
}*front,*rear,*temp,*front1;
 
int frontelement();
void enq(int data);
int deq();
int empty();
void display();
void create();
int queuesize();
 
int count = 0;

/* Create an empty queue */
void create()
{
  front = rear = NULL;
}
 
/* Returns queue size */
int queuesize()
{
  //  printf("\n Queue size : %d\n", count);
  return count;
}
 
/* Enqueing the queue */
void enq(int data)
{
  if (rear == NULL)
    {
      rear = (struct node *)malloc(1*sizeof(struct node));
      rear->ptr = NULL;
      rear->info = data;
      front = rear;
    }
  else
    {
      temp=(struct node *)malloc(1*sizeof(struct node));
      rear->ptr = temp;
      temp->info = data;
      temp->ptr = NULL;
 
      rear = temp;
    }
  count++;
}
 
/* Displaying the queue elements */
void display()
{
  front1 = front;
 
  if ((front1 == NULL) && (rear == NULL))
    {
      printf("Queue is empty");
      return;
    }
  while (front1 != rear)
    {
      printf("%d ", front1->info);
      front1 = front1->ptr;
    }
  if (front1 == rear)
    printf("%d", front1->info);
}
 
/* Dequeing the queue */
int deq()
{
  front1 = front;
  int temp_val;
 
  if (front1 == NULL)
    {
      printf("\n Error: Trying to display elements from empty queue");
      return -1;
    }
  else
    if (front1->ptr != NULL)
      {
	front1 = front1->ptr;
	temp_val = front->info;
	//	printf("\n Dequed value : %d", front->info);
	free(front);
	front = front1;
	count--;
	return temp_val;
      }
    else
      {
	//printf("\n Dequed value : %d", front->info);
	temp_val = front->info;
	free(front);
	front = NULL;
	rear = NULL;
	count--;
	return temp_val;
      }
  count--;
  
}
 
/* Returns the front element of queue */
int frontelement()
{
  if ((front != NULL) && (rear != NULL))
    return(front->info);
  else
    return 0;
}
 
/* Return 1 if empty, 0 otherwise */
int empty()
{
  if ((front == NULL) && (rear == NULL)) {
    //printf("\ Queue empty");
    return 1;
  }
  else {
    //printf("Queue not empty");
    return 0;
  }
}
