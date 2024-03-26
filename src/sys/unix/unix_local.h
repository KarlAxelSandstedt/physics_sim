#ifndef __SYSTEM_LOCAL_H__
#define __SYSTEM_LOCAL_H__

#include <stdint.h>
#include "mg_common.h"

/******************** x11_event.c ********************/

i32 X11_system_event_queue(const struct system_event *event);

#endif
