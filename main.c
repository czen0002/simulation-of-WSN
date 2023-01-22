#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mpi.h"
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include "encription.c"
#include "event.c"

#define ITERATION 1000

bool event(int *array);
void encrypt(int* num);
void decrypt(int* num);
void encrypt_array(int* num_array, int length);
void decrypt_array(int* num_array, int length);



int main(int argc, char** argv) 
{ 
    int world_rank, world_size, slave_rank, slave_size;
    MPI_Comm new_comm;
    MPI_Comm slave_comm;
    MPI_Status statuses[4];
    int up,down,right,left;
    int random_num;
    int numbers[4];
    int dim[2], period[2], reorder;
    int nodes[4];
    int i, j, k;
    // The first 9 items in event_info is:
    // number of message passing, number of events, encryption time, decryption time,
    // node number, left node number, right node number, up node number, down node number
    // If an event occurs, 4 randome number received from adjacent node, event year, event month
    // event day, event hour, event minute, event second
    int event_info[10*ITERATION+9];
    int index = 4;
    bool result = 0;
    int event_times = 0;
    int num_message = 0;
    // variables to store date and time components
	int hours, minutes, seconds, day, month, year;
	// time_t is arithmetic time type
	time_t now;
    // encryption time
    double start_encrypt, end_encrypt, time_encrypt, sum_time_encrypt = 0, time_encrypt_event;
    // decryption time
    double start_decrypt, end_decrypt, time_decrypt, sum_time_decrypt = 0, time_decrypt_event;
    // communication time among slave nodes
    double start_comm, end_comm, total_comm = 0;
    int total_comm_time;
    // communicate with base node time
    double start_pass, end_pass;
    // result file 
    FILE * file;
   
    

    MPI_Init(&argc, &argv);
    // Get world rank and world size
    // Split 21
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    // Check if world size is 21
    if (world_size != 21) MPI_Abort( MPI_COMM_WORLD , 1 );
    // Split 21 process into master and slave
    MPI_Comm_split(MPI_COMM_WORLD, world_rank == 20, world_rank, &slave_comm);
    // Get slave rank and slave size
    MPI_Comm_rank(slave_comm, &slave_rank);
    MPI_Comm_size(slave_comm, &slave_size);

    // Create 4*5 grids
    if (world_rank != 20) {
        dim[0]=4; dim[1]=5;
        period[0]=0; period[1]=0;
        reorder=1;
        MPI_Cart_create(slave_comm, 2, dim, period, reorder, &new_comm);
    }

    // Each node gets its adjacent nodes world rank
    // If an adjacent node does not exist, its value is -2 
    if (world_rank != 20) {
        MPI_Cart_shift(new_comm, 0, 1, &up, &down);
        MPI_Cart_shift(new_comm, 1, 1, &left, &right);
    }

    // set up adjacent nodes array
    nodes[0] = left;
    nodes[1] = right;
    nodes[2] = up;
    nodes[3] = down;

    // store node information in event information array
    if (world_rank !=  20) {
        event_info[index++] = world_rank;
        if (left != -2) {
            event_info[index++] = left;
        }
        if (right != -2) {
            event_info[index++] = right;
        }
        if (up != -2) {
            event_info[index++] = up;
        }
        if (down != -2) {
            event_info[index++] = down;
        }
    }


    for (i = 0; i < ITERATION; i++) {
        // set up an array stores received random numbers, default value is -1
        for (int j = 0; j < 4; j++) {
            numbers[j] = -1;
        }

        // generate a random number for each node
        if (world_rank != 20) {
            srandom(time(NULL)+slave_rank+i);
            int min = 1;
            int max = 12;
            random_num = (random ()%(max - min + 1)) + min;
        }

        // Encrypt random number and get encryption time
        start_encrypt = MPI_Wtime();
        encrypt(&random_num);
        end_encrypt = MPI_Wtime();
        time_encrypt = end_encrypt - start_encrypt;
        sum_time_encrypt += time_encrypt;

        // node 0 only receive from and send to right node and down node
        if (world_rank == 0) {
            start_comm = MPI_Wtime();
            MPI_Sendrecv(&random_num, 1, MPI_INT, right, 0, numbers+1, 1, MPI_INT, right, 1, new_comm, statuses+1);
            num_message += 1;
            MPI_Sendrecv(&random_num, 1, MPI_INT, down, 2, numbers+3, 1, MPI_INT, down, 3, new_comm, statuses+3);
            num_message += 1;
            end_comm = MPI_Wtime();
            total_comm += (end_comm - start_comm);
        }

        // node 4 only receive from and send to left node and down node
        if (world_rank == 4) {
            start_comm = MPI_Wtime();
            MPI_Sendrecv(&random_num, 1, MPI_INT, left, 1, numbers, 1, MPI_INT, left, 0, new_comm, statuses);
            num_message += 1;
            MPI_Sendrecv(&random_num, 1, MPI_INT, down, 2, numbers+3, 1, MPI_INT, down, 3, new_comm, statuses+3);
            num_message += 1;
            end_comm = MPI_Wtime();
            total_comm += (end_comm - start_comm);
        }

        // node 15 only receive from and send to right node and up node
        if (world_rank == 15) {
            start_comm = MPI_Wtime();
            MPI_Sendrecv(&random_num, 1, MPI_INT, right, 0, numbers+1, 1, MPI_INT, right, 1, new_comm, statuses+1);
            num_message += 1;
            MPI_Sendrecv(&random_num, 1, MPI_INT, up, 3, numbers+2, 1, MPI_INT, up, 2, new_comm, statuses+2);
            num_message += 1;
            end_comm = MPI_Wtime();
            total_comm += (end_comm - start_comm);
        }

        // node 19 only receive from and send to left node and up node
        if (world_rank == 19) {
            start_comm = MPI_Wtime();
            MPI_Sendrecv(&random_num, 1, MPI_INT, left, 1, numbers, 1, MPI_INT, left, 0, new_comm, statuses);
            num_message += 1;
            MPI_Sendrecv(&random_num, 1, MPI_INT, up, 3, numbers+2, 1, MPI_INT, up, 2, new_comm, statuses+2);
            num_message += 1;
            end_comm = MPI_Wtime();
            total_comm += (end_comm - start_comm);
        }

        // node 1,2,3 only receive from and send to left node, right node and down node
        if (world_rank == 1 || world_rank == 2 || world_rank == 3) {
            start_comm = MPI_Wtime();
            MPI_Sendrecv(&random_num, 1, MPI_INT, left, 1, numbers, 1, MPI_INT, left, 0, new_comm, statuses);
            num_message += 1;
            MPI_Sendrecv(&random_num, 1, MPI_INT, right, 0, numbers+1, 1, MPI_INT, right, 1, new_comm, statuses+1);
            num_message += 1;
            MPI_Sendrecv(&random_num, 1, MPI_INT, down, 2, numbers+3, 1, MPI_INT, down, 3, new_comm, statuses+3);
            num_message += 1;
            end_comm = MPI_Wtime();
            total_comm += (end_comm - start_comm);
        }

        // node 16,17,18 only receive from and send to left node, right node and up node
        if (world_rank == 16 || world_rank == 17 || world_rank == 18) {
            start_comm = MPI_Wtime();
            MPI_Sendrecv(&random_num, 1, MPI_INT, left, 1, numbers, 1, MPI_INT, left, 0, new_comm, statuses);
            num_message += 1;
            MPI_Sendrecv(&random_num, 1, MPI_INT, right, 0, numbers+1, 1, MPI_INT, right, 1, new_comm, statuses+1);
            num_message += 1;
            MPI_Sendrecv(&random_num, 1, MPI_INT, up, 3, numbers+2, 1, MPI_INT, up, 2, new_comm, statuses+2);
            num_message += 1;
            end_comm = MPI_Wtime();
            total_comm += (end_comm - start_comm);
        }

        // node 5,10 only receive from and send to right node, up node and down node
        if (world_rank == 5 || world_rank == 10) {
            start_comm = MPI_Wtime();
            MPI_Sendrecv(&random_num, 1, MPI_INT, right, 0, numbers+1, 1, MPI_INT, right, 1, new_comm, statuses+1);
            num_message += 1;
            MPI_Sendrecv(&random_num, 1, MPI_INT, up, 3, numbers+2, 1, MPI_INT, up, 2, new_comm, statuses+2);
            num_message += 1;
            MPI_Sendrecv(&random_num, 1, MPI_INT, down, 2, numbers+3, 1, MPI_INT, down, 3, new_comm, statuses+3);
            num_message += 1;
            end_comm = MPI_Wtime();
            total_comm += (end_comm - start_comm);
        }

        // node 9,14 only receive from and send to left node, up node and down node
        if (world_rank == 9 || world_rank == 14) {
            start_comm = MPI_Wtime();
            MPI_Sendrecv(&random_num, 1, MPI_INT, left, 1, numbers, 1, MPI_INT, left, 0, new_comm, statuses);
            num_message += 1;
            MPI_Sendrecv(&random_num, 1, MPI_INT, up, 3, numbers+2, 1, MPI_INT, up, 2, new_comm, statuses+2);
            num_message += 1;
            MPI_Sendrecv(&random_num, 1, MPI_INT, down, 2, numbers+3, 1, MPI_INT, down, 3, new_comm, statuses+3);
            num_message += 1;
            end_comm = MPI_Wtime();
            total_comm += (end_comm - start_comm);
        }

        // node 6,7,8,11,12,14 receive from and sent to all four adjacent nodes
        if (world_rank == 6 || world_rank == 7 || world_rank == 8 || world_rank == 11 || world_rank == 12 || world_rank == 13) {
            start_comm = MPI_Wtime();
            MPI_Sendrecv(&random_num, 1, MPI_INT, left, 1, numbers, 1, MPI_INT, left, 0, new_comm, statuses);
            num_message += 1;
            MPI_Sendrecv(&random_num, 1, MPI_INT, right, 0, numbers+1, 1, MPI_INT, right, 1, new_comm, statuses+1);
            num_message += 1;
            MPI_Sendrecv(&random_num, 1, MPI_INT, up, 3, numbers+2, 1, MPI_INT, up, 2, new_comm, statuses+2);
            num_message += 1;
            MPI_Sendrecv(&random_num, 1, MPI_INT, down, 2, numbers+3, 1, MPI_INT, down, 3, new_comm, statuses+3);
            num_message += 1;
            end_comm = MPI_Wtime();
            total_comm += (end_comm - start_comm);
        }

        // calculate decryption time per node per iteration
        start_decrypt = MPI_Wtime();
        decrypt_array(numbers, 4);
        end_decrypt = MPI_Wtime();
        time_decrypt = end_decrypt - start_decrypt;
        sum_time_decrypt += time_decrypt;

        if (event(numbers)) {
            // the code of getting time is cited from https://www.techiedelight.com/print-current-date-and-time-in-c/
            // Obtain current time
            time(&now);
            struct tm *local = localtime(&now);

            // get hours, minutes and seconds
            hours = local->tm_hour;      	
            minutes = local->tm_min;     	
            seconds = local->tm_sec;     	

             // get the year, month and day
            day = local->tm_mday;        	
            month = local->tm_mon + 1;   	
            year = local->tm_year + 1900;

            // incerement event times
            event_times += 1;

            // stores event occurs date and time
            event_info[index++] = year;
            event_info[index++] = month;
            event_info[index++] = day;
            event_info[index++] = hours;
            event_info[index++] = minutes;
            event_info[index++] = seconds;

            // stores received numbers 
            if (numbers[0] != -1) {
                event_info[index++] = numbers[0];
            }
            if (numbers[1] != -1) {
                event_info[index++] = numbers[1];
            }
            if (numbers[2] != -1) {
                event_info[index++] = numbers[2];
            }
            if (numbers[3] != -1) {
                event_info[index++] = numbers[3];
            }
            
        }
    }

    //--------------------------------------
    //out of iteration loop
    if (world_rank != 20) {
        sum_time_encrypt = (int)ceil(sum_time_encrypt*1000000);
        sum_time_decrypt = (int)ceil(sum_time_decrypt*1000000);
        total_comm_time = (int)ceil(total_comm*1000000);

        event_info[0] = num_message;
        event_info[1] = event_times;
        event_info[2] = sum_time_encrypt;
        event_info[3] = sum_time_decrypt;

        // node that only has two adjacent nodes
        // encrypt first five elements
        if (world_rank == 0 || world_rank == 4 || world_rank == 15 || world_rank == 19) {
            start_encrypt = MPI_Wtime();
            encrypt_array(event_info, 5);
            encrypt(&event_times);
            encrypt(&total_comm_time);
            end_encrypt = MPI_Wtime();
            time_encrypt_event = end_encrypt - start_encrypt;

            MPI_Send(&time_encrypt_event, 1, MPI_DOUBLE, 20, world_rank, MPI_COMM_WORLD);
            MPI_Send(&total_comm_time, 1, MPI_INT, 20, world_rank, MPI_COMM_WORLD);
            MPI_Send(&event_times, 1, MPI_INT, 20, world_rank, MPI_COMM_WORLD);
            MPI_Send(event_info, 5, MPI_INT, 20, world_rank, MPI_COMM_WORLD);
           
        }
        // node that only has three adjacent nodes
        // encrypt first eight elements, then encrypt every event information(received three random numbers and date and time)
        else if (world_rank == 1 || world_rank == 2 || world_rank == 3 || world_rank == 5 ||
        world_rank == 9 || world_rank == 10 || world_rank == 14 || world_rank == 16 ||
        world_rank == 17 || world_rank == 18) {
            start_encrypt = MPI_Wtime();
            encrypt_array(event_info, event_times * 9 + 8);
            encrypt(&event_times);
            encrypt(&total_comm_time);
            end_encrypt = MPI_Wtime();
            time_encrypt_event = end_encrypt - start_encrypt;

            MPI_Send(&time_encrypt_event, 1, MPI_DOUBLE, 20, world_rank, MPI_COMM_WORLD);
            MPI_Send(&total_comm_time, 1, MPI_INT, 20, world_rank, MPI_COMM_WORLD);
            MPI_Send(&event_times, 1, MPI_INT, 20, world_rank, MPI_COMM_WORLD);
            decrypt(&event_times);
            MPI_Send(event_info, event_times * 9 + 8, MPI_INT, 20, world_rank, MPI_COMM_WORLD);
        }
        // node that has four adjacent nodes
        // encrypt first nine elements, then encrypt every event information(received four random numbers and date and time)
        else if (world_rank == 6 || world_rank == 7 || world_rank == 8 || world_rank == 11 ||
        world_rank == 12 || world_rank == 13) {
            start_encrypt = MPI_Wtime();
            encrypt_array(event_info, event_times * 10 + 9);
            encrypt(&event_times);
            encrypt(&total_comm_time);
            end_encrypt = MPI_Wtime();
            time_encrypt_event = end_encrypt - start_encrypt;
           
            MPI_Send(&time_encrypt_event, 1, MPI_DOUBLE, 20, world_rank, MPI_COMM_WORLD);
            MPI_Send(&total_comm_time, 1, MPI_INT, 20, world_rank, MPI_COMM_WORLD);
            MPI_Send(&event_times, 1, MPI_INT, 20, world_rank, MPI_COMM_WORLD);
            decrypt(&event_times);
            MPI_Send(event_info, event_times * 10 + 9, MPI_INT, 20, world_rank, MPI_COMM_WORLD);
        }
    }

    if (world_rank == 20) {
        // store event times of each node
        int base_event_times[20];
        // record the total event times
        int total_event_times = 0;
        // store communication time of each node 
        int node_comm_time[20];
        // record total event array length
        int base_event_length = 0;
        // record number of message base station received
        int base_num_message = 0;
        // record communication time between base station and nodes
        double base_comm_time = 0;
        // sotre the time of encrypting an array each node sends to base station
        double encrypt_event_time_array[20];
        double base_encrypt_time = 0;
        double base_decrypt_time = 0;
        int offset = 0;
        
        start_pass = MPI_Wtime();
        for (k = 0; k < 20; k++) {
            if (k == 0 || k == 4 || k == 15 || k == 19) {
                MPI_Recv(encrypt_event_time_array+k, 1, MPI_DOUBLE, k, k, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                base_num_message += 1;
                MPI_Recv(node_comm_time+k, 1, MPI_INT, k, k, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                base_num_message += 1;
                MPI_Recv(base_event_times+k, 1, MPI_INT, k, k, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                base_num_message += 1;
                base_event_length += 5;
            } else if (k == 1 || k == 2 || k == 3 || k == 5 || k == 9 || k == 10 || k == 14 || k == 16 ||
            k == 17 || k == 18) {
                MPI_Recv(encrypt_event_time_array+k, 1, MPI_DOUBLE, k, k, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                base_num_message += 1;
                MPI_Recv(node_comm_time+k, 1, MPI_INT, k, k, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                base_num_message += 1;
                MPI_Recv(base_event_times+k, 1, MPI_INT, k, k, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                base_num_message += 1;
                base_event_length = base_event_length + base_event_times[k] * 9 + 8; 
            } else if (k == 6 || k == 7 || k == 8 || k == 11 || k == 12 || k == 13) {    
                MPI_Recv(encrypt_event_time_array+k, 1, MPI_DOUBLE, k, k, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                base_num_message += 1;
                MPI_Recv(node_comm_time+k, 1, MPI_INT, k, k, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                base_num_message += 1;
                MPI_Recv(base_event_times+k, 1, MPI_INT, k, k, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                base_num_message += 1;
                base_event_length = base_event_length + base_event_times[k] * 10 + 9; 
            }
        }
        end_pass = MPI_Wtime();

        // get communication time between base satation and nodes
        base_comm_time += (end_pass - start_pass);

        // decrypt base_event_tims and record decrypt time
        start_decrypt = MPI_Wtime();
        decrypt_array(base_event_times, k);
        decrypt_array(node_comm_time, k);
        end_decrypt = MPI_Wtime();
        base_decrypt_time += (end_decrypt - start_decrypt);

        // calculate the total number of events
        for (k = 0; k < 20; k++) {
            total_event_times += base_event_times[k];
        }

        // calculate the total time encrypt 20 arrays sent to base station
        for (k = 0; k < 20; k++) {
            base_encrypt_time += encrypt_event_time_array[k];
        }

        // creat an array to store all information received from nodes
        int base_event_info[base_event_length];
        start_pass = MPI_Wtime();
        for (k = 0; k < 20; k++) {
            if (k == 0 || k == 4 || k == 15 || k == 19) {
                MPI_Recv(base_event_info+offset, 5, MPI_INT, k, k, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                base_num_message += 1;
                offset += 5;
            } 
            else if (k == 1 || k == 2 || k == 3 || k == 5 || k == 9 || k == 10 || k == 14 || k == 16 ||
            k == 17 || k == 18) {
                MPI_Recv(base_event_info+offset, base_event_times[k] * 9 + 8, MPI_INT, k, k, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                base_num_message += 1;
                offset += (base_event_times[k] * 9 + 8);
            } 
            else if (k == 6 || k == 7 || k == 8 || k == 11 || k == 12 || k == 13) {    
                MPI_Recv(base_event_info+offset, base_event_times[k] * 10 + 9, MPI_INT, k, k, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                base_num_message += 1;
                offset += (base_event_times[k] * 10 + 9);
            }

        }
        end_pass = MPI_Wtime();
        base_comm_time += (end_pass - start_pass);

        start_decrypt = MPI_Wtime();
        decrypt_array(base_event_info, base_event_length);
        end_decrypt = MPI_Wtime();
        base_decrypt_time += end_decrypt - start_decrypt;

        file = fopen("./result.txt", "w");

        int total_encryption_time;
        int total_decryption_time;

        if(file == NULL) {
            printf("Error!");   
            exit(1);             
        }
        fprintf(file, "The communication time between node and base node is %f seconds\n", base_comm_time);
        fprintf(file, "The number of message base node received is %d\n", base_num_message);
        fprintf(file, "The total time of encrypting messages sent to base station is %d microseconds\n",(int)ceil(base_encrypt_time*1000000));
        fprintf(file, "The total time of decrypting messages sent to base station is %d microseconds\n",(int)ceil(base_decrypt_time*1000000));
        fprintf(file, "The total number of event occurrence is %d\n", total_event_times);
        fprintf(file, "--------------------------------------\n"); 

        offset = 0;
        for (k = 0; k < 20; k++) {
            int adjnode1;
            int adjnode2;
            int adjnode3;
            int adjnode4;
            int times;
            if (k == 0 || k == 4 || k == 15 || k == 19) {
                fprintf(file, "Node %d:\n", base_event_info[offset+4]);
                fprintf(file, "The total communication time with adjacent nodes is:%d microseconds\n", node_comm_time[k]);
                fprintf(file, "The number of passing message is:%d\n", base_event_info[offset]);
                fprintf(file, "The encryption message time is:%d microseconds\n", base_event_info[offset+2]);
                fprintf(file, "The decryption message time is:%d microseconds\n", base_event_info[offset+3]);
                fprintf(file, "The number of event is:%d\n", base_event_info[offset+1]); 
                fprintf(file, "--------------------------------------\n"); 
                total_encryption_time += base_event_info[offset+2];
                total_decryption_time += base_event_info[offset+3];
                offset += 5;
            } else if (k == 1 || k == 2 || k == 3 || k == 5 || k == 9 || k == 10 || k == 14 || k == 16 ||
            k == 17 || k == 18) {
                fprintf(file, "Node %d:\n", base_event_info[offset+4]);
                fprintf(file, "The total communication time with adjacent nodes is:%d microseconds\n", node_comm_time[k]);
                fprintf(file, "The number of passing message is:%d\n", base_event_info[offset]);
                fprintf(file, "The encryption message time is:%d microseconds\n", base_event_info[offset+2]);
                fprintf(file, "The decryption message time is:%d microseconds\n", base_event_info[offset+3]);
                fprintf(file, "The number of event is:%d\n", base_event_info[offset+1]);
                total_encryption_time += base_event_info[offset+2];
                total_decryption_time += base_event_info[offset+3];
                adjnode1 = base_event_info[offset+5];
                adjnode2 = base_event_info[offset+6];
                adjnode3 = base_event_info[offset+7];
                offset += 8;
                if (base_event_times[k]>0) {
                    times = base_event_times[k];
                    while (times > 0) {
                        fprintf(file, "Event occurs time is:%d-%d-%d %d:%d:%d\n", base_event_info[offset], base_event_info[offset+1], base_event_info[offset+2], base_event_info[offset+3], base_event_info[offset+4], base_event_info[offset+5]);
                        fprintf(file, "Adjacent nodes %d sends:%d\n", adjnode1, base_event_info[offset+6]);
                        fprintf(file, "Adjacent nodes %d sends:%d\n", adjnode2, base_event_info[offset+7]);
                        fprintf(file, "Adjacent nodes %d sends:%d\n", adjnode3, base_event_info[offset+8]);
                        times -= 1;
                        offset += 9;
                    } 
                }
                fprintf(file, "--------------------------------------\n");
            } else if (k == 6 || k == 7 || k == 8 || k == 11 || k == 12 || k == 13) {    
                fprintf(file, "Node %d:\n", base_event_info[offset+4]);
                fprintf(file, "The total communication time with adjacent nodes is:%d microseconds\n", node_comm_time[k]);
                fprintf(file, "The number of passing message is%d:\n", base_event_info[offset]);
                fprintf(file, "The encryption message time is:%d microseconds\n", base_event_info[offset+2]);
                fprintf(file, "The decryption message time is:%d microseconds\n", base_event_info[offset+3]);
                fprintf(file, "The number of event is:%d\n", base_event_info[offset+1]);
                total_encryption_time += base_event_info[offset+2];
                total_decryption_time += base_event_info[offset+3];
                adjnode1 = base_event_info[offset+5];
                adjnode2 = base_event_info[offset+6];
                adjnode3 = base_event_info[offset+7];
                adjnode4 = base_event_info[offset+8];
                offset += 9;
                if (base_event_times[k]>0) {
                    times = base_event_times[k];
                    while (times > 0) {
                        fprintf(file, "Event occurs time is:%d-%d-%d %d:%d:%d\n", base_event_info[offset], base_event_info[offset+1], base_event_info[offset+2], base_event_info[offset+3], base_event_info[offset+4], base_event_info[offset+5]);
                        fprintf(file, "Adjacent nodes %d sends:%d\n", adjnode1, base_event_info[offset+6]);
                        fprintf(file, "Adjacent nodes %d sends:%d\n", adjnode2, base_event_info[offset+7]);
                        fprintf(file, "Adjacent nodes %d sends:%d\n", adjnode3, base_event_info[offset+8]);
                        fprintf(file, "Adjacent nodes %d sends:%d\n", adjnode4, base_event_info[offset+9]);
                        times -= 1;
                        offset += 10;
                    } 
                }
                fprintf(file, "--------------------------------------\n");
            }

        }

        fclose(file);   
    }

    MPI_Finalize();

	return 0; 
} 

