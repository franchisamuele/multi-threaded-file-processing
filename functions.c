#include "functions.h"

void pushDir(char *val, Node **head) {
    Node *n = (Node*) malloc(sizeof(Node));
    if (n == NULL) {
        printf("Errore");
        exit(-1);
    }

    n->val = (char*) malloc(sizeof(char)*MAX_PATH_LENGTH);
    strncpy(n->val, val, MAX_PATH_LENGTH);
    n->next = NULL;

    if (*head == NULL) {
        *head = n;
    } else {
        Node *ptr = *head;
        while (ptr->next != NULL) ptr = ptr->next;
        ptr->next = n;
    }
}

char* popDir(Node **head) {
    if (*head == NULL) return NULL;

    char* val = (*head)->val;
    Node *ptr = *head;
    *head = (*head)->next;
    free(ptr);
    return val;
}