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
float mean_service_2_lower;
float mean_service_2_upper;

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
int type_in_server_A1;
float area_server_status_A1_type1;
float area_server_status_A1_type2;
float area_server_status_A1;

/* Server A2*/
int server_status_A2;
int type_in_server_A2;
float area_server_status_A2_type1;
float area_server_status_A2_type2;
float area_server_status_A2;

/*Server B*/
int server_status_B;
int type_in_server_B;
float area_server_status_B_type1;
float area_server_status_B_type2;
float area_server_status_B;


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
    outfile = fopen("mm1.out", "w");
    num_events = 6;

    /* Read input parameters. */
    fscanf(infile, "%f %f %f %f %f %f", &mean_interarrival, &probability_type1_client,
           &probability_type2_client, &mean_service_1, &mean_service_2_lower, &mean_service_2_upper);

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
    report();
    fclose(infile);
    fclose(outfile);
    return 0;
}

void initialize(void)  /* Initialization function. */
{
    /* Initialize the simulation clock. */
    end_time = 1000;
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
        /*---------------------------------------------- type 1 client arrived -----------------------------------------*/
        if(is_there_any_server_idle()){
            delay_1            = 0.0;
            total_of_delays_1 += delay_1;

            /* Increment the number of customers delayed */
            ++num_custs_delayed_1;

            /*** EXPLICAR CAMBIO DE LOGICA ***/
            if (server_status_A1 == IDLE){
                /* SERVER A1 BUSY*/
                server_status_A1 = BUSY;
                /* Schedule a departure from server A1 */

                time_next_event[2] = sim_time + expon(mean_service_1);
                type_in_server_A1 = 1;

            } else if (server_status_A2 == IDLE) {
                /* SERVER A2 BUSY*/
                server_status_A2 = BUSY;

                /* Schedule a departure from server A2 */
                time_next_event[3] = sim_time + expon(mean_service_1);
                type_in_server_A2 = 1;
            } else {
                /* SERVER B BUSY*/
                server_status_B = BUSY;

                /* Schedule a departure from server B */
                time_next_event[4] = sim_time + expon(mean_service_1);
                type_in_server_B = 1;
            }
        } else {
            /* Servers are busy, so increment number of customers in queue type 1. */
            ++num_in_q_1;
            time_arrival_1[num_in_q_1] = sim_time;
        }
    } else {

        /*---------------------------------------------- type 2 client arrived -----------------------------------------*/
        if (server_status_B == IDLE && is_there_any_serverA_idle()){
            delay_2            = 0.0;
            total_of_delays_2 += delay_2;

            /* Increment the number of customers delayed, and make servers busy. */
            ++num_custs_delayed_2;

            if (server_status_A1 == IDLE){
                /* SERVER B and A1 BUSY*/
                server_status_B = BUSY;
                server_status_A1 = BUSY;

                type_in_server_B = 2;
                type_in_server_A1 = 2;

                /* Schedule a departure from server B A1. */
                time_next_event[5] = sim_time + expon(get_mean_service_2());
            } else {
                 /* SERVER B and A2 BUSY*/
                server_status_B = BUSY;
                server_status_A2 = BUSY;

                type_in_server_B = 2;
                type_in_server_A2 = 2;

                /* Schedule a departure from server B A2. */
                time_next_event[6] = sim_time + expon(get_mean_service_2());
            }
        } else {
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
            type_in_server_A1 = 1;
        }
    } else {
        /* Departure from B A1  event */
        attend_queue_2(5);
        server_status_B = BUSY;

        type_in_server_A1 = 2;
        type_in_server_B = 2;

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
            type_in_server_A2 = 1;
        }
    } else {
        /* Departure from B A2  event */
        attend_queue_2(6);
        server_status_B = BUSY;
        type_in_server_A2 = 2;
        type_in_server_B = 2;

        /*** eliminate departure A2 event from consideration ***/
        time_next_event[3] /* Specify the number of events for the timing function. */= 1.0e+30;
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

            type_in_server_A1 = 2;
            type_in_server_B = 2;

        } else {
            /* Departure from B A2  event */
            attend_queue_2(6);

            type_in_server_A2 = 2;
            type_in_server_B = 2;
        }
    } else if (num_in_q_1 != 0){
        /* Service type 1 client */
        /* Departure from B event */
        attend_queue_1(4);

        type_in_server_B = 1;

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
            type_in_server_A1 = 1;

            /* Check to see whether the queue type 1 is empty. */
            if (num_in_q_1 == 0){
                server_status_B      = IDLE;
                time_next_event[4] = 1.0e+30; // depart B event
            } else {
                /* Departure from B event */
                attend_queue_1(4);

                type_in_server_B = 1;
            }
        }
    } else {
        /* Departure from B A1  event */
        attend_queue_2(5);
        type_in_server_A1 = 2;
        type_in_server_B = 2;

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

            type_in_server_A2 = 1;

            /* Check to see whether the queue type 1 is empty. */
            if (num_in_q_1 == 0){
                server_status_B      = IDLE;
                time_next_event[4] = 1.0e+30; // depart B event
            } else {
                /* Departure from B event */
                attend_queue_1(4);

                type_in_server_B = 1;
            }
        }
    } else {
        /* Departure from B A2  event */
        attend_queue_2(6);

        type_in_server_A2 = 2;
        type_in_server_B = 2;

        /*** eliminate departure A2 event from consideration ***/
        time_next_event[3] = 1.0e+30;
    }
}

