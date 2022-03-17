#include <string>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <mutex>


int main(int argc, char *argv[]) {
               for (int j = 0; j < argc; j++)
                   printf("argv[%d]: %s\n", j, argv[j]);

               exit(EXIT_SUCCESS);
}


