#pragma once
#include "stdafx.h"

#define EXPORT __declspec(dllexport)

struct B_Node
{
   void *data;
   struct B_Node *next;
};
typedef struct B_Node B_Node;

typedef struct
{
   int nodeCount;
   size_t nodeSize;
   B_Node *front;
   B_Node *back;
}
B_Queue;

EXPORT void B_InitQueue(B_Queue*, size_t);
EXPORT BOOL B_QueueIsEmpty(B_Queue*);
EXPORT void B_PushQueue(B_Queue*, void*);
EXPORT void B_PopQueue(B_Queue*);
EXPORT void* B_QueueFront(B_Queue*);

