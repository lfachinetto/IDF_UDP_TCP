#include <stdio.h>             //for basic printf commands
#include <string.h>            //for handling strings
#include "freertos/FreeRTOS.h" //for delay,mutexs,semphrs rtos operations
#include "esp_system.h"        //esp_init funtions esp_err_t
#include "esp_wifi.h"          //esp_wifi_init functions and wifi operations
#include "esp_log.h"           //for showing logs
#include "esp_event.h"         //for wifi event
#include "nvs_flash.h"         //non volatile storage
#include "lwip/err.h"          //light weight ip packets error handling
#include "lwip/sys.h"          //system applications for light weight ip apps
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

const char *ip = "192.168.0.142";
const int port = 8888;
int buffer = 1;

void udp_client_task(void *pvParameters)
{
    struct sockaddr_in dest_addr;
    char recv_buffer[64];
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        printf("Unable to create socket\n");
        vTaskDelete(NULL);
    }
    struct timeval tv;
    tv.tv_sec = 1; // 5-second timeout
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    dest_addr.sin_addr.s_addr = inet_addr(ip);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);

    while (1)
    {
        char str[5] = "\0\0\0\0\0";
        sprintf(str, "%d", buffer++);
        printf("Mandando: %s\n", str);
        int err = sendto(sock, str, sizeof(buffer), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0)
        {
            printf("erro: %d\n", errno);
        }

        struct sockaddr_in source_addr;
        socklen_t socklen = sizeof(source_addr);
        int recv_len = recvfrom(sock, recv_buffer, sizeof(recv_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

        if (recv_len > 0)
        {
            recv_buffer[recv_len] = '\0';
            printf("Recebeu: %s\n", recv_buffer);
        }
        else
        {
            printf("Erro: %d\n", errno);
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

const char *ssid = "KIRINUS";
const char *pass = "br654321";
int retry_num = 0;
static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_STA_START)
    {
        printf("WIFI CONNECTING....\n");
    }
    else if (event_id == WIFI_EVENT_STA_CONNECTED)
    {
        printf("WiFi CONNECTED\n");
    }
    else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        printf("WiFi lost connection\n");
        if (retry_num < 5)
        {
            esp_wifi_connect();
            retry_num++;
            printf("Retrying to Connect...\n");
        }
    }
    else if (event_id == IP_EVENT_STA_GOT_IP)
    {
        printf("Wifi got IP...\n\n");
        xTaskCreate(&udp_client_task, "udp_client_task", 4096, NULL, 5, NULL);
    }
}

void wifi_connection()
{
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation);
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);

    // Somente SSID
    // wifi_config_t wifi_configuration = {
    //     .sta = {
    //         .ssid = "",
    //     }
    // };
    // strcpy((char *)wifi_configuration.sta.ssid, ssid);
    // esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);

    // SSID e password
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = "",
            .password = "",
        }};
    strcpy((char *)wifi_configuration.sta.ssid, ssid);
    strcpy((char *)wifi_configuration.sta.password, pass);
    esp_log_write(ESP_LOG_INFO, "Kconfig", "SSID=%s, PASS=%s", ssid, pass);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);

    esp_wifi_start();
    esp_wifi_set_mode(WIFI_MODE_STA);

    esp_wifi_connect();
    printf("wifi_init_softap finished. SSID:%s  password:%s", ssid, pass);
}

void app_main(void)
{
    nvs_flash_init();
    wifi_connection();
}