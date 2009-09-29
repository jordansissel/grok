#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <event.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#include "grok.h"
#include "grok_program.h"
#include "grok_input.h"
#include "grok_matchconf.h"
#include "grok_logging.h"

#include "libc_helper.h"

void _program_process_stdout_read(struct bufferevent *bev, void *data);
void _program_process_start(int fd, short what, void *data);
void _program_process_buferror(struct bufferevent *bev, short what,
                               void *data);
void _program_file_repair_event(int fd, short what, void *data);


void _program_file_read_buffer(struct bufferevent *bev, void *data);
void _program_file_read_real(int fd, short what, void *data);
void _program_file_buferror(struct bufferevent *bev, short what, void *data);

void grok_program_add_input(grok_program_t *gprog, grok_input_t *ginput) {
  grok_log(gprog, LOG_PROGRAM, "Adding input of type %s",
         (ginput->type == I_FILE) ? "file" : "process");

  ginput->instance_match_count = 0;
  ginput->done = 0;
  switch (ginput->type) {
    case I_FILE:
      grok_program_add_input_file(gprog, ginput);
      break;
    case I_PROCESS:
      grok_program_add_input_process(gprog, ginput);
      break;
  }
}

void grok_program_add_input_process(grok_program_t *gprog,
                                    grok_input_t *ginput) {
  struct bufferevent *bev;
  grok_input_process_t *gipt = &(ginput->source.process);
  int childin[2], childout[2], childerr[2];
  int pid;
  struct timeval now = { 0, 0 };

  safe_pipe(childin);
  safe_pipe(childout);
  safe_pipe(childerr);

  gipt->p_stdin = childin[1];
  gipt->p_stdout = childout[0];
  gipt->p_stderr = childerr[0];
  gipt->c_stdin = childin[0];
  gipt->c_stdout = childout[1];
  gipt->c_stderr = childerr[1];

  bev = bufferevent_new(gipt->p_stdout, _program_process_stdout_read,
                        NULL, _program_process_buferror, ginput);
  bufferevent_enable(bev, EV_READ);
  ginput->bev = bev;

  if (gipt->read_stderr) {
    /* store this somewhere */
    bev = bufferevent_new(gipt->p_stderr, _program_process_stdout_read,
                          NULL, _program_process_buferror, ginput);
    bufferevent_enable(bev, EV_READ);
  }

  grok_log(ginput, LOG_PROGRAMINPUT, "Scheduling start of: %s", gipt->cmd);
  event_once(-1, EV_TIMEOUT, _program_process_start, ginput, &now);
}

void grok_program_add_input_file(grok_program_t *gprog,
                                 grok_input_t *ginput) {
  struct bufferevent *bev;
  struct stat st;
  int ret;
  int pipefd[2];
  grok_input_file_t *gift = &(ginput->source.file);
  grok_log(ginput, LOG_PROGRAMINPUT, "Adding file input: %s", gift->filename);

  ret = stat(gift->filename, &st);
  if (ret == -1) {
    grok_log(gprog, LOG_PROGRAMINPUT , "Failure stat(2)'ing file: %s",
             gift->filename);
    grok_log(gprog, LOG_PROGRAMINPUT , "strerror(%d): %s", strerror(errno));
    return;
  }
  gift->fd = open(gift->filename, O_RDONLY);

  if (gift->fd < 0) {
    grok_log(gprog, LOG_PROGRAM, "Failure open(2)'ing file for read '%s': %s",
             gift->filename, strerror(errno));
    return;
  }

  safe_pipe(pipefd);
  gift->offset = 0;
  gift->reader = pipefd[0];
  gift->writer = pipefd[1];
  memcpy(&(gift->st), &st, sizeof(st));
  gift->waittime.tv_sec = 0;
  gift->waittime.tv_usec = 0;
  gift->readbuffer = malloc(st.st_blksize);

  grok_log(ginput, LOG_PROGRAMINPUT, "dup2(%d, %d)", gift->fd, gift->writer);

  /* Tie our open file read fd to the writer of our pipe */
  // this doesn't work

  bev = bufferevent_new(gift->reader, _program_file_read_buffer,
                        NULL, _program_file_buferror, ginput);
  bufferevent_enable(bev, EV_READ);
  ginput->bev = bev;
  event_once(-1, EV_TIMEOUT, _program_file_read_real, ginput,
             &(gift->waittime));
}


void _program_process_stdout_read(struct bufferevent *bev, void *data) {
  grok_input_t *ginput = (grok_input_t *)data;
  grok_program_t *gprog = ginput->gprog;
  char *line;
  int ret;

  while ((line = evbuffer_readline(EVBUFFER_INPUT(bev))) != NULL) {
    grok_matchconfig_exec(gprog, ginput, line);
    free(line);
  }
}

