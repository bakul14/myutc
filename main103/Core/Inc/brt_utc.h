/* File brt_utc.h */

#ifndef 	stdint
#include	<stdint.h>
#endif


#define APP_START_ADDRESS 0x08002400  //ADDR of main programm memory to jump
		
extern uint64_t address_data;
extern uint32_t flag_addr;
extern uint32_t flag_data;

//extern void set_overwrite_flag(void);


