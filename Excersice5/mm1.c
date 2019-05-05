#include <stdio.h>
#include <math.h>
#include "lcgrand.h"
#include <stdlib.h>

#define BUSY      1
#define IDLE      0
#define TOTAL_PRINTERS  3
#define TOTAL_EMPLOYEES 1
#define Q_LIMIT 100000
int TOTAL_SEED=15;
int SEED;
/*Acumulators for multiple iteratations with different seeds*/
float AVERAGE_DELAY_Q1,AVERAGE_NUMBER_Q1,AVERAGE_DELAY_Q2,AVERAGE_NUMBER_Q2;
float PRINTER_UTILIZATION[1+TOTAL_PRINTERS],EMPLOYEE_UTILIZATION[1+TOTAL_EMPLOYEES];

/*General*/
int   next_event_type, num_events,num_units_attended,stop_simulation, end_time;
float sim_time, time_last_event, time_next_event[4 + TOTAL_PRINTERS  + TOTAL_EMPLOYEES];

/* Arrivals */
float mean_interarrival_t1, mean_interarrival_t2, mean_interarrival_t3;

/* Printers */
float mean_service_printer_t1,mean_service_printer_t2, mean_service_printer_t3;
struct printer {
   int paper_inside;
   int status;
   float area_server_status;
};
struct printer printers[ 1 + TOTAL_PRINTERS ];


/* Employees */
float mean_service_employee_t1,mean_service_employee_t2, mean_service_employee_t3;
struct employee {
   int status;
   float area_server_status;
};
struct employee employees[ 1 + TOTAL_EMPLOYEES ];


struct queue_item {
    int paper_type;
    float time_arrival;
};

/*Queue 1*/
int num_custs_delayed_1, num_in_q_1;
float area_num_in_q_1,total_of_delays_1;
struct queue_item queue_1[Q_LIMIT + 1];

/*Queue 2*/
int num_custs_delayed_2,num_in_q_2;
float area_num_in_q_2 ,total_of_delays_2;
struct queue_item queue_2[Q_LIMIT + 1];


FILE  *infile, *outfile;

void  initialize(void);
void  timing(void);
void  arrive(int type);
void  arrive2(int type);
void  depart_printer(int number);
void  depart_employee (int number);
void  report(void);
void  update_time_avg_stats(void);
float expon(float mean);


main()  /* Main function. */
{
    /* Open input and output files. */
    infile  = fopen("mm1.in",  "r");
    outfile = fopen("mm1.out", "w");

    /* Specify the number of events for the timing function. */
    num_events = 3 + TOTAL_PRINTERS  + TOTAL_EMPLOYEES;
    SEED = 1;


    /* Read input parameters. */

    fscanf(infile,"%f %f %f %f %f %f %f %f %f", &mean_interarrival_t1, &mean_interarrival_t2,&mean_interarrival_t3,
                                     &mean_service_printer_t1, &mean_service_printer_t2, &mean_service_printer_t3,
                                     &mean_service_employee_t1,&mean_service_employee_t2,&mean_service_employee_t3);

    mean_interarrival_t1 =  60/mean_interarrival_t1;     // 4 papers / hour
    mean_interarrival_t2 = 60/mean_interarrival_t2;    // 8 papers / hour
    mean_interarrival_t3 =  60/mean_interarrival_t3;   // 16 papers / hour


   /* Initialize the simulation. */


    /* Run the simulation while more delays are still needed. */
    for(SEED=1;SEED<=TOTAL_SEED;SEED++){
        initialize();
        while(sim_time < end_time )
    {
        /* Determine the next event. */

        timing();
        if(stop_simulation == 1){
            break;
        }
        /* Update time-average statistical accumulators. */

        update_time_avg_stats();

        /* Invoke the appropriate event function. */
        if (next_event_type <= 3){
            arrive(next_event_type);
        } else if (next_event_type <= 3 + TOTAL_PRINTERS){
            depart_printer(next_event_type  - 3 );
        } else {
            depart_employee(next_event_type - 3 - TOTAL_PRINTERS);
        }
    }
    report();
    }
    /* Invoke the report generator and end the simulation. */
    general_report();
    fclose(infile);
    fclose(outfile);

    return 0;
}


