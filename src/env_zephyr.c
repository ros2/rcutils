#include <stddef.h>

#include "rcutils/env.h"

bool rcutils_set_env(const char * env_name, const char * env_value)
{
	return false;
}

const char* rcutils_get_env(const char * env_name, const char ** env_value)
{
	if (NULL == *env_value) {
		*env_value = "";
	}
	return NULL;
}

const char* rcutils_get_home_dir(void)
{
	return NULL;
}
