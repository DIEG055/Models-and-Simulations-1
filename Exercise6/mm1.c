#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include "lcgrand.h"


#define BUSY      1
#define IDLE      0

/*General*/
int   next_event_type, num_events,num_units_attended,
        stop_simulation, end_time;

float mean_interarrival,sim_time, time_last_event,
        time_next_event[7], probability_type1_client,
        probability_type2_client;

/* Type 1 client*/
float mean_service_1;


/* Type 2 client*/
float mean_service_2;

/* Queue type 1*/
#define Q1_LIMIT 100000
int num_custs_delayed_1, num_in_q_1;
float area_num_in_q_1, time_arrival_1[Q1_LIMIT + 1],
    total_of_delays_1;

/* Queue type 2 */
#define Q2_LIMIT 100000
int num_custs_delayed_2, num_in_q_2;
float area_num_in_q_2, time_arrival_2[Q2_LIMIT + 1],
    total_of_delays_2;

/* Server A1*/
int server_status_A1;
float area_server_status_A1_type1;
float area_server_status_A1_type2;

/* Server A2*/
int server_status_A2;
float area_server_status_A2_type1;
float area_server_status_A2_type2;

/*Server B*/
int server_status_B;
float area_server_status_B_type1;
float area_server_status_B_type2;


FILE  *infile, *outfile;

void  initialize(void);
void  timing(void);
void  arrive(void);
void depart_A1(void);
void depart_A2(void);
void depart_B(void);
void depart_B_A1 (void);
void depart_B_A2 (void);
void  report(void);
void  update_time_avg_stats(void);
float expon(float mean);
float randomReal(void);
bool is_there_any_server_idle(void);
bool is_there_any_serverA_idle(void);
float get_mean_service_2(void);
void attend_queue_1 (int event_type);
void attend_queue_2 (int event_type);


main()  /* Main function. */
{
    /* Specify the number of events for the timing function. */
    infile  = fopen("mm1.in",  "r");
    num_events = 6;

    /* Read input parameters. */

    /*fscanf(infile, "%f %f %f", &mean_interarrival, &mean_service_1,
           &mean_service_2);*/

    mean_interarrival = 1.0;
    probability_type1_client = 0.7;
    probability_type2_client = 0.3;
    mean_service_1 = 0.8;

    /**
        En el programa se usa una funcion get_mean_service_2(); que solo devuelve mean_service_2
        TOCA CAMBIARLA PARA QUE ESTE BIEN
    **/
    mean_service_2 = 0.5;


    /* Initialize the simulation. */
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

        switch (next_event_type)
        {
            case 1:
                arrive();
                break;
            case 2:
                depart_A1();
                break;
            case 3:
                depart_A2();
                break;
            case 4:
                depart_B();
                break;
            case 5:
                depart_B_A1();
                break;
            case 6:
                depart_B_A2();
                break;
        }
    }

    /* Invoke the report generator and end the simulation. */

    fclose(infile);
    //report();

    return 0;
}


void initialize(void)  /* Initialization function. */
{
    /* Initialize the simulation clock. */
    end_time = 10;
    stop_simulation = 0;
    sim_time = 0.0;

    /* Initialize the state variables. */
    server_status_A1 = IDLE;
    server_status_A2 = IDLE;
    server_status_B = IDLE;
    num_in_q_1 = 0;
    num_in_q_2 = 0;
    time_last_event = 0.0;
    num_units_attended = 0;

    /* Initialize the statistical counters. */

    num_custs_delayed_1  = 0;
    total_of_delays_1    = 0.0;
    area_num_in_q_1      = 0.0;

    num_custs_delayed_2  = 0;
    total_of_delays_2    = 0.0;
    area_num_in_q_2      = 0.0;

    area_server_status_A1_type1 = 0.0;
    area_server_status_A1_type2 = 0.0;

    area_server_status_A2_type1 = 0.0;
    area_server_status_A2_type2 = 0.0;

    area_server_status_B_type1 = 0.0;
    area_server_status_B_type2 = 0.0;

    /* Initialize event list.  Since no customers are present, the departure
       (service completion) event is eliminated from consideration. */

    time_next_event[1] = sim_time + expon(mean_interarrival);   // arrival
    time_next_event[2] = 1.0e+30;                               // depart_A1
    time_next_event[3] = 1.0e+30;                               // depart_A2
    time_next_event[4] = 1.0e+30;                               // depart_B
    time_next_event[5] = 1.0e+30;                               // depart_B_A1
    time_next_event[6] = 1.0e+30;                               // depart_B_A2

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




    switch (next_event_type)
        {
            case 1:
                printf("ARRIVAL %3.5f\n", time_next_event[next_event_type] );
                break;
            case 2:
                printf("A1 %3.5f\n", time_next_event[next_event_type] );
                break;
            case 3:
                printf("A2 %3.5f\n", time_next_event[next_event_type] );
                break;
            case 4:
                printf("B %3.5f\n", time_next_event[next_event_type] );
                break;
            case 5:
                printf("B A1 %3.5f\n", time_next_event[next_event_type] );
                break;
            case 6:
                printf("B A2 %3.5f\n", time_next_event[next_event_type] );
                break;
        }



}

