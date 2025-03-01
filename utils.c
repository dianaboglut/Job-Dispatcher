#include "utils.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>

int countPrimesNumbers(int n)
{
    int count = 0;

    bool *prime = malloc((n+1) * sizeof(bool));
    if(prime == NULL){
        perror("Could not allocate memory for prime!\n");
        exit(-1);
    }
    
    for(int i = 0; i <= n; i++){
        prime[i] = true;
    }

    prime[0] = false;   // 0 is not prime number
    prime[1] = false;   // 1 is not prime number

    // Eratosthenes
    for(int i = 2; i*i <= n; i++){
        if(prime[i] == true){
            for(int j = i*i; j <= n; j+=i){
                prime[j] = false;
            }
        }
    }

    for (int i = 2; i <= n; i++) {
        if (prime[i]) {
            count++;
        }
    }

#ifdef DEBUG
    printf("\n");
#endif

    free(prime);
    return count;
}

int countPrimeDivisors(int n)
{
    int count = 0;

    bool *prime = malloc((n+1) * sizeof(bool));
    if(prime == NULL){
        perror("Could not allocate memory for prime!\n");
        exit(-1);
    }
    
    for(int i = 0; i <= n; i++){
        prime[i] = true;
    }

    prime[0] = false;   // 0 is not prime number
    prime[1] = false;   // 1 is not prime number

    // Eratosthenes
    for(int i = 2; i*i <= n; i++){
        if(n%i == 0 && prime[i] == true){
            for(int j = i*i; j <= n; j+=i){
                prime[j] = false;
            }
        }
    }

    for (int i = 2; i <= n/2; i++) {
        if (prime[i] == true  && n%i == 0) {
            count++;
        }
    }

    if(prime[n] == true) count++;

#ifdef DEBUG
    printf("\n");
#endif

    free(prime);
    return count;
}

void swap(char *a, char *b) {
    char aux = *a;
    *a = *b;
    *b = aux;
}

static int resindex;
void anagrams_aux(char *str, int start, int end, char *result) {
    if(start == end) {
        strcpy(result + resindex, str);
        resindex += 9;
    }
    for(int i = start; i <= end; i ++) {
        swap(str + start, str + i);
        anagrams_aux(str, start + 1, end, result);
        swap(str + start, str + i);
    }
}

void anagrams(char *str, int start, int end, char *result) {
    resindex = 0;
    anagrams_aux(str, start, end, result);
}