void initialize(void)  /* Initialization function. */
{
    /* Initialize the simulation clock. */
    end_time = 3000;
    stop_simulation = 0;
    sim_time = 0.0;

    /* Initialize the state variables. */

    for (int i = 1 ;  i <= TOTAL_PRINTERS ;  i++){
        printers[i].status = IDLE;
        printers[i].area_server_status = 0.0;
    }

    for (int i = 1 ;  i <= TOTAL_EMPLOYEES ;  i++){
        employees[i].status = IDLE;
        employees[i].area_server_status = 0.0;
    }

    num_in_q_1 = 0;
    num_in_q_2 = 0;
    time_last_event = 0.0;

    /* Initialize the statistical counters. */

    num_custs_delayed_1  = 0;
    total_of_delays_1    = 0.0;
    area_num_in_q_1      = 0.0;

    num_custs_delayed_2  = 0;
    total_of_delays_2    = 0.0;
    area_num_in_q_2      = 0.0;

    /* Initialize event list.  Since no customers are present, the departure
       (service completion) event is eliminated from consideration. */

    time_next_event[1] = sim_time + expon(mean_interarrival_t1);
    time_next_event[2] = sim_time + expon(mean_interarrival_t2);
    time_next_event[3] = sim_time + expon(mean_interarrival_t3);

    for (int i = 4; i <= num_events; ++i)
        time_next_event[i] = 1.0e+30;
}


void timing(void)  /* Timing function. */
{
    int   i;
    float min_time_next_event = 1.0e+29;

    next_event_type = 0;

    /* Determine the event type of the next event to occur. */

    for (i = 1; i <= num_events; ++i)
        if (time_next_event[i] < min_time_next_event)
        {
            min_time_next_event = time_next_event[i];
            next_event_type     = i;
        }

    /* Check to see whether the event list is empty. */

    if (next_event_type == 0)
    {
        /* The event list is empty, so stop the simulation. */

        fprintf(outfile, "\nEvent list empty at time %f", sim_time);
        exit(1);
    }

    /* The event list is not empty, so advance the simulation clock. */


    if(min_time_next_event > end_time){
        stop_simulation= 1;
    }else{
        sim_time = min_time_next_event;
    }

}

void arrive2(int type){
    float delay_2;
    /* Check to see whether server is busy. */

    for (int i = 1 ; i <= TOTAL_EMPLOYEES ; ++i)
        if (employees[i].status == IDLE){
            delay_2            = 0.0;
            total_of_delays_2 += delay_2;

            /* Increment the number of customers delayed, and make server busy. */
            ++num_custs_delayed_2;
            employees[i].status = BUSY;

            /* Schedule a departure (service completion). */
            switch (type)
            {
                case 1:
                    time_next_event[3 + TOTAL_PRINTERS + i] = sim_time + expon(mean_service_employee_t1);
                    break;
                case 2:
                    time_next_event[3 + TOTAL_PRINTERS + i] = sim_time + expon(mean_service_employee_t2);
                    break;
                case 3:
                    time_next_event[3 + TOTAL_PRINTERS + i] = sim_time + expon(mean_service_employee_t3);
                    break;
            }

            return;
        }

    /* All printers are BUSY */
    ++num_in_q_2;
    queue_2[num_in_q_2].time_arrival = sim_time;
    queue_2[num_in_q_2].paper_type = type;
};


