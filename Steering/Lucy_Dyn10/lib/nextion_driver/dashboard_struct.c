#include "dashboard_struct.h"
#include <string.h>

void dashboard_struct_init(dashboard_struct_t* dash) {
    if (dash) {
        memset(dash, 0, sizeof(dashboard_struct_t));
    }
}
