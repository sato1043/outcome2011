#ifndef __reels_h__
#define __reels_h__

#include "rdwr.h"

typedef struct reels_t_  reels_t;
typedef struct reels_t_* reels_tp;

#define NUM_REELS (3)

#define NUM_REEL_PTRN  (10)

#define REEL_7SEG_STRING_LEN (3)

typedef enum rotation_ptn_t_  rotation_ptn_t ;
typedef enum rotation_ptn_t_* rotation_ptn_tp;

enum rotation_ptn_t_ 
{
    RP_FIXED
  , RP_STATIC
  , RP_RANDOM
  , RP_INCREMENT
  , RP_DECREMENT
  , RP_MAX
};
#define RP_DEFAULT RP_STATIC

#define NOT_FIXED (-1)

typedef struct rotation_info_t_  rotation_info_t ;
typedef struct rotation_info_t_* rotation_info_tp;

struct reels_t_
{
  pthread_rdwr_t rwlock;

  void (*construct)(int sps
    , int l0 ,int o0 ,rotation_ptn_t rp0
    , int l1 ,int o1 ,rotation_ptn_t rp1
    , int l2 ,int o2 ,rotation_ptn_t rp2);
  void (*destruct)();

  int slices_per_second;

  struct rotation_info_t_
  {
    int slice;
    int limit;
    int offs;

    rotation_ptn_t ptn;     //TODO:rename to ptn_id
    int reel[NUM_REEL_PTRN];//TODO:rename to ptn
    int curs;

    int was_fixed;
    int fixed_value;

  } reels[NUM_REELS]; //TODO:rename reel

  int  (*rotate)(char str[REEL_7SEG_STRING_LEN+1]);
  int  (*get_fixed)(int n);
  void (*get_string)(char str[REEL_7SEG_STRING_LEN+1]);
  void (*change_ptn_id)(int n ,rotation_ptn_t ptn_id);

  int (*was_fixed)(int n);
  int (*not_fixed)(int n);
  void (*fix)(int n);
  void (*cancel)(int n);

  int (*was_reached)();
  int (*was_complated)();
  int (*get_rate)();

  void (*reset_fixed)();
};

#define DECLARE_REELS_METHOD \
  static void reels_construct(              \
      int sps                               \
    , int l0 ,int o0 ,rotation_ptn_t rp0    \
    , int l1 ,int o1 ,rotation_ptn_t rp1    \
    , int l2 ,int o2 ,rotation_ptn_t rp2);  \
  static void reels_destruct();             \
  static int  reels_was_fixed(int n);       \
  static int  reels_not_fixed(int n);       \
  static void reels_fix(int n);             \
  static void reels_cancel(int n);          \
  static int  reels_rotate(char str[REEL_7SEG_STRING_LEN+1]);     \
  static int  reels_get_fixed(int n);       \
  static void reels_get_string(char str[REEL_7SEG_STRING_LEN+1]); \
  static void reels_change_ptn_id(int n ,rotation_ptn_t ptn_id);  \
  static int  reels_was_reached();          \
  static int  reels_was_complated();        \
  static int  reels_get_rate();             \
  static void reels_reset_fixed();

#define IMPLEMENT_REELS(so,l0,o0,rp0,l1,o1,rp1,l2,o2,rp2) \
  reels_t reels = \
  {                                       \
      .construct  =reels_construct        \
    , .destruct   =reels_destruct         \
    , .slices_per_second =so              \
    , .reels[0] ={-1,l0,o0,rp0,{0,0,0,0,0,0,0,0,0,0},NUM_REEL_PTRN-1,FALSE,NOT_FIXED} \
    , .reels[1] ={-1,l1,o1,rp1,{0,0,0,0,0,0,0,0,0,0},NUM_REEL_PTRN-1,FALSE,NOT_FIXED} \
    , .reels[2] ={-1,l2,o2,rp2,{0,0,0,0,0,0,0,0,0,0},NUM_REEL_PTRN-1,FALSE,NOT_FIXED} \
    , .rotate        =reels_rotate        \
    , .get_fixed     =reels_get_fixed     \
    , .get_string    =reels_get_string    \
    , .was_fixed     =reels_was_fixed     \
    , .not_fixed     =reels_not_fixed     \
    , .fix           =reels_fix           \
    , .cancel        =reels_cancel        \
    , .change_ptn_id =reels_change_ptn_id \
    , .was_reached   =reels_was_reached   \
    , .was_complated =reels_was_complated \
    , .get_rate      =reels_get_rate      \
    , .reset_fixed=reels_reset_fixed      \
  }

extern reels_t reels;

#endif/*__reels_h__*/
