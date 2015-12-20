#include <pebble.h>

#include "calculator.h"


void log_grect(GRect grect, char *info) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "GRect %s: .x = %d, .y = %d, .w = %d, .h = %d",
          info, grect.origin.x, grect.origin.y, grect.size.w, grect.size.h);
}


void log_gpoint(GPoint gpoint, char *info) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "gpoint %s: .x = %d, .y = %d",
          info, gpoint.x, gpoint.y);
}


void log_gsize(GSize gsize, char *info) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "GSize %s: .w = %d, .h = %d",
          info, gsize.w, gsize.h);
}