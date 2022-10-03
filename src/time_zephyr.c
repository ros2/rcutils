#include "rcutils/time.h"

#include <zephyr/kernel.h>

#include "./common.h"
#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"

rcutils_ret_t
rcutils_system_time_now(rcutils_time_point_value_t * now)
{
	RCUTILS_CHECK_ARGUMENT_FOR_NULL(now, RCUTILS_RET_INVALID_ARGUMENT);
	*now = k_ticks_to_ns_floor64(k_uptime_ticks());
	return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_steady_time_now(rcutils_time_point_value_t * now)
{
	RCUTILS_CHECK_ARGUMENT_FOR_NULL(now, RCUTILS_RET_INVALID_ARGUMENT);
	*now = k_ticks_to_ns_floor64(k_uptime_ticks());
	return RCUTILS_RET_OK;
}
