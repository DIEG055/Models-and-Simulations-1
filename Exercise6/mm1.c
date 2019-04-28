#include <stdio.h>
#include <math.h>
#include "lcgrand.h"


#define BUSY      1
#define IDLE      0

/*General*/
int   next_event_type, num_events,num_units_attended,stop_simulation, end_time;
float mean_interarrival,sim_time, time_last_event,
      time_next_event[4];

/*Server 1*/
#define Q1_LIMIT 10
int num_custs_delayed_1, num_in_q_1, server_status_1;
float area_num_in_q_1, area_server_status_1, mean_service_1,
      time_arrival_1[Q1_LIMIT + 1],total_of_delays_1;
/*Server 2*/
#define Q2_LIMIT 100000
int num_custs_delayed_2,num_in_q_2,server_status_2;
float area_num_in_q_2, area_server_status_2, mean_service_2,
      time_arrival_2[Q2_LIMIT + 1],total_of_delays_2;



FILE  *infile, *outfile;

void  initialize(void);
void  timing(void);
void  arrive(void);
void arrive2(void);
void  depart(void);
void  report(void);
void  update_time_avg_stats(void);
float expon(float mean);
float poisson(float lambda);


main()  /* Main function. */
{
    /* Open input and output files. */



    /* Specify the number of events for the timing function. */
    infile  = fopen("mm1.in",  "r");
    num_events = 3;

    /* Read input parameters. */

    fscanf(infile, "%f %f %f", &mean_interarrival, &mean_service_1,
           &mean_service_2);


    /* Write report heading and input parameters. */



    /* Initialize the simulation. */

    initialize();

    /* Run the simulation while more delays are still needed. */

    /*while (num_in_q1 < Q1_LIMIT)*/
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

        switch (next_event_type)
        {
            case 1:
                arrive();
                break;
            case 2:
                depart();
                break;
            case 3:
                depart2();
                break;
        }
    }

    /* Invoke the report generator and end the simulation. */

    fclose(infile);
    report();

    return 0;
}


