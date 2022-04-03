#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>
#include <sys/poll.h>
#include <pthread.h>
#include <getopt.h>

#include "util.h"
#include "main.h"

static int fd_control_pipe =-1;
static FILE* fp_log_file =0;

FILE* fp_control =0;

/**
 * atoBitPrnString
 *  - from acsii 8 bit value to 8-char string
 *  - fro led driver
 *
 * @param led_dat 8 bit value
 * @param led_string 8-char string
 */
void atoBitPrnString(char led_dt, char* led_string)
{
  int i,j;

  for (i=7, j=0; 0<=i; --i, ++j)
  {
    if (led_dt & (1<<i))
      *(led_string+j) = '*';
    else
      *(led_string+j) = '_';
  }
  *(led_string+j) = '\0';
}

/**
 * getstring
 *  - a little utilized fgets
 *
 * @param buf pointer to the buffer to retrieve string
 * @param maxchar size fo buf
 * @return same as buf
 */
char* getstring(char buf[], size_t maxchar)
{
  memset(buf,'\0',maxchar);
  if(fgets(buf,maxchar,stdin))//adds null
  {
    char *p = strchr(buf,'\n');
    if(p) *p = '\0';
    else
    {
      char tmp[32];
      while(fgets(tmp,sizeof(tmp),stdin))//adds null
      {
        if(strchr(tmp,'\n')) break;
      }
    }
  }
  return buf;
}

/**
 * dbgprint_hdr
 *  - make a header string for pretty print debug out
 *
 * @param funcname function name string as __FUNCTION__
 * @param buf buffer to retrieve header string
 * @param len size of buf
 * @return same as buf
 */
char* dbgprint_hdr(const char* funcname ,char* buf, size_t len)
{
  time_t now =time(NULL);
  struct tm result;
  localtime_r(&now,&result);

  memset(buf,'\0',len);

  strftime(buf,len,"[%Y%m%d %T] ",&result);
  len -=strlen(buf);

  sprintf(&buf[strlen(buf)],"%05d:%08lx:%s - " ,sid ,pthread_self() ,funcname);

  return buf;
}

/**
 * dbgprint_ftr
 *  - make a footer string for pretty print debug out
 *
 * @param funcname function name string as __FUNCTION__
 * @param buf buffer to retrieve header string
 * @param len size of buf
 * @return same as buf
 */
char* dbgprint_ftr(const char* funcname ,char* buf, size_t len)
{
  buf[0] ='\0';

  if (errno == 0)
    return buf;

  sprintf(buf," [current errno=%d:%s]",errno,strerror(errno));
    //TODO: Is there anything like strerror_r?

  errno =0;
  return buf;
}

#if 0
// under sh4-linux-gcc 3.4, vfprintf not work well...
void dbgprint(const char* funcname ,char *fmt, ...)
{
  time_t now =time(NULL);
  struct tm result;
  localtime_r(&now,&result);

  char buf[256];
  memset(buf,'\0',sizeof(buf));
  strftime(buf,sizeof(buf),"%Y%m%d %T",&result);

  va_list ap;
  va_start(ap, fmt);
  fprintf(stderr,"[%s] %05d:%08lx:%s - " ,buf ,sid ,pthread_self() ,funcname);
  vfprintf(stderr,fmt,ap);
  fprintf(stderr,"\n");
  va_end(ap);

  fflush(stderr);
}
#endif

/**
 * create_log_file
 *  - create and open daemon log file
 *
 * @param dir working directory to store file
 * @param basename basename of log file.
 * @return 0 succuss
 * @return errno failed
 */
int create_log_file(char *dir ,char *basename)
{
#define SUCCESS_TO_OPEN_LOG (0)
#define FAILED_TO_OPEN_LOG  (errno)

  assert(basename);

  static char *def_dir =".";
  if (!dir)
    dir =def_dir;

  char path[PATH_MAX];
  sprintf(path,"%s/%s.log" ,dir ,basename);

  if ((fp_log_file =fopen(path ,"a")) == NULL)
    return FAILED_TO_OPEN_LOG;

  return SUCCESS_TO_OPEN_LOG;

#undef SUCCESS_TO_OPEN_LOG
#undef FAILED_TO_OPEN_LOG
}

