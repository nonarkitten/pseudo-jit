/*
 * pinmux.c
 *
 *  Created on: May 13, 2021
 *      Author: renee.cousins
 */

#include "pinmux.h"

void config_mux(pin_muxing_t* pin_mux) {
	while(pin_mux->offset != 0xffffffff) {
		*(volatile uint32_t*)(CFG_MOD_BASE + pin_mux->offset) =
				(pin_mux->val);
		pin_mux++;
	}
}

