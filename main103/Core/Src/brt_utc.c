/* File brt_utc.c */

/*#ifndef stm32f1xx_hal_flash
#include "stm32f1xx_hal_flash.h"
#endif

#ifndef stm32f1xx_hal_flash_ex
#include "stm32f1xx_hal_flash_ex.h"
#endif*/
//#include <stm32f1xx_hal_def.h>


#include "brt_utc.h"

uint64_t address_data = APP_START_ADDRESS;
uint32_t flag_addr = 0x0800FC00;
uint32_t flag_data = 0x12345678;

//extern void HAL_FLASH_Unlock(void);
//extern HAL_StatusTypeDef HAL_FLASH_Unlock(void);
//extern void HAL_FLASH_Lock(void);
//extern void HAL_NVIC_SystemReset(void);

/*void set_overwrite_flag(void) {
    HAL_FLASH_Unlock(); \
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, flag_addr, flag_data); \
    HAL_FLASH_Lock(); \
    HAL_NVIC_SystemReset();
}
*/


