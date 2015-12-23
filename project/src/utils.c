#include <pebble.h>

#include "calculator.h"


static bool debug = true;


void log_grect(GRect grect, char *info) {
  if(debug) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "GRect: \t%4d\t%4d\t%4d\t%4d\t(%s)",
            grect.origin.x, grect.origin.y, grect.size.w, grect.size.h, info);
  }
}


void log_gpoint(GPoint gpoint, char *info) {
  if(debug) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "GPoint:\t%4d\t%4d\t%4d\t%4d\t(%s)",
            gpoint.x, gpoint.y, 0, 0, info);
  }

}


void log_gsize(GSize gsize, char *info) {
  if(debug) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "GSize: \t%4d\t%4d\t%4d\t%4d\t(%s)",
            0, 0, gsize.w, gsize.h, info);
  }
}