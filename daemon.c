#include <unistd.h> //fork
#include <sys/stat.h>//umask
#include <string.h>//strsignal
#include <errno.h>//errno
#include <signal.h>//signal
#include <sys/poll.h>//poll

#include "rdwr.h"
#include "main.h"
#include "state.h"
#include "daemon.h"
#include "gamectx.h"
#include "settings.h"
#include "util.h"

void signal_handler(int sig);

#define BUFSZ (256)

#define N_POLL    (1)
#define N_TMOUT   (1000)

typedef struct daemon_t_  daemon_t;
typedef struct daemon_t_* daemon_tp;

struct daemon_t_
{
  pthread_rdwr_t rwlock;
  int is_running;
};

static daemon_t dmn;

/**
 * daemon_construct
 *  - construct instance of daemon
 */
void daemon_construct()
{
  int rtn;
  if ((rtn =pthread_rdwr_init_np(&dmn.rwlock, NULL)) !=0)
  {
    dbgprint(__FUNCTION__ ,"pthread_rdwr_init_np error %d" ,rtn);
    exit(EXIT_FAILURE);
  }

  pthread_rdwr_wlock_np(&dmn.rwlock);

  dmn.is_running =TRUE;

  pthread_rdwr_wunlock_np(&dmn.rwlock);

  gamectx.construct(&settings);
}

/**
 * daemon_destruct
 *  - destruct instance of daemon
 */
void daemon_destruct()
{
  gamectx.destruct();
}

/**
 * daemon_main
 *  - main method of this system.
 *
 * @param argc amount of command line arguments
 * @param argv array of pointers to argument
 * @return EXIT_SUCCESS when successfully
 * @return EXIT_FAILURE with failure
 */
int daemon_main(int argc ,char *argv[])
{
#ifdef NDEBUG
  pid_t pid;
  
  if ((pid =fork()) < 0)
  {
    exit(EXIT_FAILURE);
  }
  else if (0 < pid)
  {
    exit(EXIT_FAILURE);
  }

  //here comes child process
#endif

  umask(0);
    
  //TODO: use /var/run/esmd.pid

  if (create_log_file(".",PGNAME_DAEMON))
  {
    fputs(PGNAME_DAEMON ":can't open log file.\n", stderr);
    exit(EXIT_FAILURE);
  }

  if (switch_stderr_to_logfile())
  {
    dbgprint(__FUNCTION__,"can't duplicate stderr to log file.");
    exit(EXIT_FAILURE);
  }

#ifdef NDEBUG
  if (switch_stdout_to_logfile())
  {
    dbgprint(__FUNCTION__,"can't duplicate stdout to log file.");
    exit(EXIT_FAILURE);
  }
#endif

  //
  dbgprint(__FUNCTION__ ,PGNAME_DAEMON " hello.");
  //

  if (restrict_process(".",PGNAME_DAEMON))
  {
    dbgprint(__FUNCTION__ ,"process already running.");
    exit(EXIT_FAILURE);
  }

  if (create_control_pipe(".",PGNAME_DAEMON))
  {
    dbgprint(__FUNCTION__ ,"can't open control between daemon and control.");
    exit(EXIT_FAILURE);
  }

#ifdef NDEBUG
  if ((sid =setsid()) < 0)
  {
    dbgprint(__FUNCTION__ ,"can't create a new SID for child process.");
    exit(EXIT_FAILURE);
  }
  
  if ((chdir("/")) < 0)
  {
    dbgprint(__FUNCTION__ ,"can't change directory to / .");
    exit(EXIT_FAILURE);
  }
#endif
  
  if (switch_stdin_to_fifo())
  {
    dbgprint(__FUNCTION__,"can't duplicate stdin to control pipe.");
    exit(EXIT_FAILURE);
  }

  setup_default_settings(&settings);

  if (extract_settings_from_cmd_line(&settings ,argc ,argv))
  {
    dbgprint(__FUNCTION__ ,"can't extract setting from cmd line.");
    exit(EXIT_FAILURE);
  }

  daemon_construct();

  signal(SIGINT  ,signal_handler);
  signal(SIGHUP  ,signal_handler); //TODO: thread signal mask
  signal(SIGTERM ,signal_handler);
  signal(SIGQUIT ,signal_handler);

  struct pollfd ufds[N_POLL];

  memset(&ufds,0,sizeof(ufds));
  ufds[0].fd =fileno(fp_control);
  ufds[0].events =POLLIN;

  while (dmn.is_running)
  {
    if (poll(ufds, N_POLL, 100) == -1)
    {
      dbgprint(__FUNCTION__,"poll(ufs):");
      exit(EXIT_FAILURE);
    }

    if (ufds[0].revents & (POLLIN))
    {
      char buf[BUFSZ];
      memset(buf,'\0',sizeof(buf));
      dbgprint(__FUNCTION__ , "control read '%s'" ,getstring(buf,BUFSZ));

      if (strncmp(buf,"quit",4) == 0)
        break;

      /* do some task here ... */
    }

    usleep(10);
  }

  daemon_destruct();

  dbgprint(__FUNCTION__ ,PGNAME_DAEMON " bye.");

  exit(EXIT_SUCCESS);
}

void shutdown_daemon()
{
  pthread_rdwr_wlock_np(&dmn.rwlock);

  dmn.is_running =FALSE;

  pthread_rdwr_wunlock_np(&dmn.rwlock);
}

void signal_handler(int sig)
{
  switch(sig)
  {
  case SIGHUP:
    dbgprint(__FUNCTION__ ,"Received SIGHUP signal.");
    break;
  case SIGTERM:
    dbgprint(__FUNCTION__ ,"Received SIGTERM signal.");
    break;
  default:
    dbgprint(__FUNCTION__ ,"Unhandled signal (%d) %s", sig ,strsignal(sig));
    break;
  }
  shutdown_daemon();
}
