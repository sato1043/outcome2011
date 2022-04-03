#ifndef __state_h__
#define __state_h__

#include <pthread.h>

#include "thr_glue.h"

#include "title_lcd.h"
#include "ready_lcd.h"
#include "ready_leds.h"
#include "fixed_7seg.h"
#include "ready_sw.h"

#include "bet_lcd.h"
#include "bet_leds.h"

#include "single_sw.h"
#include "roll_7seg.h"

#include "reach_7seg.h"

#include "slip_7seg.h"

#include "praise_lcd.h"
#include "repay_leds.h"

//types used in state interface

struct reels_t_; 
struct player_t_; 
struct gamectx_t_; 
struct cointank_t_; 
struct settings_t_;

//state class
#define DECLARE_STATE_BASE \
    pthread_mutex_t unitary;       \
    struct state_t_* this;         \
    struct reels_t_* preels;       \
    struct player_t_* pplayer;     \
    struct gamectx_t_* pcontext;   \
    struct cointank_t_* pcointank; \
    struct settings_t_* psettings; \
    void (*construct)();           \
    void (*destruct)();            \
    void (*entry)();               \
    void (*exit_state)();          \
    void (*abort_state)();         \
    void (*add_ctx)(void (*routine)(void *) ,void *arg); \
    void (*rm_ctx)(void (*routine)(void *) ,void *arg); \
    int thread_ref_cnt;            \
    pthread_cond_t no_more_thr;    \
    int safely_terminated;

#define IMPLEMENT_STATE_BASE(prefix) \
    .unitary  =PTHREAD_MUTEX_INITIALIZER \
  , .this     =(state_t*)&prefix         \
  , .preels   =&reels                    \
  , .pplayer  =&player                   \
  , .pcontext =&gamectx                  \
  , .pcointank=&cointank                 \
  , .psettings=&settings                 \
  , .construct  =prefix##_construct      \
  , .destruct   =prefix##_destruct       \
  , .entry      =prefix##_entry          \
  , .exit_state =prefix##_exit_state     \
  , .abort_state=prefix##_abort_state    \
  , .rm_ctx     =prefix##_rm_ctx         \
  , .add_ctx    =prefix##_add_ctx        \
  , .thread_ref_cnt =0                   \
  , .no_more_thr =PTHREAD_COND_INITIALIZER \
  , .safely_terminated =0

#define DECLARE_STATE_BASE_METHOD(prefix) \
  static void prefix##_construct();   \
  static void prefix##_destruct();    \
  static void prefix##_entry();       \
  static void prefix##_exit_state();  \
  static void prefix##_abort_state(); \
  static void prefix##_rm_ctx(void (*routine)(void *) ,void *arg);  \
  static void prefix##_add_ctx(void (*routine)(void *) ,void *arg);

typedef struct state_t_  state_t ;
typedef struct state_t_* state_tp;

struct state_t_
{
  DECLARE_STATE_BASE;
};

typedef struct state_preparing_t_  state_preparing_t ;
typedef struct state_preparing_t_* state_preparing_tp;

struct state_preparing_t_
{
  DECLARE_STATE_BASE;
  DECLERE_TITLE_LCD ;
  DECLERE_READY_LCD ;
  DECLERE_READY_LEDS;
  DECLERE_FIXED_7SEG;
  DECLERE_READY_SW  ;
};

typedef struct state_betting_t_  state_betting_t ;
typedef struct state_betting_t_* state_betting_tp;

struct state_betting_t_
{
  DECLARE_STATE_BASE;
  DECLERE_TITLE_LCD ;
  DECLERE_BET_LCD   ;
  DECLERE_BET_LEDS  ;
  DECLERE_FIXED_7SEG;
  int is_turnable;
  int is_startable;
  void (*begin_wait)();
  DECLERE_SINGLE_SW(2);
};

typedef struct state_rolling_3_t_  state_rolling_3_t ;
typedef struct state_rolling_3_t_* state_rolling_3_tp;

