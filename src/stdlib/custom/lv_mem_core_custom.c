/**
 * @file lv_mem_core_custom.c
 * @brief LVGL custom memory allocator usando PSRAM (ESP32-S3)
 * 
 * Este módulo implementa funciones de memoria custom para LVGL que
 * fuerzan la alocación en PSRAM para liberar SRAM interna.
 * 
 * Configuración requerida en sdkconfig:
 * CONFIG_LV_USE_CUSTOM_MALLOC=y
 * 
 * Basado en: components/lvgl/src/stdlib/clib/lv_mem_core_clib.c
 */

/*********************
 *      INCLUDES
 *********************/
#include "../lv_mem.h"
#if LV_USE_STDLIB_MALLOC == LV_STDLIB_CUSTOM

#include <stdlib.h>
#include <string.h>

#ifdef ESP_PLATFORM
#include "esp_heap_caps.h"
#include "esp_log.h"
#endif

/*********************
 *      DEFINES
 *********************/
#define TAG "lv_mem_psram"

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief Inicializa el sistema de memoria de LVGL
 * No se requiere inicialización especial para heap_caps
 */
void lv_mem_init(void)
{
#ifdef ESP_PLATFORM
    ESP_LOGI(TAG, "LVGL memory system initialized (using PSRAM via heap_caps)");
#endif
}

/**
 * @brief Desinicializa el sistema de memoria de LVGL
 * No se requiere cleanup especial para heap_caps
 */
void lv_mem_deinit(void)
{
#ifdef ESP_PLATFORM
    ESP_LOGI(TAG, "LVGL memory system deinitialized");
#endif
}

/**
 * @brief Agrega un pool de memoria (no soportado con heap_caps)
 * 
 * @param mem Puntero a memoria
 * @param bytes Tamaño del pool
 * @return NULL (no soportado)
 */
lv_mem_pool_t lv_mem_add_pool(void * mem, size_t bytes)
{
    LV_UNUSED(mem);
    LV_UNUSED(bytes);
#ifdef ESP_PLATFORM
    ESP_LOGW(TAG, "lv_mem_add_pool not supported with heap_caps allocator");
#endif
    return NULL;
}

/**
 * @brief Remueve un pool de memoria (no soportado con heap_caps)
 * 
 * @param pool Pool a remover
 */
void lv_mem_remove_pool(lv_mem_pool_t pool)
{
    LV_UNUSED(pool);
#ifdef ESP_PLATFORM
    ESP_LOGW(TAG, "lv_mem_remove_pool not supported with heap_caps allocator");
#endif
}

/**
 * @brief Aloca memoria en PSRAM para LVGL
 * 
 * Esta función es llamada por LVGL para todas las alocaciones de memoria
 * del pool interno (objetos, estilos, buffers internos, etc.)
 * 
 * @param size Tamaño en bytes a alocar
 * @return Puntero a memoria alocada o NULL si falla
 */
void* lv_malloc_core(size_t size)
{
    if (size == 0) {
        return NULL;
    }
    
#ifdef ESP_PLATFORM
    // Alocar en PSRAM para liberar SRAM
    void* ptr = heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
    
    if (ptr == NULL) {
        ESP_LOGE(TAG, "Failed to allocate %zu bytes in PSRAM (free PSRAM: %zu)", 
                 size, heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    }
    
    return ptr;
#else
    return malloc(size);
#endif
}

/**
 * @brief Realoca memoria en PSRAM para LVGL
 * 
 * @param p Puntero a memoria previa (puede ser NULL)
 * @param new_size Nuevo tamaño en bytes
 * @return Puntero a memoria realocada o NULL si falla
 */
void* lv_realloc_core(void* p, size_t new_size)
{
    // Si new_size es 0, equivale a free
    if (new_size == 0) {
        if (p != NULL) {
#ifdef ESP_PLATFORM
            heap_caps_free(p);
#else
            free(p);
#endif
        }
        return NULL;
    }
    
#ifdef ESP_PLATFORM
    // heap_caps_realloc maneja p==NULL correctamente (equivale a malloc)
    void* new_ptr = heap_caps_realloc(p, new_size, MALLOC_CAP_SPIRAM);
    
    if (new_ptr == NULL) {
        ESP_LOGE(TAG, "Failed to reallocate from %p to %zu bytes in PSRAM", p, new_size);
    }
    
    return new_ptr;
#else
    return realloc(p, new_size);
#endif
}

/**
 * @brief Libera memoria alocada por lv_malloc_core/lv_realloc_core
 * 
 * @param p Puntero a memoria a liberar
 */
void lv_free_core(void* p)
{
    if (p != NULL) {
#ifdef ESP_PLATFORM
        heap_caps_free(p);
#else
        free(p);
#endif
    }
}

/**
 * @brief Obtiene información de monitoreo de memoria
 * 
 * Reporta estadísticas del heap PSRAM para LVGL
 * 
 * @param mon_p Puntero a estructura de monitoreo a llenar
 */
void lv_mem_monitor_core(lv_mem_monitor_t * mon_p)
{
    if (mon_p == NULL) {
        return;
    }
    
#ifdef ESP_PLATFORM
    // Obtener información del heap PSRAM
    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_SPIRAM);
    
    mon_p->total_size = info.total_free_bytes + info.total_allocated_bytes;
    mon_p->free_cnt = 1; // No podemos saber número exacto de bloques libres
    mon_p->free_size = info.total_free_bytes;
    mon_p->free_biggest_size = info.largest_free_block;
    mon_p->used_cnt = 1; // No podemos saber número exacto de bloques usados
    mon_p->max_used = info.total_allocated_bytes;
    mon_p->used_pct = (mon_p->total_size > 0) ? 
                      (uint8_t)((info.total_allocated_bytes * 100) / mon_p->total_size) : 0;
    mon_p->frag_pct = 0; // ESP-IDF maneja fragmentación internamente
#else
    // Fallback para otras plataformas
    LV_UNUSED(mon_p);
#endif
}

/**
 * @brief Test de integridad de memoria (no soportado con heap_caps)
 * 
 * @return LV_RESULT_OK siempre (test no disponible)
 */
lv_result_t lv_mem_test_core(void)
{
    // heap_caps no provee función de test de integridad
    // El heap manager de ESP-IDF es confiable
    return LV_RESULT_OK;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /*LV_STDLIB_CUSTOM*/
