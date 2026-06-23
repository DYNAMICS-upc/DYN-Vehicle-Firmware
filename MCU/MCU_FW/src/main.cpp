#include "app.h"
#include "ota_service.h"

extern "C" void app_main(void) {
    app_init();
    ota_service_init();
    app_run();
}