void arrive(int type)  /* Arrival event function. */
{
    float delay_1;


    /* Schedule next arrival. */
    switch (type)
    {
        case 1:
            time_next_event[1] = sim_time + expon(mean_interarrival_t1);
            break;
        case 2:
            time_next_event[2] = sim_time + expon(mean_interarrival_t2);
            break;
        case 3:
            time_next_event[3] = sim_time + expon(mean_interarrival_t3);
            break;
    }

    /* Check to see whether server is busy. */

    for (int i = 1 ; i <= TOTAL_PRINTERS ; ++i)
        if (printers[i].status == IDLE){
            delay_1            = 0.0;
            total_of_delays_1 += delay_1;

            /* Increment the number of customers delayed, and make server busy. */
            ++num_custs_delayed_1;
            printers[i].status = BUSY;
            printers[i].paper_inside = type;


            /* Schedule a departure (service completion). */
            switch (type)
            {
                case 1:
                    time_next_event[3 + i] = sim_time + expon(mean_service_printer_t1);
                    break;
                case 2:
                    time_next_event[3 + i] = sim_time + expon(mean_service_printer_t2);
                    break;
                case 3:
                    time_next_event[3 + i] = sim_time + expon(mean_service_printer_t3);
                    break;
            }

            return;
        }

    /* All printers are BUSY */
    ++num_in_q_1;
    queue_1[num_in_q_1].time_arrival = sim_time;
    queue_1[num_in_q_1].paper_type = type;
}


void depart_printer(int number)  /* Departure event function. */
{
    int   i;
    float delay;
    int paper_in_printer;
    int type;

    /* Get type of paper assigned to printer */
    paper_in_printer = printers[ number ].paper_inside;

    arrive2(paper_in_printer);

    /* Check to see whether the queue is empty. */

    if (num_in_q_1 == 0)
    {
        /* The queue is empty so make the server idle and eliminate the
           departure (service completion) event from consideration. */

        printers[ number ].status     = IDLE;
        time_next_event[3 + number] = 1.0e+30;
    }

    else
    {
        /* The queue is nonempty, so decrement the number of customers in
           queue. */
        --num_in_q_1;

        /* Compute the delay of the customer who is beginning service and update
           the total delay accumulator. */

        delay            = sim_time - queue_1[1].time_arrival;
        total_of_delays_1 += delay;

        /* Increment the number of customers delayed, and schedule departure. */

        ++num_custs_delayed_1;

        /* type of paper entering the service in printer */
        type = queue_1[1].paper_type;

        printers[ number ].paper_inside = type;


        /* Schedule a departure (service completion). */
        switch (type)
        {
            case 1:
                time_next_event[3 + number] = sim_time + expon(mean_service_printer_t1);
                break;
            case 2:
                time_next_event[3 + number] = sim_time + expon(mean_service_printer_t2);
                break;
            case 3:
                time_next_event[3 + number] = sim_time + expon(mean_service_printer_t3);
                break;
        }


        /* Move each customer in queue (if any) up one place. */

        for (i = 1; i <= num_in_q_1; ++i)
            queue_1[i] = queue_1[i + 1];
    }
}

void depart_employee (int number)  /* Departure event function. */
{
    int   i;
    float delay;
    int type;

    /* Check to see whether the queue is empty. */

    if (num_in_q_2 == 0)
    {

        /* The queue is empty so make the server idle and eliminate the
           departure (service completion) event from consideration. */

        employees[ number ].status     = IDLE;
        time_next_event[3 + TOTAL_PRINTERS + number] = 1.0e+30;
    }

    else
    {
        /* The queue is nonempty, so decrement the number of customers in
           queue. */

        --num_in_q_2;

        /* Compute the delay of the customer who is beginning service and update
           the total delay accumulator. */

        delay            = sim_time -  queue_2[1].time_arrival;
        total_of_delays_2 += delay;

        /* Increment the number of customers delayed, and schedule departure. */

        ++num_custs_delayed_2;

        /* type of paper entering the service in employee */
        type = queue_2[1].paper_type;

        // time_next_event[2] = sim_time + expon(mean_service_1);
        /* Schedule a departure (service completion). */
        switch (type)
        {
            case 1:
                time_next_event[3 + TOTAL_PRINTERS + number] = sim_time + expon(mean_service_employee_t1);
                break;
            case 2:
                time_next_event[3 + TOTAL_PRINTERS + number] = sim_time + expon(mean_service_employee_t2);
                break;
            case 3:
                time_next_event[3 + TOTAL_PRINTERS + number] = sim_time + expon(mean_service_employee_t3);
                break;
        }
        /* Move each customer in queue (if any) up one place. */

        for (i = 1; i <= num_in_q_1; ++i)
            queue_2[i] = queue_2[i + 1];
    }
}

