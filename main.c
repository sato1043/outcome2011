#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <libgen.h>

#include "daemon.h"
#include "control.h"

#include "main.h"

pid_t sid;

/**
 * main
 *  - system's startup routine
 *
 * @param argc amount of command line arguments
 * @param argv array of pointers to argument
 */
int main(int argc ,char *argv[])
{
  int exit_code =EXIT_FAILURE;

  char* bname =basename(argv[0]);

  if (!strncmp(bname,PGNAME_DAEMON,strlen(bname)))
  {
    exit_code =daemon_main(argc,argv);
  }
  else if (!strncmp(bname,PGNAME_CONTROL,strlen(bname)))
  {
    exit_code =control_main(argc,argv);
  }
  else
  {
    fprintf(stderr,"basename:%sに対応していません。\n" ,bname);
  }
  exit(exit_code);
}
