/*
 * asm-x86/hvm/vmware.h
 *
 * Copyright (C) 2012-2015 Verizon Corporation
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License Version 2 (GPLv2)
 * as published by the Free Software Foundation.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details. <http://www.gnu.org/licenses/>.
 */

#ifndef ASM_X86_HVM_VMWARE_H__
#define ASM_X86_HVM_VMWARE_H__

#include <xen/types.h>

void cpuid_vmware_leaves(const struct vcpu *v, uint32_t leaf,
                         uint32_t subleaf, struct cpuid_leaf *res);

#endif /* ASM_X86_HVM_VMWARE_H__ */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