void report(void)  /* Report generator function. */
{
    /* Compute and write estimates of desired measures of performance. */
    printf( "------ Contadores estadisticos de cola Q1 ------\n");
    printf( "Demora promedio de clientes en cola : %10.4f minutos\n",total_of_delays_1 / num_custs_delayed_1);
    printf( "Numero promedio de clientes en cola : %10.4f\n\n",area_num_in_q_1 / sim_time);

    printf( "------ Contadores estadisticos de cola Q2 ------\n");
    printf( "Demora promedio de clientes en cola : %10.4f minutos\n", total_of_delays_2 / num_custs_delayed_2);
    printf( "Numero promedio de clientes en cola : %10.4f\n\n\n", area_num_in_q_2 / sim_time);

    printf( "------ Contadores estadisticos de servidor A1 ------\n");
    printf("Utiliacion del servidor                     : %.5f\n", area_server_status_A1 / sim_time);
    printf("Proporcion tiempo dedicada a cliente tipo1  : %.4f\n", area_server_status_A1_type1 / area_server_status_A1);
    printf("Proporcion tiempo dedicada a cliente tipo2  : %.4f\n\n", area_server_status_A1_type2 / area_server_status_A1);

    printf( "------ Contadores estadisticos de servidor A2 ------\n");
    printf("Utiliacion del servidor                     : %.5f\n", area_server_status_A2 / sim_time);
    printf("Proporcion tiempo dedicada a cliente tipo 1 : %.4f\n", area_server_status_A2_type1 / area_server_status_A2);
    printf("Proporcion tiempo dedicada a cliente tipo 2 : %.4f\n\n", area_server_status_A2_type2 / area_server_status_A2);

    printf( "------ Contadores estadisticos de servidor B  ------\n");
    printf("Utiliacion del servidor                     : %.5f\n", area_server_status_B / sim_time);
    printf("Proporcion tiempo dedicada a cliente tipo 1 : %.4f\n", area_server_status_B_type1 / area_server_status_B);
    printf("Proporcion tiempo dedicada a cliente tipo 2 : %.4f\n\n", area_server_status_B_type2 / area_server_status_B);

    printf( "Termino del reloj de la simulacion%12.3f minutos\n\n", sim_time);


    fprintf( outfile, "------ Contadores estadisticos de cola Q1 ------\n");
    fprintf( outfile,"Demora promedio de clientes en cola : %10.4f minutos\n",total_of_delays_1 / num_custs_delayed_1);
    fprintf( outfile,"Numero promedio de clientes en cola : %10.4f\n\n",area_num_in_q_1 / sim_time);

    fprintf( outfile,"------ Contadores estadisticos de cola Q2 ------\n");
    fprintf(outfile,"Demora promedio de clientes en cola : %10.4f minutos\n", total_of_delays_2 / num_custs_delayed_2);
    fprintf( outfile,"Numero promedio de clientes en cola : %10.4f\n\n\n", area_num_in_q_2 / sim_time);

    fprintf( outfile,"------ Contadores estadisticos de servidor A1 ------\n");
    fprintf(outfile,"Utiliacion del servidor                     : %.5f\n", area_server_status_A1 / sim_time);
    fprintf(outfile,"Proporcion tiempo dedicada a cliente tipo1  : %.4f\n", area_server_status_A1_type1 / area_server_status_A1);
    fprintf(outfile,"Proporcion tiempo dedicada a cliente tipo2  : %.4f\n\n", area_server_status_A1_type2 / area_server_status_A1);

    fprintf( outfile,"------ Contadores estadisticos de servidor A2 ------\n");
    fprintf(outfile,"Utiliacion del servidor                     : %.5f\n", area_server_status_A2 / sim_time);
    fprintf(outfile,"Proporcion tiempo dedicada a cliente tipo 1 : %.4f\n", area_server_status_A2_type1 / area_server_status_A2);
    fprintf(outfile,"Proporcion tiempo dedicada a cliente tipo 2 : %.4f\n\n", area_server_status_A2_type2 / area_server_status_A2);

    fprintf(outfile, "------ Contadores estadisticos de servidor B  ------\n");
    fprintf(outfile,"Utiliacion del servidor                     : %.5f\n", area_server_status_B / sim_time);
    fprintf(outfile,"Proporcion tiempo dedicada a cliente tipo 1 : %.4f\n", area_server_status_B_type1 / area_server_status_B);
    fprintf(outfile,"Proporcion tiempo dedicada a cliente tipo 2 : %.4f\n\n", area_server_status_B_type2 / area_server_status_B);

    fprintf(outfile, "Termino del reloj de la simulacion%12.3f minutos\n\n", sim_time);

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

    /* Server A1*/
    area_server_status_A1 += server_status_A1 * time_since_last_event;
    area_server_status_A1_type1 += !(type_in_server_A1 - 1) * server_status_A1 * time_since_last_event;
    area_server_status_A1_type2 +=  (type_in_server_A1 - 1) * server_status_A1 * time_since_last_event;

    /* Server A2*/
    area_server_status_A2 += server_status_A2 * time_since_last_event;
    area_server_status_A2_type1 += !(type_in_server_A2 - 1) * server_status_A2 * time_since_last_event;
    area_server_status_A2_type2 +=  (type_in_server_A2 - 1) * server_status_A2 * time_since_last_event;

    /*Server B*/
    area_server_status_B += server_status_B * time_since_last_event;
    area_server_status_B_type1 += !(type_in_server_B - 1) * server_status_B * time_since_last_event;
    area_server_status_B_type2 +=  (type_in_server_B - 1) * server_status_B * time_since_last_event;


}

float expon(float mean)  /* Exponential variate generation function. */
{
    return -mean * log(lcgrand(1));
}

float randomReal(void){
    return lcgrand(lcgrand(1) );
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
   }


bool is_there_any_server_idle(void){
    return server_status_A1 == IDLE || server_status_A2 == IDLE || server_status_B == IDLE;
}


bool is_there_any_serverA_idle(void){
    return server_status_A1 == IDLE || server_status_A2 == IDLE;
}

float get_mean_service_2(void){
    return mean_service_2_lower + lcgrand(1) * (mean_service_2_upper - mean_service_2_lower);
}
