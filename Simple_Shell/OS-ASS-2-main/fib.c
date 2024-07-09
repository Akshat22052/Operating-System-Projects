#include <stdio.h>
#include <stdlib.h> 

long int fib(int n) {
    if (n == 0 || n == 1) {
        return 1;
    }
    return fib(n - 1) + fib(n - 2);
}

int main(int argc, char *argv[]) {
    for (int i=0;i<argc;i++) {
        printf("%s\n",argv[i]);
    }
    int n = atoi(argv[1]); 

    

    long int t = fib(n);
    printf("Fibonacci(%d) = %ld\n", n, t);

    return 0; 
}


