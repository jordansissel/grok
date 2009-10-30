#include "grok.h"
#include "grok_program.h"
#include "grok_input.h"
#include "grok_matchconf.h"

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <event.h>
#include <signal.h>
#include <fcntl.h>

static void *_event_init = NULL;
void _collection_sigchld(int sig, short what, void *data);

grok_collection_t *grok_collection_init() {
  grok_collection_t *gcol;
  gcol = calloc(1, sizeof(grok_collection_t));
  gcol->nprograms = 0;
  gcol->program_size = 10;
  gcol->programs = calloc(gcol->program_size, sizeof(grok_program_t));
  gcol->ebase = event_init();
  gcol->exit_code = 0;

  gcol->ev_sigchld = malloc(sizeof(struct event));
  signal_set(gcol->ev_sigchld, SIGCHLD, _collection_sigchld, gcol);
  signal_add(gcol->ev_sigchld, NULL);

  return gcol;
}

void grok_collection_check_end_state(grok_collection_t *gcol) {
  int still_alive = 0;
  int p, i, m;
  int reaction_count = 0;
  for (p = 0; p < gcol->nprograms; p++) {
    grok_program_t *gprog = gcol->programs[p];
    reaction_count += gprog->reactions;
    for (i = 0; i < gprog->ninputs; i++) {
      grok_input_t *ginput = &gprog->inputs[i];
      still_alive += (ginput->done == 0);
    }
    for (m = 0; m < gprog->nmatchconfigs; m++) {
      grok_matchconf_t *gmc = &gprog->matchconfigs[m];
      still_alive += (gmc->pid != 0);
    }
  }

  if (still_alive == 0) {
    struct timeval nodelay = { 0, 0 };
    grok_log(gcol, LOG_PROGRAM, 
             "No more subprocesses are running. Breaking event loop now.");

    /* Cleanup */
    grok_matchconfig_global_cleanup();
    event_base_loopexit(gcol->ebase, &nodelay);
    if (reaction_count == 0) {
      gcol->exit_code = 1;
    }
  }
}

void grok_collection_add(grok_collection_t *gcol, grok_program_t *gprog) {
  int i = 0;
  grok_log(gcol, LOG_PROGRAM, "Adding %d inputs", gprog->ninputs);

  for (i = 0; i < gprog->ninputs; i++) {
    grok_log(gprog, LOG_PROGRAM, "Adding input %d", i);
    gprog->inputs[i].gprog = gprog;
    grok_program_add_input(gprog, gprog->inputs + i);
  }

  gcol->nprograms++;
  if (gcol->nprograms == gcol->program_size) {
    gcol->program_size *= 2;
    gcol->programs = realloc(gcol->programs,
                             gcol->program_size * sizeof(grok_collection_t *));
  }

  gcol->programs[gcol->nprograms - 1] = gprog;
  gprog->gcol = gcol;
}

void _collection_sigchld(int sig, short what, void *data) {
  grok_collection_t *gcol = (grok_collection_t*)data;
  struct timeval nodelay = { 0, 0 };

  int i = 0;
  int prognum;
  int pid, status;

  grok_log(gcol, LOG_PROGRAM, "SIGCHLD received");
  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    grok_log(gcol, LOG_PROGRAM, "Found dead child pid %d", pid);
    for (prognum = 0; prognum < gcol->nprograms; prognum++) {
      grok_program_t *gprog = gcol->programs[prognum];

      /* we found a dead child. Look for an input_process it belongs to,
       * then see if we should restart it
       */
      for (i = 0; i < gprog->nmatchconfigs; i++) {
        grok_matchconf_t *gmc = (gprog->matchconfigs + i);
        if (gmc->pid != pid)
          continue;

        grok_log(gcol, LOG_PROGRAM, "Pid %d is a matchconf shell", pid);
        gmc->pid = 0;
      } 

      for (i = 0; i < gprog->ninputs; i++) {
        grok_input_t *ginput = (gprog->inputs + i);
        if (ginput->type != I_PROCESS)
          continue;

        grok_input_process_t *gipt = &(ginput->source.process);
        if (gipt->pid != pid)
          continue;

        grok_log(gcol, LOG_PROGRAM, "Pid %d is an exec process", pid);

        /* use ginput's log values */
        grok_log(ginput, LOG_PROGRAM, "Reaped child pid %d. Was process '%s'",
                 pid, gipt->cmd);

        if (PROCESS_SHOULD_RESTART(gipt)) {
          /* Calculate the restart delay */
          /* XXX: This code should go in the eof handler */
          struct timeval restart_delay = { 0, 0 };
          if (gipt->run_interval > 0) {
            struct timeval interval = { gipt->run_interval, 0 };
            struct timeval duration;
            struct timeval now;
            gettimeofday(&now, NULL);
            timersub(&now, &(gipt->start_time), &duration);
            timersub(&interval, &duration, &restart_delay);
          }

          if (gipt->min_restart_delay > 0) {
            struct timeval fixed_delay = { gipt->min_restart_delay, 0 };

            if (timercmp(&restart_delay, &fixed_delay, <)) {
              restart_delay.tv_sec = fixed_delay.tv_sec;
              restart_delay.tv_usec = fixed_delay.tv_usec;
            }
          }

          grok_log(ginput, LOG_PROGRAM, 
                   "Scheduling process restart in %d.%d seconds: %s",
                   restart_delay.tv_sec, restart_delay.tv_usec, gipt->cmd);
          ginput->restart_delay.tv_sec = restart_delay.tv_sec;
          ginput->restart_delay.tv_usec = restart_delay.tv_usec;
        } else {
          grok_log(gprog, LOG_PROGRAM, "Not restarting process '%s'", gipt->cmd);
        }

        event_once(-1, EV_TIMEOUT, grok_input_eof_handler, ginput, &nodelay);
      } /* end for looping over gprog's inputs */
    } /* end for looping over gcol's programs */
  } /* while waitpid */

  grok_collection_check_end_state(gcol);
}

void grok_collection_loop(grok_collection_t *gcol) {
  event_base_dispatch(gcol->ebase);
}
