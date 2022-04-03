#ifndef __util_h__
#define __util_h__

#define FALSE (0)
#define TRUE  (1)

#define FAILED  (1)
#define SUCCEED (0)

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <memory.h>
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <errno.h>
#include <limits.h>
#include <linux/limits.h> //PATH_MAX

/**
 * fp_control
 *  - FILE* of the FIFO to control daemon from outside.
 */
extern FILE* fp_control;

void atoBitPrnString(char led_dt, char* led_string);

char* getstring(char buf[], size_t maxchar);

//void dbgprint(const char* funcname ,char *fmt, ...);
char* dbgprint_hdr(const char* funcname ,char* buf, size_t len);
char* dbgprint_ftr(const char* funcname ,char* buf, size_t len);

#define dbgprint(funcname , ...) \
{                                                       \
  char hdr[256];                                        \
  fprintf(stderr,"%s" ,dbgprint_hdr(funcname,hdr,256)); \
  fprintf(stderr,##__VA_ARGS__);                        \
  fprintf(stderr,"%s" ,dbgprint_ftr(funcname,hdr,256)); \
    /*TODO: Is there anything like strerror_r? */       \
  fprintf(stderr,"\n");                                 \
  fflush(stderr);                                       \
}

int switch_stderr_to_logfile();
int switch_stdout_to_logfile();
int switch_stdin_to_fifo();

int restrict_process(char *dir ,char *basename);
int create_control_pipe(char *dir ,char *basename);
int create_log_file(char *dir ,char *basename);

#endif/*__util_h__*/
