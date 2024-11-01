#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

int* child_pids;
int pnum = -1;

void kill_children(int signum) {
  printf("Timeout reached! Killing all children process.\n");
  for (int i = 0; i < pnum; i++)
    kill(child_pids[i], SIGKILL);
  free(child_pids);
  exit(1);
}

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  //int pnum = -1;
  int timeout = 0;
  bool with_files = false;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"timeout", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            // your code here
            if (seed <= 0) {
              printf("Seed must be a positive number\n");
              return 1;
            }
            // error handling
            break;
          case 1:
            array_size = atoi(optarg);
            // your code here
            if (array_size <= 0) {
              printf("Array size must be a positive number\n");
              return 1;
            }
            // error handling
            break;
          case 2:
            pnum = atoi(optarg);
            // your code here
            if (pnum <= 0) {
              printf("Number of processes must be a positive number\n");
              return 1;
            }
            // error handling
            break;
          case 3:
            timeout = atoi(optarg);
            if (timeout < 0) {
              printf("Timeout must be a non-negative number\n");
              return 1;
            }
          case 4:
            with_files = true;
            break;

          defalut:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" --timeout \"num\"\n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  int active_child_processes = 0;

  int pipefd[pnum][2];
  for (int i = 0; i < pnum; i++) {
    if (pipe(pipefd[i]) == -1) {
      perror("pipe failed");
      return 1;
    }
  }

  child_pids = (int*)malloc(sizeof(int) * pnum);
  int segment_size = array_size / pnum;

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  if (timeout > 0) {
    signal(SIGALRM, kill_children);
    alarm(timeout);
  }

  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // successful fork
      active_child_processes += 1;
      if (child_pid == 0) {
        // child process
        unsigned int begin = i * segment_size;
        unsigned int end = (i == pnum - 1) ? array_size - 1 : (begin + segment_size - 1);
        struct MinMax min_max = GetMinMax(array, begin, end);
        // parallel somehow

        if (with_files) {
          // use files here
          char filename[256];
          sprintf(filename, "tmp_%d.txt", i);
          FILE *file = fopen(filename, "w");
          if (file == NULL) {
            printf("Failed to open file\n");
            return 1;
          }
          fprintf(file, "%d %d\n", min_max.min, min_max.max);
          fclose(file);
        } else {
          // use pipe here
          close(pipefd[i][0]);
          write(pipefd[i][1], &min_max, sizeof(struct MinMax));
          close(pipefd[i][1]);
          exit(0);
        }
        return 0;
      }
      else {
        child_pids[i] = child_pid;
      }

    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

  while (active_child_processes > 0) {
    // your code here
    wait(NULL);
    active_child_processes -= 1;
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      // read from files
      char filename[256];
      sprintf(filename, "tmp_%d.txt", i);
      FILE *file = fopen(filename, "r");
      if (file == NULL) {
        printf("Failed to open file\n");
        return 1;
      }
      fscanf(file, "%d %d", &min, &max);
      fclose(file);
      remove(filename);
    } else {
      // read from pipes
      struct MinMax local_min_max;
      close(pipefd[i][1]);
      read(pipefd[i][0], &local_min_max, sizeof(struct MinMax));
      min = local_min_max.min;
      max = local_min_max.max;
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);
  free(child_pids);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}
