#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <mpi/mpi.h>

#include "utils.h"

#define MASTER 0
#define LINE_MAX 256
#define CLI_MAX 100
#define WK_MAX 8
#define PERM_SIZE 65536 // around 40k possible permutations at most
#define STR_SIZE 9 // anagrams receive max 8 chars
#define WK_FREE 0
#define WK_BUSY 1

FILE *file;
FILE *cli_file[CLI_MAX];

int task_id;
int num_proc;
int cli_id;
char cmd_str[LINE_MAX]; 
char cmd_prm[LINE_MAX]; // the command parameter
int wk_status[WK_MAX];  // Free, busy
double duration;

// Structure for the workload
struct cmd {
    int wk_id;  // the worker assigned to the command
    int cli_id;
    char cmd_type[LINE_MAX]; // Primes, Prime div, Anagrams
    char param[LINE_MAX];
};

struct res {
    int wk_id;
    int cli_id;
    char cmd_type[LINE_MAX];
    int res1;
    char res2[PERM_SIZE][STR_SIZE];
    // anagrams - (40k+ permutations) * (8 chars per string)
    // primes, primedivisors
};

void print_time(FILE *file) {
    time_t t = time(NULL);
    char time_str[64];
    struct tm *local_time = localtime(&t);
    strftime(time_str, sizeof(time_str), "%H:%M:%S", local_time);
    #ifdef DEBUG
    fprintf(file, "%s\n", time_str);
    #endif
}

