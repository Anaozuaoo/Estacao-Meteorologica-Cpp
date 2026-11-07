#ifndef STM32_HAL_COMPAT_H
#define STM32_HAL_COMPAT_H

#include <cstdint>
#include <cstddef>

/**
 * @file stm32_hal_compat.h
 * @brief STM32 HAL compatibility header.
 *
 * In an actual STM32CubeIDE or STM32 firmware project, this file includes the
 * family-specific HAL header (e.g., <stm32f4xx_hal.h>, <stm32f1xx_hal.h>,
 * <stm32g4xx_hal.h>, or <stm32l4xx_hal.h>).
 *
 * For host-based builds and static verification without vendor SDKs installed,
 * lightweight fallback definitions are provided below.
 */

#if defined(STM32F4) || defined(STM32F401xE) || defined(STM32F411xE)
#include "stm32f4xx_hal.h"
#elif defined(STM32F1) || defined(STM32F103xB)
#include "stm32f1xx_hal.h"
#elif defined(STM32L4)
#include "stm32l4xx_hal.h"
#elif defined(STM32G4)
#include "stm32g4xx_hal.h"
#else

// Compatibility HAL definitions for cross-compilation and simulation
typedef enum {
    HAL_OK       = 0x00U,
    HAL_ERROR    = 0x01U,
    HAL_BUSY     = 0x02U,
    HAL_TIMEOUT  = 0x03U
} HAL_StatusTypeDef;

#define I2C_MEMADD_SIZE_8BIT  0x00000001U
#define I2C_MEMADD_SIZE_16BIT 0x00000002U

// Forward declare HAL handle structs
struct I2C_HandleTypeDef {
    void* Instance;
    uint32_t ErrorCode;
};

struct UART_HandleTypeDef {
    void* Instance;
    uint32_t ErrorCode;
};

// Weak fallback functions for host simulation
#if defined(__cplusplus)
extern "C" {
#endif

HAL_StatusTypeDef HAL_Init(void);
HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_I2C_IsDeviceReady(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Trials, uint32_t Timeout);
HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *huart, const uint8_t *pData, uint16_t Size, uint32_t Timeout);
void HAL_Delay(uint32_t Delay);
uint32_t HAL_GetTick(void);

#if defined(__cplusplus)
}
#endif

#endif // STM32 families

#endif // STM32_HAL_COMPAT_H
