#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int findStart(int p, int n, int i)
{
    int rest = n%p;

    if (rest == 0) {
        return i*(n/p);
    }
    else{
        return i*((n-rest)/p);
    }
}
int findStop(int p, int n, int i)
{
    if (i == p-1) {
        return findStart(p, n, i+1)+ n%p;
    } else{
        return findStart(p, n, i+1)-1;
    }
}


int main(int argc, char* argv[]){

    int numThread = atoi(argv[1]);
    int arrsize = atoi(argv[2]);
    int numBlocks = atoi(argv[3]);
    int arr[arrsize];
    
    for(int i = 0; i < arrsize; i++){
        arr[i]= rand(); 
    }

    for (size_t i = 0; i < numBlocks; i++)
    {
        printf("start = %d ,stop = %d\n", findStart(numBlocks, arrsize, i), findStop(numBlocks, arrsize, i));
    }
}