int main(void)
{
    MPI_Init(NULL, NULL);

    MPI_Comm_rank(MPI_COMM_WORLD, &task_id);
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

    // Master Part
    if(task_id == MASTER){
        duration = omp_get_wtime();

        file = fopen("commands.txt","r");
        if(file == NULL) {
            perror("fopen() failed");
            MPI_Finalize();
            exit(EXIT_FAILURE);
        }

        char line[LINE_MAX];

        while (fgets(line,LINE_MAX,file) != NULL){
            if(strncmp(line,"WAIT",4) == 0){
                sleep(atoi(line + 4));
                continue;
            }

            // Reading from the file
            sscanf(line, "CLI%d %s %s", &cli_id, cmd_str, cmd_prm);
            print_time(stderr);
            #ifdef DEBUG
            fprintf(stderr, "Command received from client %d: %s(%s)\n", cli_id, cmd_str, cmd_prm);
            #endif

            if(cli_file[cli_id] == NULL) {
                char filename[64];
                sprintf(filename, "CLI%d.data", cli_id);
                cli_file[cli_id] = fopen(filename, "w+");
            }

            int all_busy = 1;
            for(int i = 1; i < num_proc; i ++) {
                if(wk_status[i] == WK_FREE) {
                    wk_status[i] = WK_BUSY;
                    struct cmd args;
                    args.wk_id = i;
                    args.cli_id = cli_id;
                    strcpy(args.cmd_type, cmd_str);
                    strcpy(args.param, cmd_prm);
                    MPI_Send(&args,sizeof(struct cmd), MPI_BYTE, i, 0, MPI_COMM_WORLD);
                    print_time(stderr);
                    #ifdef DEBUG
                    fprintf(stderr, "Command %s(%s) dispatched to worker %d\n", cmd_str, cmd_prm, i);
                    #endif
                    all_busy = 0;
                    break;
                }
            }

            if(all_busy) {
                struct res result = {0};
                MPI_Recv(&result, sizeof(struct res), MPI_BYTE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if(!strcmp(result.cmd_type, "PRIMES") || !strcmp(result.cmd_type, "PRIMEDIVISORS")) {
                    print_time(cli_file[result.cli_id]);
                    fprintf(cli_file[result.cli_id], "%s -> %d\n", result.cmd_type, result.res1);
                    print_time(stderr);
                    #ifdef DEBUG
                    fprintf(stderr, "Client %d result: %d by worker %d\n", result.cli_id, result.res1, result.wk_id);
                    #endif
                } else {
                    print_time(cli_file[result.cli_id]);
                    fprintf(cli_file[result.cli_id], "ANAGRAMS ->\n");
                    print_time(stderr);
                    #ifdef DEBUG
                    fprintf(stderr, "Client %d result by worker %d:\n", result.cli_id, result.wk_id);
                    #endif
                    for(int i = 0; i < PERM_SIZE; i ++) {
                        if(result.res2[i][0] == '\0') break;
                        fprintf(cli_file[result.cli_id], "%s\n", result.res2[i]);
                        // #ifdef DEBUG
                        // fprintf(stderr, "%s\n", result.res2[i]);
                        // #endif
                    }
                }
                
                // Afetr recv I continue to give commands to workers
                struct cmd args;
                args.wk_id = result.wk_id;
                args.cli_id = cli_id;
                strcpy(args.cmd_type, cmd_str);
                strcpy(args.param, cmd_prm);
                print_time(stderr);
                MPI_Send(&args,sizeof(struct cmd), MPI_BYTE, result.wk_id, 0, MPI_COMM_WORLD);
                #ifdef DEBUG
                fprintf(stderr, "Command %s(%s) dispatched to worker %d\n", cmd_str, cmd_prm, result.wk_id);
                #endif
            }
        }

        // At the end I have to wait for all the workers,
        // without those that had never been busy
        for(int i = 1; i < num_proc; i ++) {
            if(wk_status[i] == WK_FREE) continue;
            struct res result = {0};
            MPI_Recv(&result, sizeof(struct res), MPI_BYTE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if(!strcmp(result.cmd_type, "PRIMES") || !strcmp(result.cmd_type, "PRIMEDIVISORS")) {
                print_time(cli_file[result.cli_id]);
                fprintf(cli_file[result.cli_id], "%s -> %d\n", result.cmd_type, result.res1);
                print_time(stderr);
                #ifdef DEBUG
                fprintf(stderr, "Client %d result: %d by worker %d\n", result.cli_id, result.res1, result.wk_id);
                #endif
            } else {
                print_time(cli_file[result.cli_id]);
                fprintf(cli_file[result.cli_id], "ANAGRAMS ->\n");
                print_time(stderr);
                #ifdef DEBUG
                fprintf(stderr, "Client %d result by worker %d:\n", result.cli_id, result.wk_id);
                #endif
                for(int i = 0; i < PERM_SIZE; i ++) {
                    if(result.res2[i][0] == '\0') break;
                    fprintf(cli_file[result.cli_id], "%s\n", result.res2[i]);
                    // #ifdef DEBUG
                    // fprintf(stderr, "%d. %s\n", i, result.res2[i]);
                    // #endif
                }
            }
        }

        fclose(file);

        // Close all workers
        struct cmd args = {0};
        args.cli_id = -1;
        for(int i = 1; i < num_proc; i ++) {
            MPI_Send(&args, sizeof(struct cmd), MPI_BYTE, i, 0, MPI_COMM_WORLD);
        }

        duration = omp_get_wtime() - duration;

        FILE *performance = fopen("performance", "a");
        if(performance == NULL) {
            perror("fopen() failed");
            MPI_Finalize();
            exit(EXIT_FAILURE);
        }

        fprintf(stderr, "On %d workers, duration: %lf\n", num_proc, duration);
        fprintf(performance, "On %d workers, duration: %lf\n", num_proc, duration);
    }
    // Worker
    else{
        while (1) {
            struct cmd args = {0};
            struct res result = {0};

            MPI_Recv(&args, sizeof(struct cmd), MPI_BYTE, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            if(args.cli_id == -1) break;

            result.cli_id = args.cli_id;
            result.wk_id = args.wk_id;
            strcpy(result.cmd_type, args.cmd_type);

            if(!strcmp(args.cmd_type, "PRIMES")) {
                result.res1 = countPrimesNumbers(atoi(args.param));
            } else if(!strcmp(args.cmd_type, "PRIMEDIVISORS")) {
                result.res1 = countPrimeDivisors(atoi(args.param));
            } else {
                anagrams(args.param, 0, strlen(args.param) - 1, result.res2);
            }

            MPI_Send(&result, sizeof(struct res), MPI_BYTE, MASTER, 0, MPI_COMM_WORLD);
        }
    }

    #ifdef DEBUG
    fprintf(stderr, "Worker %d closing...\n", task_id);
    #endif
    MPI_Finalize();
}
