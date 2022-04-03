#ifndef __settings_h__
#define __settings_h__

typedef struct settings_t_  settings_t;
typedef struct settings_t_* settings_tp;

struct settings_t_
{
  long coins;
  long r1slices, r1ptn;
  long r2slices, r2ptn;
  long r3slices, r3ptn;
  long slices_per_second;
};

extern settings_t settings;

void setup_default_settings(settings_t* psettings);
int extract_settings_from_cmd_line(settings_t* psettings ,int argc ,char *argv[]);

#endif/*__settings_h__*/
