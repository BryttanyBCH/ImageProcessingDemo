#include "stdafx.h"
#include "B_DataStructs.h"
#include "B_Processors.h"

#define EXPORT __declspec(dllexport)

EXPORT void B_InitQueue(B_Queue *q, size_t nodeSize)
{
   q->nodeCount = 0;
   q->nodeSize = nodeSize;
   q->front = NULL;
   q->back = NULL;
}

EXPORT BOOL B_QueueIsEmpty(B_Queue *q)
{
   return (q->back == NULL);
}

EXPORT void B_PushQueue(B_Queue *q, void *data)
{
   B_Node *newNode = (B_Node*)malloc(sizeof(B_Node));
   newNode->data = malloc(q->nodeSize);
   newNode->next = NULL;

   memcpy(newNode->data, data, q->nodeSize);

   if (!B_QueueIsEmpty(q))
   {
      q->back->next = newNode;
      q->back = newNode;
   }
   else
   {
      q->front = newNode;
      q->back = newNode;
   }

   q->nodeCount++;
}

EXPORT void B_PopQueue(B_Queue *q)
{
   if (q->nodeCount > 0)
   {
      B_Node *tmp = q->front;
      q->front = q->front->next;
      if (q->front == NULL) { q->back = NULL; }
      q->nodeCount--;
      free(tmp->data);
      free(tmp);
   }
}

EXPORT void* B_QueueFront(B_Queue *q)
{
   return (q->front->data);
}