/**
 * switch_stderr_to_logfile
 *  - close stderr and dup to logfile
 *
 * @return 0 succuss
 * @return errno failed
 */
int switch_stderr_to_logfile()
{
#define SUCCESS_TO_SWITCH (0)
#define FAILED_TO_SWITCH  (errno)

  close(STDERR_FILENO);

  if (dup(fileno(fp_log_file)) == -1)
    return FAILED_TO_SWITCH;

  return SUCCESS_TO_SWITCH;
#undef SUCCESS_TO_SWITCH
#undef FAILED_TO_SWITCH
}

/**
 * switch_stdout_to_logfile
 *  - close stdout and dup to logfile
 *
 * @return 0 succuss
 * @return errno failed
 */
int switch_stdout_to_logfile()
{
#define SUCCESS_TO_SWITCH (0)
#define FAILED_TO_SWITCH  (errno)

  close(STDOUT_FILENO);

  if (dup(fileno(fp_log_file)) == -1)
    return FAILED_TO_SWITCH;

  return SUCCESS_TO_SWITCH;
#undef SUCCESS_TO_SWITCH
#undef FAILED_TO_SWITCH
}

/**
 * switch_stdin_to_fifo
 *  - close stdin and dup to file
 *
 * @return 0 succuss
 * @return errno failed
 */
int switch_stdin_to_fifo()
{
#define SUCCESS_TO_SWITCH (0)
#define FAILED_TO_SWITCH  (errno)

  close(STDIN_FILENO);
  
  if (dup(fd_control_pipe) == -1)
    return FAILED_TO_SWITCH;

  return SUCCESS_TO_SWITCH;
#undef SUCCESS_TO_SWITCH
#undef FAILED_TO_SWITCH
}

/**
 * create_control_pipe
 *  - create and open daemon fifo
 *
 * @param dir working directory to store file
 * @param basename basename of log file.
 * @return 0 succuss
 * @return errno failed
 */
int create_control_pipe(char *dir ,char *basename)
{
#define CONTROL_PIPE_CREATED      (0)
#define CONTROL_PIPE_NOT_CREATED  (errno)

  assert(basename);

  static char *def_dir =".";
  if (!dir)
    dir =def_dir;

  char path[PATH_MAX];
  sprintf(path,"%s/%s.ctl" ,dir ,basename);

  mode_t mode =0666;

  if (mkfifo(path,mode) == -1)
  {
    if (errno != EEXIST)
    {
      return CONTROL_PIPE_NOT_CREATED;
    }
  }

  if ((fd_control_pipe =open(path ,O_RDONLY|O_NONBLOCK)) == -1)
  {
    return (fd_control_pipe == -1);
  }
  
  if ((fp_control =fdopen(fd_control_pipe,"rw")) == NULL)
  {
    return (fp_control == NULL);
  }

  dbgprint(__FUNCTION__ ,"fd_control_pipe =%d" ,fd_control_pipe);

  return CONTROL_PIPE_CREATED;

#undef CONTROL_PIPE_CREATED
#undef CONTROL_PIPE_NOT_CREATED
}

/**
 * restrict_process
 *  - restrict only one process at one time
 *
 * @return 0 allowed
 * @return 1 denied
 */
int restrict_process(char *dir ,char *basename)
{
#define RESTRICTION_ALLOWED   (0)
#define RESTRICTION_DENIED    (1)

  assert(basename);

  static char *def_dir =".";
  if (!dir)
    dir =def_dir;

  static int fdlock =-1;
  if (fdlock != -1)
    return RESTRICTION_ALLOWED;

  struct flock fl;

  fl.l_type   =F_WRLCK;
  fl.l_whence =SEEK_SET;
  fl.l_start  =0;
  fl.l_len    =0;

  char path[PATH_MAX];
  sprintf(path,"%s/%s.lck" ,dir ,basename);

  if((fdlock = open(path, O_WRONLY|O_CREAT, 0666)) == -1)
    return RESTRICTION_DENIED;

  if(fcntl(fdlock, F_SETLK, &fl) == -1)
    return RESTRICTION_DENIED;

  dbgprint(__FUNCTION__ ,"restricted fd =%d" ,fdlock);

  return RESTRICTION_ALLOWED;

#undef RESTRICTION_ALLOWED
#undef RESTRICTION_DENIED
}
