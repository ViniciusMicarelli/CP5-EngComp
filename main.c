/*
 * CP2 - Sistema de Dados Robusto
 * Aluno: Vinícius Ribeiro Micarelli
 * RM: 86868
 * Curso: Engenharia da Computação - Sistemas de Tempo Real
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "esp_random.h"


#define TAG_ALUNO "Vinícius Ribeiro Micarelli RM86868"
#define PREFIXO_LOG  "{" TAG_ALUNO "}"

#define TASK_STACK   4096
#define TASK_PRIO    5
#define FILA_TAM     1

#define BIT_TASK1_OK (1 << 0)
#define BIT_TASK2_OK (1 << 1)


typedef struct {
    int id;
    int valor;
} dado_t;

static QueueHandle_t fila = NULL;
static EventGroupHandle_t event_supervisor = NULL;

void Task1(void *pvParameters)
{
    int seq = 1;
    for(;;)
    {
        dado_t *dados = (dado_t *) malloc(sizeof(dado_t));
        if (dados == NULL) {
            printf("%s [ERRO] Falha ao alocar memória dinâmica!\n", PREFIXO_LOG);
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        dados->id = seq++;
        dados->valor = (esp_random() % 15) + 1;

        if (xQueueSend(fila, &dados, 0) != pdTRUE) {
            printf("%s [FILA] Cheia - Dado %d descartado\n", PREFIXO_LOG, dados->valor);
            free(dados);
        } else {
            xEventGroupSetBits(event_supervisor, BIT_TASK1_OK);
            printf("%s [FILA] Dado %d (ID %d) enviado com sucesso\n", PREFIXO_LOG, dados->valor, dados->id);
        }

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void Task2(void *pvParameters)
{
    dado_t *dados_recebidos = NULL;
    int timeout = 0;

    for(;;)
    {
        if (xQueueReceive(fila, &dados_recebidos, pdMS_TO_TICKS(1000)) == pdTRUE)
        {
            printf("%s [FILA] Dado %d (ID %d) recebido com sucesso\n", PREFIXO_LOG,
                   dados_recebidos->valor, dados_recebidos->id);

            xEventGroupSetBits(event_supervisor, BIT_TASK2_OK);
            timeout = 0;

            free(dados_recebidos);
            dados_recebidos = NULL;
            esp_task_wdt_reset();
        }
        else
        {
            timeout++;
            printf("%s [TIMEOUT] Nenhum dado recebido (%d tentativas)\n", PREFIXO_LOG, timeout);

            if (timeout == 3)
                printf("%s [ALERTA] Task2 com falhas intermitentes!\n", PREFIXO_LOG);
            if (timeout >= 5) {
                printf("%s [RECUPERAÇÃO] Reinicializando contadores Task2\n", PREFIXO_LOG);
                timeout = 0;
            }
        }
    }
}

void Task3(void *pvParameters)
{
    for(;;)
    {
        EventBits_t bits = xEventGroupWaitBits(
            event_supervisor,
            BIT_TASK1_OK | BIT_TASK2_OK,
            pdTRUE,
            pdFALSE,
            pdMS_TO_TICKS(2000)
        );

        if ((bits & BIT_TASK1_OK) && (bits & BIT_TASK2_OK)) {
            printf("%s [SUPERVISOR] Sistema OK - Todas as tasks ativas\n", PREFIXO_LOG);
        }
        else if (bits & BIT_TASK1_OK) {
            printf("%s [SUPERVISOR] Sistema parcialmente OK - Apenas Task1 ativa\n", PREFIXO_LOG);
        }
        else if (bits & BIT_TASK2_OK) {
            printf("%s [SUPERVISOR] Sistema parcialmente OK - Apenas Task2 ativa\n", PREFIXO_LOG);
        }
        else {
            printf("%s [SUPERVISOR] Falha geral - Nenhuma task respondeu!\n", PREFIXO_LOG);
        }

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void app_main(void)
{
    printf("%s [SISTEMA] Inicializando Sistema Robusto CP2...\n", PREFIXO_LOG);

    const esp_task_wdt_config_t wdt_config = {
        .timeout_ms = 5000,
        .idle_core_mask = (1 << 0) | (1 << 1),
        .trigger_panic = true
    };
    esp_task_wdt_init(&wdt_config);

    fila = xQueueCreate(FILA_TAM, sizeof(dado_t *));
    event_supervisor = xEventGroupCreate();

    if (fila == NULL || event_supervisor == NULL) {
        printf("%s [ERRO] Falha ao criar fila ou grupo de eventos! Reiniciando...\n", PREFIXO_LOG);
        esp_restart();
    }

    TaskHandle_t t1, t2, t3;
    xTaskCreate(Task1, "Task1", TASK_STACK, NULL, TASK_PRIO, &t1);
    xTaskCreate(Task2, "Task2", TASK_STACK, NULL, TASK_PRIO, &t2);
    xTaskCreate(Task3, "Task3", TASK_STACK, NULL, TASK_PRIO, &t3);

    esp_task_wdt_add(t1);
    esp_task_wdt_add(t2);
    esp_task_wdt_add(t3);
}
