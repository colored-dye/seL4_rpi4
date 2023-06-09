/*
 * Copyright 2020, Arm Ltd
 *
 * Copyright 2017, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the terms of
 * the GNU General Public License version 2. Note that NO WARRANTY is provided.
 * See "LICENSE_GPLv2.txt" for details.
 *
 * @TAG(DATA61_GPL)
 */

#include <autoconf.h>
#include <elfloader/gen_config.h>
#include <assembler.h>
#include <armv/assembler.h>

#if CONFIG_MAX_NUM_NODES > 1

.text

.extern core_entry
.extern core_stack
BEGIN_FUNC(core_entry_head)
    ldr x0, =core_stack
    ldr x3, [x0]
    mov x0, x3
    mov sp, x3
    b   core_entry
END_FUNC(core_entry_head)

#endif