struct state_rolling_3_t_
{
  DECLARE_STATE_BASE;
  DECLERE_TITLE_LCD ;
  DECLERE_BET_LCD   ;
  DECLERE_READY_LEDS;
  DECLERE_ROLL_7SEG ;
  int fixed_index;
  DECLERE_SINGLE_SW(3);
};

typedef struct state_rolling_2_t_  state_rolling_2_t ;
typedef struct state_rolling_2_t_* state_rolling_2_tp;

struct state_rolling_2_t_
{
  DECLARE_STATE_BASE;
  DECLERE_TITLE_LCD ;
  DECLERE_BET_LCD   ;
  DECLERE_READY_LEDS;
  DECLERE_ROLL_7SEG ;
  int fixed_index;
  DECLERE_SINGLE_SW(3);
};

typedef struct state_rolling_1_t_  state_rolling_1_t ;
typedef struct state_rolling_1_t_* state_rolling_1_tp;

struct state_rolling_1_t_
{
  DECLARE_STATE_BASE;
  DECLERE_TITLE_LCD ;
  DECLERE_BET_LCD   ;
  DECLERE_READY_LEDS;
  DECLERE_ROLL_7SEG ;
  int fixed_index;
  DECLERE_SINGLE_SW(3);
};

typedef struct state_rolling_0_t_  state_rolling_0_t ;
typedef struct state_rolling_0_t_* state_rolling_0_tp;

struct state_rolling_0_t_
{
  DECLARE_STATE_BASE;
  DECLERE_THR_GLUE;
};

typedef struct state_rolling_r_t_  state_rolling_r_t ;
typedef struct state_rolling_r_t_* state_rolling_r_tp;

struct state_rolling_r_t_
{
  DECLARE_STATE_BASE;
  DECLERE_TITLE_LCD ;
  DECLERE_BET_LCD   ;
  DECLERE_READY_LEDS;
  DECLERE_REACH_7SEG;
  int fixed_index;
  DECLERE_SINGLE_SW(3);
};

typedef struct state_holding_t_  state_holding_t ;
typedef struct state_holding_t_* state_holding_tp;

struct state_holding_t_
{
  DECLARE_STATE_BASE;
  DECLERE_TITLE_LCD ;
  DECLERE_BET_LCD   ;
  DECLERE_READY_LEDS;
  DECLERE_SLIP_7SEG;
};

typedef struct state_winning_t_  state_winning_t ;
typedef struct state_winning_t_* state_winning_tp;

struct state_winning_t_
{
  DECLARE_STATE_BASE;
  DECLERE_PRAISE_LCD;
  DECLERE_BET_LCD   ;
  DECLERE_REPAY_LEDS;
  DECLERE_FIXED_7SEG;
  long coin_increment_delay_usec;
};

//getting state instance

typedef enum state_type_t_  state_type_t ;
typedef enum state_type_t_* state_type_tp;

enum state_type_t_
{
    NEUTRAL
  , PREPARING
  , BETTING
  , ROLLING_3
  , ROLLING_2
  , ROLLING_1
  , ROLLING_0
  , ROLLING_R
  , HOLDING
  , WINNING
  , MAX_STATE_TYPE
};

state_t* get_state_instance(state_type_t type);

//state instances
extern struct state_preparing_t_ preparing;
extern struct state_betting_t_   betting  ;
extern struct state_rolling_3_t_ rolling_3;
extern struct state_rolling_2_t_ rolling_2;
extern struct state_rolling_1_t_ rolling_1;
extern struct state_rolling_0_t_ rolling_0;
extern struct state_rolling_r_t_ rolling_r;
extern struct state_holding_t_   holding  ;
extern struct state_winning_t_   winning  ;

#define TURNABLE_COIN_THRESHOLD  (3)

void state_construct(state_t* pstate);
void state_add_ctx(state_t* pstate ,void (*routine)(void *) ,void *arg);
void state_rm_ctx(state_t* pstate ,void (*routine)(void *) ,void *arg);
void state_exit_wait(state_t* pstate);
void state_destruct(state_t* pstate);

#endif/*__state_h__*/
