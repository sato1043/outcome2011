#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>

#include "settings.h"
#include "reels.h"
#include "util.h"

settings_t settings;

/**
 * setup_default_settings
 *  - setup default settings
 *
 * @param psettings pointer to the struct that retrived.
 */
void setup_default_settings(settings_t* psettings)
{
  settings_t dummy_settings;

  memset(&dummy_settings,0,sizeof(dummy_settings));

  dummy_settings.coins =6;//30;
  dummy_settings.r1slices =12;
  dummy_settings.r1ptn =RP_DEFAULT;
  dummy_settings.r2slices =12;
  dummy_settings.r2ptn =RP_DEFAULT;
  dummy_settings.r3slices =12;
  dummy_settings.r3ptn =RP_DEFAULT;
  dummy_settings.slices_per_second =48;

  *psettings =dummy_settings;
}

/**
 * extract_settings_from_cmd_line
 *  - extract setting from command line
 *
 * @param argc amount of command line arguments
 * @param argv array of pointers to argument
 */
int extract_settings_from_cmd_line(settings_t* psettings ,int argc ,char *argv[])
{
#define EXTRACT_SUCCESS (0)
#define EXTRACT_FAILED  (-1)

  settings_t dummy_settings =*psettings;

  do
  {
    int c ,option_index =0;
    static struct option long_options[] =
    {
      {"coins"              ,1,0,0},
      {"r1slices"           ,1,0,0},
      {"r1ptn"              ,1,0,0},
      {"r2slices"           ,1,0,0},
      {"r2ptn"              ,1,0,0},
      {"r3slices"           ,1,0,0},
      {"r3ptn"              ,1,0,0},
      {"slices_per_second"  ,1,0,0},
      {0, 0, 0, 0}
    };

    c = getopt_long(argc ,argv ,"" ,long_options ,&option_index);
    if (c == -1)
      break;

    switch (c)
    {
    case 0:

      if (strncmp(long_options[option_index].name ,"coins" ,5) == 0)
      {
        if (!optarg) return EXTRACT_FAILED;
        long tmp =strtol(optarg,0,10);
        if (errno == EINVAL || errno == ERANGE) return EXTRACT_FAILED;

        dummy_settings.coins =tmp;

        dbgprint(__FUNCTION__ ,"dummy_settings.coins =%ld" ,dummy_settings.coins);
      }

      if (strncmp(long_options[option_index].name ,"r1slices" ,5) == 0)
      {
        if (!optarg) return EXTRACT_FAILED;
        long tmp =strtol(optarg,0,10);
        if (errno == EINVAL || errno == ERANGE) return EXTRACT_FAILED;

        dummy_settings.r1slices =tmp;

        dbgprint(__FUNCTION__ ,"dummy_settings.r1slices =%ld" ,dummy_settings.r1slices);
      }

      if (strncmp(long_options[option_index].name ,"r1ptn" ,5) == 0)
      {
        if (!optarg) return EXTRACT_FAILED;
        long tmp =strtol(optarg,0,10);
        if (errno == EINVAL || errno == ERANGE) return EXTRACT_FAILED;

        dummy_settings.r1ptn =tmp;

        if (tmp <= RP_FIXED || RP_MAX <= tmp) return EXTRACT_FAILED;

        dbgprint(__FUNCTION__ ,"dummy_settings.r1ptn =%ld" ,dummy_settings.r1ptn);
      }

      if (strncmp(long_options[option_index].name ,"r2slices" ,5) == 0)
      {
        if (!optarg) return EXTRACT_FAILED;
        long tmp =strtol(optarg,0,10);
        if (errno == EINVAL || errno == ERANGE) return EXTRACT_FAILED;

        dummy_settings.r2slices =tmp;

        dbgprint(__FUNCTION__ ,"dummy_settings.r2slices =%ld" ,dummy_settings.r2slices);
      }

      if (strncmp(long_options[option_index].name ,"r2ptn" ,5) == 0)
      {
        if (!optarg) return EXTRACT_FAILED;
        long tmp =strtol(optarg,0,10);
        if (errno == EINVAL || errno == ERANGE) return EXTRACT_FAILED;

        dummy_settings.r2ptn =tmp;

        if (tmp <= RP_FIXED || RP_MAX <= tmp) return EXTRACT_FAILED;

        dbgprint(__FUNCTION__ ,"dummy_settings.r2ptn =%ld" ,dummy_settings.r2ptn);
      }

      if (strncmp(long_options[option_index].name ,"r3slices" ,5) == 0)
      {
        if (!optarg) return EXTRACT_FAILED;
        long tmp =strtol(optarg,0,10);
        if (errno == EINVAL || errno == ERANGE) return EXTRACT_FAILED;

        dummy_settings.r3slices =tmp;

        dbgprint(__FUNCTION__ ,"dummy_settings.f3fps =%ld" ,dummy_settings.r3slices);
      }

      if (strncmp(long_options[option_index].name ,"r3ptn" ,5) == 0)
      {
        if (!optarg) return EXTRACT_FAILED;
        long tmp =strtol(optarg,0,10);
        if (errno == EINVAL || errno == ERANGE) return EXTRACT_FAILED;

        dummy_settings.r3ptn =tmp;

        if (tmp <= RP_FIXED || RP_MAX <= tmp) return EXTRACT_FAILED;

        dbgprint(__FUNCTION__ ,"dummy_settings.r3ptn =%ld" ,dummy_settings.r3ptn);
      }

      if (strncmp(long_options[option_index].name ,"slices_per_second" ,5) == 0)
      {
        if (!optarg) return EXTRACT_FAILED;
        long tmp =strtol(optarg,0,10);
        if (errno == EINVAL || errno == ERANGE) return EXTRACT_FAILED;

        dummy_settings.slices_per_second =tmp;

        dbgprint(__FUNCTION__ ,"dummy_settings.slices_per_second =%ld" ,dummy_settings.slices_per_second);
      }
      break;

    default:
      return EXTRACT_FAILED;
    }
    
  } while(1);

  if (optind < argc)
  {
		while (optind < argc)
    {
      dbgprint(__FUNCTION__ ,"non-option ARGV-elements [%d] =%s" ,optind ,argv[optind]);
      ++optind;
		}
  }

  memset(psettings,0,sizeof(settings_t));

  *psettings =dummy_settings;

  return EXTRACT_SUCCESS;

#undef EXTRACT_SUCCESS
#undef EXTRACT_FAILED
}
