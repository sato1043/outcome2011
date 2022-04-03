#include "state.h"
#include "reels.h"
#include "player.h"
#include "gamectx.h"
#include "cointank.h"
#include "settings.h"
#include "util.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "lcddrv.h"

#define TPOOL_MAX (30)

static void gamectx_construct(settings_t* psettings);
static void gamectx_destruct();
static void gamectx_add_ctx(void (*routine)(void *) ,void *arg);
static void gamectx_alternate(state_type_t curr ,state_type_t next);

static void clear_lcd();
static void clear_leds();
static void clear_7seg();

/**
 * gamextx
 *  - global instance of game context
 */
gamectx_t gamectx = 
{
    .construct =gamectx_construct
  , .destruct  =gamectx_destruct
  , .add_ctx   =gamectx_add_ctx
  , .alternate =gamectx_alternate
};

/**
 * gamectx_construct
 *  - construct instance of game context.
 */
void gamectx_construct(settings_t* psettings)
{
  assert(psettings);

  clear_lcd();
  clear_leds();
  clear_7seg();

  int rtn;
  if ((rtn =pthread_rdwr_init_np(&gamectx.rwlock, NULL)) !=0)
  {
    dbgprint(__FUNCTION__ ,"pthread_rdwr_init_np error %d" ,rtn);
    exit(EXIT_FAILURE);
  }

  tpool_init(&gamectx.tpool ,TPOOL_MAX ,TPOOL_MAX ,TRUE);

  gamectx_alternate(NEUTRAL,PREPARING);
}

/**
 * gamectx_destruct
 *  - destruct instance of game context.
 */
void gamectx_destruct()
{
  gamectx.pcurrent_state->abort_state();

  tpool_destroy(gamectx.tpool,TRUE);

  clear_lcd();
  clear_leds();
  clear_7seg();
}

/**
 * gamectx_add_ctx
 *  - add some work to game context
 *
 * @param routine thread routinue
 * @param arg thread's argument
 */
void gamectx_add_ctx(
    void      (*routine)(void *)
  , void      *arg)
{
  if (tpool_add_work(gamectx.tpool,routine,arg))
  {
    dbgprint(__FUNCTION__ ,"tpool_add_work %08x,%08x"
        ,(unsigned int)routine,(unsigned int)arg);
    exit(EXIT_FAILURE);
  }
}

/**
 * gamectx_alternate
 *  - supposing that current state has safely terminated.
 *  - some global objects are constructed/destructed/modified
 *
 * @param from state from which alternate
 * @param to state to which alternate
 */
void gamectx_alternate(state_type_t from ,state_type_t to)
{
  assert(gamectx.pcurrent_state
      ? gamectx.pcurrent_state->safely_terminated : 1);

  pthread_rdwr_wlock_np(&gamectx.rwlock);

  //game will be stopped.
  if (to == PREPARING)
  {
    if (from == ROLLING_0
     || from == HOLDING) //player lose
    {
      cointank.destruct();
      player.destruct();
      reels.destruct();
    }
  }
  //game will be played.
  else if (to == BETTING)
  {
    if (from == PREPARING) //game becomes starting
    {
      reels.construct(settings.slices_per_second
                         ,settings.r1slices,0,settings.r1ptn //greater value, slower rotation
                         ,settings.r2slices,4,settings.r2ptn
                         ,settings.r3slices,8,settings.r3ptn);
      player.construct();
      cointank.construct(&settings);
    }
    else if (from == ROLLING_0
          || from == HOLDING
          || from == WINNING) //game continues to play
    {
      reels.destruct();
      reels.construct(settings.slices_per_second
                         ,settings.r1slices,0,settings.r1ptn //greater value, slower rotation
                         ,settings.r2slices,4,settings.r2ptn
                         ,settings.r3slices,8,settings.r3ptn);
    }
  }
  else if (to == ROLLING_3)
  {
    if (from == BETTING)
    {
      cointank.gain_from_player();
    }
  }

  gamectx.pcurrent_state =get_state_instance(to);

  gamectx.pcurrent_state->entry();

  pthread_rdwr_wunlock_np(&gamectx.rwlock);
}

/**
 * clear_lcd
 *  - clear LCD panel
 */
void clear_lcd()
{
#define DEVNAME "/dev/gpio/lcd0"
  int fd;

  if ((fd =open(DEVNAME,O_WRONLY)) == -1)
  {
    dbgprint(__FUNCTION__,"%s:",DEVNAME);
    exit(EXIT_FAILURE);
  }

  if (ioctl(fd,LCD_CLEAR,0) < 0)
  {
    dbgprint(__FUNCTION__,"%s:",DEVNAME);
    exit(EXIT_FAILURE);
  }

  close(fd);
#undef DEVNAME
}

/**
 * clear_leds
 *  - clear red leds
 */
void clear_leds()
{
#define DEVNAME "/dev/gpio/ddsp0"
  int fd;

  if ((fd =open(DEVNAME,O_WRONLY)) == -1)
  {
    dbgprint(__FUNCTION__,"%s:",DEVNAME);
    exit(EXIT_FAILURE);
  }

  if (write(fd,"        \0",8) != 8)
  {
    dbgprint(__FUNCTION__,"%s:",DEVNAME);
    exit(EXIT_FAILURE);
  }

  close(fd);
#undef DEVNAME
}

/**
 * clear_leds
 *  - clear red leds
 */
void clear_7seg()
{
#define DEVNAME "/dev/gpio/ddsp1"
  int fd;

  if ((fd =open(DEVNAME,O_WRONLY)) == -1)
  {
    dbgprint(__FUNCTION__,"%s:",DEVNAME);
    exit(EXIT_FAILURE);
  }

  if (write(fd,"   \0",3) != 3)
  {
    dbgprint(__FUNCTION__,"%s:",DEVNAME);
    exit(EXIT_FAILURE);
  }

  close(fd);
#undef DEVNAME
}