void initialize(void)  /* Initialization function. */
{
    /* Initialize the simulation clock. */
    end_time = 360;
    stop_simulation = 0;
    sim_time = 0.0;

    /* Initialize the state variables. */

    server_status_1   = IDLE;
    server_status_2   = IDLE;
    num_in_q_1 = 0;
    num_in_q_2 = 0;
    time_last_event = 0.0;

    /* Initialize the statistical counters. */

    num_custs_delayed_1  = 0;
    total_of_delays_1    = 0.0;
    area_num_in_q_1      = 0.0;
    area_server_status_1 = 0.0;

    num_custs_delayed_2  = 0;
    total_of_delays_2    = 0.0;
    area_num_in_q_2      = 0.0;
    area_server_status_2 = 0.0;

    /* Initialize event list.  Since no customers are present, the departure
       (service completion) event is eliminated from consideration. */

    time_next_event[1] = sim_time + expon(mean_interarrival);
    time_next_event[2] = 1.0e+30;
    time_next_event[3] = 1.0e+30;

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

void arrive2(void){

    float delay_2;
    if (server_status_2 == BUSY)
    {

       ++num_in_q_2;
       time_arrival_2[num_in_q_2] = sim_time;
    }else
    {

        delay_2            = 0.0;
        total_of_delays_2 += delay_2;
        /* Increment the number of customers delayed, and make server busy. */
        ++num_custs_delayed_2;
        server_status_2 = BUSY;
        /* Schedule a departure (service completion). */
        time_next_event[3] = sim_time + expon(mean_service_2);
    }
};


void arrive(void)  /* Arrival event function. */
{
    float delay_1;


    /* Schedule next arrival. */

    time_next_event[1] = sim_time + expon(mean_interarrival);

    /* Check to see whether server is busy. */

    if (server_status_1 == BUSY)
    {

        /* Check to see whether an Q1 is full. */
        if (num_in_q_1 > Q1_LIMIT-1)
        {

            arrive2();


        }else{
            /* Server is busy, so increment number of customers in queue. */
            ++num_in_q_1;
            time_arrival_1[num_in_q_1] = sim_time;
        }
    }

    else
    {
        /* Server is idle, so arriving customer has a delay of zero.  (The
           following two statements are for program clarity and do not affect
           the results of the simulation.) */

        delay_1            = 0.0;
        total_of_delays_1 += delay_1;

        /* Increment the number of customers delayed, and make server busy. */

        ++num_custs_delayed_1;
        server_status_1 = BUSY;

        /* Schedule a departure (service completion). */

        time_next_event[2] = sim_time + expon(mean_service_1);
    }
}


void depart(void)  /* Departure event function. */
{
    int   i;
    float delay;

    /* Check to see whether the queue is empty. */

    if (num_in_q_1 == 0)
    {
        /* The queue is empty so make the server idle and eliminate the
           departure (service completion) event from consideration. */

        server_status_1      = IDLE;
        time_next_event[2] = 1.0e+30;
    }

    else
    {
        /* The queue is nonempty, so decrement the number of customers in
           queue. */
        --num_in_q_1;

        /* Compute the delay of the customer who is beginning service and update
           the total delay accumulator. */

        delay            = sim_time - time_arrival_1[1];
        total_of_delays_1 += delay;

        /* Increment the number of customers delayed, and schedule departure. */

        ++num_custs_delayed_1;
        time_next_event[2] = sim_time + expon(mean_service_1);

        /* Move each customer in queue (if any) up one place. */

        for (i = 1; i <= num_in_q_1; ++i)
            time_arrival_1[i] = time_arrival_1[i + 1];
    }
}

void depart2(void)  /* Departure event function. */
{
    int   i;
    float delay;

    /* Check to see whether the queue is empty. */

    if (num_in_q_2 == 0)
    {

        /* The queue is empty so make the server idle and eliminate the
           departure (service completion) event from consideration. */

        server_status_2      = IDLE;
        time_next_event[3] = 1.0e+30;
    }

    else
    {
        /* The queue is nonempty, so decrement the number of customers in
           queue. */

        --num_in_q_2;

        /* Compute the delay of the customer who is beginning service and update
           the total delay accumulator. */

        delay            = sim_time - time_arrival_2[1];
        total_of_delays_2 += delay;

        /* Increment the number of customers delayed, and schedule departure. */

        ++num_custs_delayed_2;
        time_next_event[3] = sim_time + expon(mean_service_2);

        /* Move each customer in queue (if any) up one place. */

        for (i = 1; i <= num_in_q_2; ++i)
            time_arrival_2[i] = time_arrival_2[i + 1];
    }
}


void report(void)  /* Report generator function. */
{
    /* Compute and write estimates of desired measures of performance. */

    printf( "\n\nAverage delay in queue%11.3f minutes\n\n",
            total_of_delays_1 / num_custs_delayed_1);
    printf( "Average number in queue 1 %10.3f\n\n",
            area_num_in_q_1 / sim_time);
    printf( "Server 1 utilization%15.3f\n\n",
            area_server_status_1 / sim_time);

        printf( "\n\nAverage delay in queue%11.3f minutes\n\n",
            total_of_delays_2 / num_custs_delayed_2);
    printf( "Average number in queue 2 %10.3f\n\n",
            area_num_in_q_2 / sim_time);
    printf( "Server 2 utilization%15.3f\n\n",
            area_server_status_2 / sim_time);

    printf( "Time simulation ended%12.3f minutes\n\n", sim_time);

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

    area_server_status_1 += server_status_1 * time_since_last_event;
    area_server_status_2 += server_status_2 * time_since_last_event;
}

float poisson(float lambda) {
  float L = exp(-lambda);
  float p = 1.0;
  float k = 0;

  do {
    k++;
    p *= lcgrand(1);
  } while (p > L);

  return k - 1;
}

float expon(float mean)  /* Exponential variate generation function. */
{
    /* Return an exponential random variate with mean "mean". */
    return -mean * log(lcgrand(1));
}