void arrive(void)  /* Arrival event function. */
{
    float delay_1;
    float delay_2;

    /* Schedule next arrival. */

    time_next_event[1] = sim_time + expon(mean_interarrival);

    /* CHANGE log for something truly random.  */
    /* DETERMINE THE TYPE OF CLIENT */
    if (randomReal() <= probability_type1_client){
        printf("     type 1" );
        /*---------------------------------------------- type 1 client arrived -----------------------------------------*/
        if(is_there_any_server_idle()){
            delay_1            = 0.0;
            total_of_delays_1 += delay_1;

            /* Increment the number of customers delayed */
            ++num_custs_delayed_1;

            /*** EXPLICAR CAMBIO DE LOGICA ***/
            if (server_status_A1 == IDLE){
                    printf("     METIDO EN A1\n" );
                /* SERVER A1 BUSY*/
                server_status_A1 = BUSY;

                /* Schedule a departure from server A1 */
                time_next_event[2] = sim_time + expon(mean_service_1);

            } else if (server_status_A2 == IDLE) {
                printf("     METIDO EN A2\n" );
                /* SERVER A2 BUSY*/
                server_status_A2 = BUSY;

                /* Schedule a departure from server A2 */
                time_next_event[3] = sim_time + expon(mean_service_1);

            } else {
                printf("     METIDO EN AB\n" );
                /* SERVER B BUSY*/
                server_status_B = BUSY;

                /* Schedule a departure from server B */
                time_next_event[4] = sim_time + expon(mean_service_1);
            }
        } else {
            printf("     TOCO ESPERAR\n" );
            /* Servers are busy, so increment number of customers in queue type 1. */
            ++num_in_q_1;
            time_arrival_1[num_in_q_1] = sim_time;
        }
    } else {
        printf("     type 2" );
        /*---------------------------------------------- type 2 client arrived -----------------------------------------*/
        if (server_status_B == IDLE && is_there_any_serverA_idle()){
            delay_2            = 0.0;
            total_of_delays_2 += delay_2;

            /* Increment the number of customers delayed, and make servers busy. */
            ++num_custs_delayed_2;

            if (server_status_A1 == IDLE){
                    printf("     METIDO EN A1 Y B\n" );
                /* SERVER B and A1 BUSY*/
                server_status_B = BUSY;
                server_status_A1 = BUSY;

                /* Schedule a departure from server B A1. */
                time_next_event[5] = sim_time + expon(get_mean_service_2());
            } else {
                printf("     METIDO EN A2 Y B\n" );
                 /* SERVER B and A2 BUSY*/
                server_status_B = BUSY;
                server_status_A2 = BUSY;

                /* Schedule a departure from server B A2. */
                time_next_event[6] = sim_time + expon(get_mean_service_2());
            }
        } else {
            printf("     ESPERA\n" );
            /* Servers are busy, so increment number of customers in queue type 2. */
            ++num_in_q_2;
            time_arrival_2[num_in_q_2] = sim_time;
        }
    }
}

