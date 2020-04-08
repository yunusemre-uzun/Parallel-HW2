#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

#define SIMULATION_TIME 10

int simulate_game(int processor_rank) {
    usleep(SIMULATION_TIME); // Simulation takes time
    srand(time(NULL) + processor_rank);
    return (rand()%2); // HOME_WIN or AWAY_WIN
}

int main(int argc, char **argv){
    uint process_rank, number_of_processors;
    unsigned long int number_of_teams = pow(2,atoi(argv[1]));
    unsigned long int number_of_teams_per_processor, i;
    unsigned long int* teams_array = malloc(sizeof(unsigned long int)*number_of_teams);
    double start_time, end_time;
    for(i=0;i<number_of_teams;i++) {
        teams_array[i] = i;
    }
    MPI_Status status; 
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);
    // Get the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);
    //Get the number of processors in the environment
    MPI_Comm_size(MPI_COMM_WORLD, &number_of_processors);
    // Number of teams is halved in each iteration
    unsigned long int index, leg=1;
    // The array to store all winner teams of the leg 
    unsigned long int* global_winner_teams = NULL;
    while(number_of_teams > 1) {
        // If it is the root processor, then allocate memory
        if (process_rank == 0) {
            // If it is first leg
            if (leg == 1) {
                global_winner_teams = malloc(sizeof(unsigned long int)*number_of_teams);
                //Copy the original team array to global winners in first leg
                for(index = 0; index<number_of_teams; index++) {
                    global_winner_teams[index] = teams_array[index];
                }
                start_time = MPI_Wtime(); //Start time
            }
        }
        // Calculate number of teams per processor in each iteration
        number_of_teams_per_processor = number_of_teams/number_of_processors;
        // The array with size of number_of_teams/number_of_processors at first leg
        unsigned long int* teams_to_be_processed = malloc(sizeof(unsigned long int)*number_of_teams_per_processor);

        // Scatter all of the array to the processors with size(global_winner_teams)/number_of_processors
        MPI_Scatter(global_winner_teams, number_of_teams_per_processor, MPI_UNSIGNED_LONG, teams_to_be_processed,
            number_of_teams_per_processor, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);
        
        if (process_rank == 0) {
            // If it is not the first leg
            if (leg > 1)
                global_winner_teams = realloc(global_winner_teams,sizeof(unsigned long int)*number_of_teams);
            } else {
                global_winner_teams = NULL;
        }
        // Kill the proccesses when needed
        if(number_of_teams<process_rank)
            break;

        // To store every processors winners in local
        unsigned long int* local_winner_teams = malloc(sizeof(unsigned long int)*number_of_teams_per_processor/2);
        // Simulate the tournament in the subarray
        for(index = 0 ; index<number_of_teams_per_processor;index+=2){
            unsigned long int winner = simulate_game(process_rank);
            //printf("%d \n ", winner);
            local_winner_teams[index/2] = teams_to_be_processed[index+winner];
        }

        // Gather all local winners from local_winner_teams to the global_winner_teams
        MPI_Gather(local_winner_teams, number_of_teams_per_processor/2, MPI_UNSIGNED_LONG, global_winner_teams,
            number_of_teams_per_processor/2, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);

        free(local_winner_teams);
        number_of_teams /= 2;
        leg++;
    }
    // There is only 1 teams left 
    if(process_rank == 0) {
        unsigned long int winner = global_winner_teams[0];
        printf("%ld\n", winner);
        free(global_winner_teams);
        end_time = MPI_Wtime();
        printf("%f\n", end_time-start_time);
    }
    MPI_Finalize();
    free(teams_array);
    return 0;
}