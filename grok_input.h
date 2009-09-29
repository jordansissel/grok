#ifndef _GROK_INPUT_H_
#define _GROK_INPUT_H_

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <event.h>

#include "grok_program.h"

struct grok_program;
typedef struct grok_input grok_input_t;
typedef struct grok_input_process grok_input_process_t;
typedef struct grok_input_file grok_input_file_t;

#define PROCESS_SHOULD_RESTART(gipt) ((gipt)->restart_on_death || (gipt)->run_interval)

struct grok_input_process {
  char *cmd;
  int cmdlen;
  
  /* State information */
  int p_stdin; /* parent descriptors */
  int p_stdout;
  int p_stderr;
  int c_stdin; /* child descriptors */
  int c_stdout;
  int c_stderr;
  int pid;
  int pgid;
  struct timeval start_time;

  /* Specific options */
  int restart_on_death;
  int min_restart_delay;
  int run_interval;
  int read_stderr;
};

struct grok_input_file {
  char *filename;

  /* State information */
  struct stat st;
  char *readbuffer; /* will be initialized to the blocksize reported by stat(2) */
  off_t offset; /* what position in the file are we in? */
  int writer; /* read data from file and write to here */
  int reader; /* point libevent eventbuffer here */
  int fd; /* the fd from open(2) */
  struct timeval waittime;

  /* Options */
  int follow;
};

struct grok_input {
  enum { I_FILE, I_PROCESS } type;
  union {
    grok_input_file_t file;
    grok_input_process_t process;
  } source;
  struct grok_program *gprog; /* pointer back to our program */

  struct bufferevent *bev;
  int instance_match_count;
  int logmask;
  int logdepth;
  struct timeval restart_delay;
  int done;
};

void grok_program_add_input(struct grok_program *gprog, grok_input_t *ginput);
void grok_program_add_input_process(struct grok_program *gprog, grok_input_t *ginput);
void grok_program_add_input_file(struct grok_program *gprog, grok_input_t *ginput);
void grok_input_eof_handler(int fd, short what, void *data);

#endif /* _GROK_INPUT_H_ */