void depart_A1(void)  /* Departure event function. */
{
    /* Check to see whether the queue type 2 is empty OR server B  is BUSY*/
    if (num_in_q_2 == 0 || server_status_B == BUSY) {


        /* Check to see whether the queue type 1 is empty. */
        if (num_in_q_1 == 0) {
            /* The queue is empty so make the server idle and eliminate the
               departure (service completion) event from consideration. */
            server_status_A1     = IDLE;

            time_next_event[2] = 1.0e+30; // depart A1
        } else {
            /* Departure from A1 event */
            attend_queue_1(2);
        }
    } else {
        /* Departure from B A1  event */
        attend_queue_2(5);
        server_status_B = BUSY;

        /*** eliminate departure A1 event from consideration ***/
        time_next_event[2] = 1.0e+30;
    }
}

void depart_A2(void)  /* Departure event function. */
{
    /* Check to see whether the queue type 2 is empty OR server B  is BUSY*/
    if (num_in_q_2 == 0 || server_status_B == BUSY) {
        /* Check to see whether the queue type 1 is empty. */
        if (num_in_q_1 == 0) {
            /* The queue is empty so make the server idle and eliminate the
               departure (service completion) event from consideration. */
            server_status_A2     = IDLE;

            time_next_event[3] = 1.0e+30; // depart A2
        } else {
            /* Departure from A2 event */
            attend_queue_1(3);
        }
    } else {
        /* Departure from B A2  event */
        attend_queue_2(6);
        server_status_B = BUSY;

        /*** eliminate departure A2 event from consideration ***/
        time_next_event[3] = 1.0e+30;
    }
}


void depart_B(void) {
    /* Check to see if queue type 2 is not empty and there is at least one server A */
    if (num_in_q_2 != 0 && is_there_any_serverA_idle()){
        /* Service type 2 client */
        /*** eliminate departure B event from consideration ***/
        time_next_event[4] = 1.0e+30;

        if (server_status_A1 == IDLE){
            /* Departure from B A1  event */
            attend_queue_2(5);
        } else {
            /* Departure from B A2  event */
            attend_queue_2(6);
        }
    } else if (num_in_q_1 != 0){
        /* Service type 1 client */
        /* Departure from B event */
        attend_queue_1(4);
    } else {
        server_status_B = IDLE;
        /*** eliminate departure B event from consideration ***/
        time_next_event[4] = 1.0e+30;
    }

}

void depart_B_A1 (void){
     /* Check to see whether the queue type 2 is empty.*/
    if (num_in_q_2 == 0) {

        /*** eliminate departure B A1 event from consideration ***/
        time_next_event[5] = 1.0e+30; // depart B A1 event

        /* Check to see whether the queue type 1 is empty. */
        if (num_in_q_1 == 0) {
            /* The queue is empty so make the server idle and eliminate the
               departure (service completion) event from consideration. */
            server_status_A1     = IDLE;
            server_status_B      = IDLE;

            /*** eliminate departure A1 and B event from consideration ***/
            time_next_event[2] = 1.0e+30;
            time_next_event[4] = 1.0e+30;
        } else {
            /* Departure from A1 event */
            attend_queue_1(2);

            /* Check to see whether the queue type 1 is empty. */
            if (num_in_q_1 == 0){
                server_status_B      = IDLE;
                time_next_event[4] = 1.0e+30; // depart B event
            } else {
                /* Departure from B event */
                attend_queue_1(4);
            }
        }
    } else {
        /* Departure from B A1  event */
        attend_queue_2(5);

        /*** eliminate departure A1 event from consideration ***/
        time_next_event[2] = 1.0e+30;
    }
}


void depart_B_A2 (void){
     /* Check to see whether the queue type 2 is empty.*/
    if (num_in_q_2 == 0) {

        /*** eliminate departure B A2 event from consideration ***/
        time_next_event[6] = 1.0e+30; // depart B A1 event

        /* Check to see whether the queue type 1 is empty. */
        if (num_in_q_1 == 0) {
            /* The queue is empty so make the server idle and eliminate the
               departure (service completion) event from consideration. */
            server_status_A2     = IDLE;
            server_status_B      = IDLE;

            /*** eliminate departure A2 and B event from consideration ***/
            time_next_event[3] = 1.0e+30;
            time_next_event[4] = 1.0e+30;
        } else {
            /* Departure from A2 event */
            attend_queue_1(3);

            /* Check to see whether the queue type 1 is empty. */
            if (num_in_q_1 == 0){
                server_status_B      = IDLE;
                time_next_event[4] = 1.0e+30; // depart B event
            } else {
                /* Departure from B event */
                attend_queue_1(4);
            }
        }
    } else {
        /* Departure from B A2  event */
        attend_queue_2(6);

        /*** eliminate departure A2 event from consideration ***/
        time_next_event[3] = 1.0e+30;
    }
}