void general_report(void){
    fprintf(outfile,"AVERAGE_DELAY_Q1: %11.3f\n",AVERAGE_DELAY_Q1/TOTAL_SEED);
    fprintf(outfile,"AVERAGE_NUMBER_Q1: %11.3f\n",AVERAGE_NUMBER_Q1/TOTAL_SEED);
    fprintf(outfile,"AVERAGE_DELAY_Q2: %11.3f\n",AVERAGE_DELAY_Q2/TOTAL_SEED);
    fprintf(outfile,"AVERAGE_NUMBER_Q2: %11.3f\n",AVERAGE_NUMBER_Q2/TOTAL_SEED);

    for (int i = 1 ;  i <= TOTAL_PRINTERS ;  i++){
           fprintf(outfile, "Printer #%d utilization : %15.3f\n", i , PRINTER_UTILIZATION[i]/TOTAL_SEED);
        }
    for (int i = 1 ;  i <= TOTAL_EMPLOYEES ;  i++){
           fprintf(outfile, "Employee #%d utilization : %15.3f\n", i , EMPLOYEE_UTILIZATION[i]/TOTAL_SEED);
        }
}
void report(void)  /* Report generator function. */
{
    /* Compute and write estimates of desired measures of performance. */
    AVERAGE_DELAY_Q1+=(total_of_delays_1 / num_custs_delayed_1);
    AVERAGE_NUMBER_Q1+=(area_num_in_q_1 / sim_time);
    AVERAGE_DELAY_Q2+=(total_of_delays_2 / num_custs_delayed_2);
    AVERAGE_NUMBER_Q2+=(area_num_in_q_2 / sim_time);

    for (int i = 1 ;  i <= TOTAL_PRINTERS ;  i++){
        PRINTER_UTILIZATION[i]+= (printers[i].area_server_status / sim_time );
        printf("Printer #%d utilization : %15.3f\n", i , printers[i].area_server_status / sim_time );
    }

    /*fprintf(outfile, "\n\nEmployees\n\n");*/
    for (int i = 1 ;  i <= TOTAL_EMPLOYEES ;  i++){
        EMPLOYEE_UTILIZATION[i]+=(employees[i].area_server_status / sim_time );
       //printf("Employee #%d utilization : %15.3f\n", i , employees[i].area_server_status / sim_time );
    }


   /* fprintf(outfile, "\n\nTime simulation ended%12.3f minutes\n\n", sim_time);*/
}


void update_time_avg_stats(void)  /* Update area accumulators for time-average
                                     statistics. */
{
    float time_since_last_event;

    /* Compute time since last event, and update last-event-time marker. */

    time_since_last_event = sim_time - time_last_event;
    time_last_event       = sim_time;

    /* Update area under number-in-queue function. */

    area_num_in_q_1      += num_in_q_1 * time_since_last_event;
    area_num_in_q_2      += num_in_q_2 * time_since_last_event;

    /* Update area under server-busy indicator function. */
    for (int i = 1 ;  i <= TOTAL_PRINTERS ;  i++){
        printers[i].area_server_status += printers[i].status * time_since_last_event;
    }

    for (int i = 1 ;  i <= TOTAL_EMPLOYEES ;  i++){
        employees[i].area_server_status += employees[i].status * time_since_last_event;
    }

}

float expon(float mean)  /* Exponential variate generation function. */
{
    /* Return an exponential random variate with mean "mean". */
    return -mean * log(lcgrand(SEED));
}
