#ifndef __common_h__
#define __common_h__

#include <stdlib.h> //pid_t

/**
 * PGNAME_DAEMON / PGNAME_CONTROL
 *  - program file names (binary or link)
 *  - esmd depends its basename to determine the run-mode.
 */
#define PGNAME_DAEMON   "esmd"
#define PGNAME_CONTROL  "esmctl"

/**
 * sid
 *  - sid at daemon mode
 */
extern pid_t sid;

#endif/*__common_h__*/