void report(void)  /* Report generator function. */
{
    /* Compute and write estimates of desired measures of performance. */
    printf("\n\n total of delays1 : %11.3f / num_cuts_delayed1: %11.3f\n", total_of_delays_1 , num_custs_delayed_1);
    printf( "\n\nAverage delay in queue%11.3f minutes\n\n",total_of_delays_1 / num_custs_delayed_1);

    printf("\n\n area num in q1 : %11.3f / sim time : %11.3f\n", area_num_in_q_1 ,  sim_time);
    printf( "Average number in queue 1 %10.3f\n\n",area_num_in_q_1 / sim_time);
    /*printf( "Server 1 utilization%15.3f\n\n",
            area_server_status_1 / sim_time);*/
    printf("\n\n total of delays 2: %11.3f / num_cuts_delayed2 : %11.3f\n", total_of_delays_2 , num_custs_delayed_2);
    printf( "\n\nAverage delay in queue%11.3f minutes\n\n", total_of_delays_2 / num_custs_delayed_2);

    printf("\n\n area num in q 2: %11.3f / sim time : %11.3f\n", area_num_in_q_2 ,  sim_time);
    printf( "Average number in queue 2 %10.3f\n\n", area_num_in_q_2 / sim_time);
    /*printf( "Server 2 utilization%15.3f\n\n",
            area_server_status_2 / sim_time);*/

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
    /*
    area_server_status_1 += server_status_1 * time_since_last_event;
    area_server_status_2 += server_status_2 * time_since_last_event;
    */
}

float expon(float mean)  /* Exponential variate generation function. */
{
    /* Return an exponential random variate with mean "mean". */
    return -mean * log(lcgrand(1));
}

float randomReal(void){
    return lcgrand(1);
}

void attend_queue_1 (int event_type){ // 2, 3 , 4
    int   i;
    float delay;

   /* The queue is nonempty, so decrement the number of customers in
       queue. */
    --num_in_q_1;

    /* Compute the delay of the customer who is beginning service and update
       the total delay accumulator. */

    delay            = sim_time - time_arrival_1[1];
    total_of_delays_1 += delay;

    /* Increment the number of customers delayed, and schedule departure. */

    ++num_custs_delayed_1;
    time_next_event[event_type] = sim_time + expon(mean_service_1);

    /* Move each customer in queue (if any) up one place. */

    for (i = 1; i <= num_in_q_1; ++i)
        time_arrival_1[i] = time_arrival_1[i + 1];

    printf("total of delays_1 : %11.3f / num_cuts_delayed _ 1:  %d\n", total_of_delays_1 , num_custs_delayed_1);
}

void attend_queue_2 (int event_type){
    int   i;
    float delay;



    /* The queue is nonempty, so decrement the number of customers in
       queue. */

    --num_in_q_2;

    /* Compute the delay of the customer who is beginning service and update
       the total delay accumulator. */

    delay            = sim_time - time_arrival_2[1];
    total_of_delays_2 += delay;

    /* Increment the number of customers delayed, and schedule departure. */

    ++num_custs_delayed_2;
    time_next_event[event_type] = sim_time + expon(get_mean_service_2());

    /* Move each customer in queue (if any) up one place. */

    for (i = 1; i <= num_in_q_2; ++i)
        time_arrival_2[i] = time_arrival_2[i + 1];
    printf("total of delays_2 : %11.3f / num_cuts_delayed _ 2: %d\n", total_of_delays_2 , num_custs_delayed_2);
}


bool is_there_any_server_idle(void){
    return server_status_A1 == IDLE || server_status_A2 == IDLE || server_status_B == IDLE;
}


bool is_there_any_serverA_idle(void){
    return server_status_A1 == IDLE || server_status_A2 == IDLE;
}

float get_mean_service_2(void){
    return mean_service_2;
}