void _program_process_buferror(struct bufferevent *bev, short what,
                               void *data) {
  grok_input_t *ginput = (grok_input_t *)data;
  grok_input_process_t *gipt = &(ginput->source.process);
  grok_program_t *gprog = ginput->gprog;
  grok_log(ginput, LOG_PROGRAMINPUT, "Buffer error %d on process %d: %s",
           what, gipt->pid, gipt->cmd);
}

void _program_process_start(int fd, short what, void *data) {
  grok_input_t *ginput = (grok_input_t*)data;
  grok_input_process_t *gipt = &(ginput->source.process);
  grok_program_t *gprog = ginput->gprog;
  int pid = 0;

  /* reset the 'instance match count' since we're starting the process */
  ginput->instance_match_count = 0;

  /* start the process */
  pid = fork();
  if (pid != 0) {
    gipt->pid = pid;
    gipt->pgid = getpgid(pid);
    gettimeofday(&(gipt->start_time), NULL);
    grok_log(ginput, LOG_PROGRAMINPUT,
             "Starting process: '%s' (%d)", gipt->cmd, getpid());
    return;
  }
  dup2(gipt->c_stdin, 0);
  dup2(gipt->c_stdout, 1);
  if (gipt->read_stderr) {
    dup2(gipt->c_stderr, 2);
  }
  execlp("sh", "sh", "-c", gipt->cmd, NULL);
  grok_log(ginput, LOG_PROGRAM, 
           "execlp(2) returned unexpectedly. Is 'sh' in your path?");
  grok_log(ginput, LOG_PROGRAM, "execlp: %s", strerror(errno));
  exit(-1); /* in case execlp fails */
}

void _program_file_read_buffer(struct bufferevent *bev, void *data) {
  grok_input_t *ginput = (grok_input_t *)data;
  grok_program_t *gprog = ginput->gprog;
  char *line;
  int i;

  while ((line = evbuffer_readline(EVBUFFER_INPUT(bev))) != NULL) {
    grok_matchconfig_exec(gprog, ginput, line);
    free(line);
  }
}

void _program_file_buferror(struct bufferevent *bev, short what,
                            void *data) {
  grok_input_t *ginput = (grok_input_t *)data;
  grok_input_file_t *gift = &(ginput->source.file);
  grok_program_t *gprog = ginput->gprog;
  struct timeval nodelay = { 0, 0 };
  grok_log(ginput, LOG_PROGRAMINPUT, "Buffer error %d on file %d: %s",
           what, gift->fd, gift->filename);

  if (what & EVBUFFER_EOF) {
    /* EOF erro on a file, which means libevent forgets about it.
     * let's re-add it */
    grok_log(ginput, LOG_PROGRAMINPUT, 
             "EOF Error on file buffer for '%s'. Ignoring.", gift->filename);
    ginput->restart_delay.tv_sec = gift->waittime.tv_sec;
    ginput->restart_delay.tv_usec = gift->waittime.tv_usec;
    event_once(0, EV_TIMEOUT, grok_input_eof_handler, ginput, &nodelay);
  //} else if (what & EVBUFFER_TIMEOUT) {
    ///* Timeout reading from our file buffer */
    //ginput->restart_delay.tv_sec = gift->waittime.tv_sec;
    //ginput->restart_delay.tv_usec = gift->waittime.tv_usec;
    //bufferevent_enable(ginput->bev, EV_READ);
  }
}

void _program_file_repair_event(int fd, short what, void *data) {
  grok_input_t *ginput = (grok_input_t *)data;
  grok_input_file_t *gift = &(ginput->source.file);
  struct bufferevent *bev = ginput->bev;
  struct stat st;

  if (stat(gift->filename, &st) != 0) {
    grok_log(ginput, LOG_PROGRAM, "Failure stat(2)'ing file '%s': %s",
             gift->filename, strerror(errno));
    grok_log(ginput, LOG_PROGRAM, 
             "Unrecoverable error (stat failed). Can't continue watching '%s'",
             gift->filename);
    return;
  }

  if (gift->st.st_ino != st.st_ino) {
    /* inode changed, reopen file */
    grok_log(ginput, LOG_PROGRAMINPUT, 
             "File inode changed from %d to %d. Reopening file '%s'",
             gift->st.st_ino, st.st_ino, gift->filename);
    close(gift->fd);
    gift->fd = open(gift->filename, O_RDONLY);
    gift->waittime.tv_sec = 0;
    gift->waittime.tv_usec = 0;
    gift->offset = 0;
  } else if (st.st_size < gift->st.st_size) {
    /* File size shrank */
    grok_log(ginput, LOG_PROGRAMINPUT, 
             "File size shrank from %d to %d. Seeking to beginning of file '%s'",
             gift->st.st_size, st.st_size, gift->filename);
    gift->offset = 0;
    lseek(gift->fd, gift->offset, SEEK_SET);
    gift->waittime.tv_sec = 0;
    gift->waittime.tv_usec = 0;
  } else {
    /* Nothing changed, we should wait */
    if (gift->waittime.tv_sec == 0) {
      gift->waittime.tv_sec = 1;
    } else {
      gift->waittime.tv_sec *= 2;
      if (gift->waittime.tv_sec > 60) {
        gift->waittime.tv_sec = 60;
      }
    }
  }

  memcpy(&(gift->st), &st, sizeof(st));

  grok_log(ginput, LOG_PROGRAMINPUT, 
           "Repairing event with fd %d file '%s'. Will read again in %d.%d secs",
           bev->ev_read.ev_fd, gift->filename,
           gift->waittime.tv_sec, gift->waittime.tv_usec);

  //event_add(&bev->ev_read, &(gift->waittime));
  event_once(0, EV_TIMEOUT, _program_file_read_real, ginput, &(gift->waittime));
}

