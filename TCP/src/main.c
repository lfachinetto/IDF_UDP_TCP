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
#include <freertos/task.h>
#include <lwip/sockets.h>

const char *ssid = "";
const char *pass = "";

const char *ip = "192.168.18.67";
const int port = 8000;

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

    while (1)
    {
        int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        struct sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        inet_pton(AF_INET, ip, &serverAddress.sin_addr.s_addr);
        serverAddress.sin_port = htons(port);

        int rc = connect(sock, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr_in));
        if (rc == -1)
        {
            printf("Erro de conexao, esperando 5 segundos...\n");
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }
        else
            printf("Conectado!\n");

        char data[50];
        int random = rand() % 1000;
        sprintf(data, "Passo Fundo %d", random);

        rc = send(sock, data, strlen(data), 0);
        printf("Enviado: %s\n", data);

        char buffer[100];
        rc = recv(sock, buffer, 100, 0);
        printf("Recebido: %s\n", buffer);

        rc = close(sock);

        printf("Esperando 5 segundos...\n\n");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}