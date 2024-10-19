#include "util.h"
#define MAX_PATH_LENGTH 255

struct data {
    int n;
    float avg;
    float std;
    char filename[MAX_PATH_LENGTH];
};

typedef struct n {
    char* val;
    struct n* next;
} Node;

void pushDir(char*, Node**);
char* popDir(Node**);