void _program_file_read_real(int fd, short what, void *data) {
  grok_input_t *ginput = (grok_input_t *)data;
  grok_input_file_t *gift = &(ginput->source.file);
  grok_program_t *gprog = ginput->gprog;

  int write_ret;
  int bytes = 0;
  bytes = read(gift->fd, gift->readbuffer, gift->st.st_blksize);
  write_ret = write(gift->writer, gift->readbuffer, bytes);

  if (write_ret == -1) {
    grok_log(ginput, LOG_PROGRAMINPUT,
             "fatal write() to pipe fd %d of %d bytes: %s",
             gift->writer, bytes, strerror(errno));
    /* XXX: Maybe just shutdown this particular process/file instead
     * of exiting */
    exit(1);
  }
  gift->offset += bytes;

  /* we can potentially read past our last 'filesize' if the file
   * has been updated since stat()'ing it. */
  if (gift->offset > gift->st.st_size)
    gift->st.st_size = gift->offset;

  grok_log(ginput, LOG_PROGRAMINPUT, "%s: read %d bytes", gift->filename, bytes);

  if (bytes == 0) { /* nothing to read, at EOF */
    grok_input_eof_handler(0, 0, ginput);
  } else if (bytes < 0) {
    grok_log(ginput, LOG_PROGRAMINPUT, "Error: Bytes read < 0: %d", bytes);
    grok_log(ginput, LOG_PROGRAMINPUT, "Error: strerror() says: %s",
             strerror(errno));
  } else {
    /* We read more than 0 bytes, so we should keep reading this file
     * immediately */

    gift->waittime.tv_sec = 0;
    gift->waittime.tv_usec = 0;
    event_once(0, EV_TIMEOUT, _program_file_read_real, ginput,
               &(gift->waittime));
  }
}

void grok_input_eof_handler(int fd, short what, void *data) {
  grok_input_t *ginput = (grok_input_t *)data;
  grok_program_t *gprog = ginput->gprog;

  if (ginput->instance_match_count == 0) {
    /* execute nomatch if there is one on this program */
    grok_matchconfig_exec_nomatch(gprog, ginput);
  }

  switch (ginput->type) {
    case I_PROCESS:
      if (PROCESS_SHOULD_RESTART(&(ginput->source.process))) {
        ginput->instance_match_count = 0;
        event_once(-1, EV_TIMEOUT, _program_process_start, ginput,
                   &ginput->restart_delay);
      } else {
        grok_log(ginput->gprog, LOG_PROGRAM, "Not restarting process: %s",
                 ginput->source.process.cmd);
        bufferevent_disable(ginput->bev, EV_READ);
        close(ginput->source.process.p_stdin);
        close(ginput->source.process.p_stdout);
        close(ginput->source.process.p_stderr);
        ginput->done = 1;
      }
      break;
    case I_FILE:
      if (ginput->source.file.follow) {
        ginput->instance_match_count = 0;
        event_once(-1, EV_TIMEOUT, _program_file_repair_event, ginput,
                   &ginput->restart_delay);
      } else {
        grok_log(ginput->gprog, LOG_PROGRAM, "Not restarting file: %s",
                 ginput->source.file.filename);
        bufferevent_disable(ginput->bev, EV_READ);
        close(ginput->source.file.reader);
        close(ginput->source.file.writer);
        close(ginput->source.file.fd);
        ginput->done = 1;
      }
      break;
  }

  /* If all inputs are now done, close the shell */
  int still_open = 0;
  int i = 0;
  
  for (i = 0; i < gprog->ninputs; i++) {
    still_open += !gprog->inputs[i].done;
    if (!gprog->inputs[i].done) {
      grok_log(gprog, LOG_PROGRAM, "Input still open: %d", i);
    }
  }

  if (still_open == 0) {
    for (i = 0; i < gprog->nmatchconfigs; i++) {
      grok_matchconfig_close(gprog, &gprog->matchconfigs[i]);
    }
    grok_collection_check_end_state(gprog->gcol);
  }
}

