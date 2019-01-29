#include "stack.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

Node* root = NULL;
int size = 0;

Node* get_last_node()
{
    Node* curNode = root;
    while(curNode->nextNode != NULL){
        curNode = curNode->nextNode;
    }
    return curNode;
}

int peek()
{
    if(empty())
    {
        perror("Nothing to peek");
        return 0;
    }

    return get_last_node()->data;
}

void push(int data)
{
    if(size == 0)
    {
        root = malloc(sizeof(Node));
        root->data = data;
        root->nextNode = NULL;
    }
    else
    {
        Node* lastNode = get_last_node();
        Node* newNode = malloc(sizeof(Node));
        newNode->data = data;
        newNode->nextNode = NULL;
        lastNode->nextNode = newNode;
    }
    size++;
}

void pop()
{
    Node* preNode = root;
    if(size == 1)
    {
        free(root);
        root = NULL;
        size--;
    }
    else if(size > 1)
    {
        free(preNode->nextNode);
        preNode->nextNode = NULL;
        size--;
    }
}

int empty()
{
    return size == 0;
}

void display()
{
    Node* curNode = root;
    while(curNode->nextNode != NULL){
        printf("%d\n", curNode->data);
        curNode = curNode->nextNode;
    }
    printf("%d\n", curNode->data);
}

void create()
{
    while(size > 0){
        pop();
    }
}

void stack_size()
{
    printf("%d\n", size);
}