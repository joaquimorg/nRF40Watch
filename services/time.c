#include "time.h"
#include <stdio.h>
#include <time.h>

static char time_str[16];

void time_service_init(void) {
    // RTC init placeholder
}

const char *time_service_get_time_string(void) {
    snprintf(time_str, sizeof(time_str), "%02d:%02d", 12, 34);
    return time_str;
}
