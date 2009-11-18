#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "libc_helper.h"

void safe_pipe(int pipefd[2]) {
  if (pipe(pipefd) == -1) {
    perror("[fatal] pipe() failed");
    exit(1);
  }
}
