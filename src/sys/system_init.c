#include "system_public.h"
#include "system_local.h"
#include "system_common.h"
#include "timer.h"

void system_resources_init(struct arena *mem)
{
	input_state_init(mem);
	system_event_queue_new(mem);
	precision_timer_init();
}

void system_resources_cleanup(void)
{

}
