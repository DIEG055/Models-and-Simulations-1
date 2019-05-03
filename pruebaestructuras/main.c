#include <stdio.h>
#include <stdlib.h>


#define Q1_LIMIT 100000
#define TOTAL_PRINTERS 3
#define TOTAL_EMPLOYEES 1

struct printer {
   int paper_inside;
   int status;
   float area_service;
};
struct printer printers[ 1 + TOTAL_PRINTERS ];


struct employee {
   int status;
   float area_service;
};
struct employee employees[ 1 + TOTAL_EMPLOYEES ];


struct queue_item {
    int paper_type;
    float time_arrival;
};
struct queue_item queue_2[Q1_LIMIT + 1];

float time_next_event[4 + TOTAL_PRINTERS + TOTAL_EMPLOYEES];

int main()
{
    printers[0].status = 1;
    printf("Hello world%d!\n", printers[0].status);
    return 0;
}
