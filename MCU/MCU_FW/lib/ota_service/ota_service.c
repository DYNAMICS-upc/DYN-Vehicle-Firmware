#include "ota_service.h"

#if defined(ESP_PLATFORM)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_server.h"
#include <string.h>
#include <sys/param.h>

#define WIFI_SSID "DynamicsRouter"
#define WIFI_PASS "Dynamics2026"
static const char *TAG = "OTA_SERVICE";
static bool s_ota_r2d_active = false;

void ota_set_r2d_state(bool is_r2d) {
    s_ota_r2d_active = is_r2d;
}

static esp_err_t ota_update_handler(httpd_req_t *req) {
    if (s_ota_r2d_active) {
        ESP_LOGE(TAG, "OTA rejected: Vehicle is in R2D (Ready to Drive)!");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    esp_ota_handle_t ota_handle;
    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
    if (!update_partition) {
        ESP_LOGE(TAG, "No OTA partition found");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    if (esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &ota_handle) != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    char buf[1024];
    int received = 0;
    int remaining = req->content_len;

    while (remaining > 0) {
        if ((received = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)))) <= 0) {
            if (received == HTTPD_SOCK_ERR_TIMEOUT) continue;
            esp_ota_abort(ota_handle);
            return ESP_FAIL;
        }
        if (esp_ota_write(ota_handle, buf, received) != ESP_OK) {
            esp_ota_abort(ota_handle);
            return ESP_FAIL;
        }
        remaining -= received;
    }

    if (esp_ota_end(ota_handle) != ESP_OK || esp_ota_set_boot_partition(update_partition) != ESP_OK) {
        ESP_LOGE(TAG, "OTA end or set boot partition failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_sendstr(req, "OTA Success. Rebooting...");
    vTaskDelay(pdMS_TO_TICKS(1000));
    esp_restart();
    return ESP_OK;
}

static void wifi_init_sta(void) {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
    esp_wifi_connect();
}

static void ota_server_task(void *pvParameter) {
    wifi_init_sta();
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.core_id = 0; // Run HTTP server on Core 0
    httpd_handle_t server = NULL;
    
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t ota_uri = {
            .uri       = "/update",
            .method    = HTTP_POST,
            .handler   = ota_update_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &ota_uri);
    }
    vTaskDelete(NULL);
}

void ota_service_init(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      nvs_flash_init();
    }
    xTaskCreatePinnedToCore(ota_server_task, "ota_task", 8192, NULL, 5, NULL, 0);
}
#else
void ota_service_init(void) {}
void ota_set_r2d_state(bool is_r2d) { (void)is_r2d; }
#endif
