#ifndef _STACK_H_
#define _STACK_H_

typedef struct Node Node;

struct Node
{
    int data;
    Node* nextNode;
};

int peek();

void push(int date);

void pop(); 

int empty();

void display();

void create();

void stack_size();

#endif