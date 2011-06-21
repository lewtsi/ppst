#ifndef AX_WDG_H
#define AX_WDG_H

#include "ax_common.h"

#define AX_BOOT_NORMAL		((uint8_t)0xB0)
#define AX_BOOT_IWDG		((uint8_t)0xB1)

#define ax_iwdg_reset()		IWDG_ReloadCounter()

void ax_iwdg_init(void);
opt_result_t ax_iwdg_system_boot_is_iwdg_reset(void);

#endif

/* EOF */

