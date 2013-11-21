/*
 * arch/x86/hvm/vmware/cpuid.c
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

#include <xen/sched.h>
#include <xen/version.h>
#include <xen/hypercall.h>
#include <xen/domain_page.h>
#include <xen/param.h>
#include <asm/guest_access.h>
#include <asm/guest/hyperv-tlfs.h>
#include <asm/paging.h>
#include <asm/p2m.h>
#include <asm/apic.h>
#include <asm/hvm/support.h>
#include <public/sched.h>
#include <public/hvm/hvm_op.h>

/*
 * VMware hardware version 7 defines some of these cpuid levels,
 * below is a brief description about those.
 *
 *     Leaf 0x40000000, Hypervisor CPUID information
 * # EAX: The maximum input value for hypervisor CPUID info (0x40000010).
 * # EBX, ECX, EDX: Hypervisor vendor ID signature. E.g. "VMwareVMware"
 *
 *     Leaf 0x40000010, Timing information.
 * # EAX: (Virtual) TSC frequency in kHz.
 * # EBX: (Virtual) Bus (local apic timer) frequency in kHz.
 * # ECX, EDX: RESERVED
 */

void cpuid_vmware_leaves(const struct vcpu *v, uint32_t leaf,
                         uint32_t subleaf, struct cpuid_leaf *res)
{
    struct domain *d = current->domain;

    ASSERT(has_vmware_cpuid(d));
    ASSERT(leaf >= 0x40000000 && leaf < 0x40000100);

    leaf -= 0x40000000;

    switch ( leaf )
    {
    case 0x0:
        res->a = 0x40000010; /* Maximum leaf */
        memcpy(&res->b, "VMwa", 4);
        memcpy(&res->c, "reVM", 4);
        memcpy(&res->d, "ware", 4);
        break;

    case 0x10:
        /* (Virtual) TSC frequency in kHz. */
        res->a = d->arch.tsc_khz;
        /* (Virtual) Bus (local apic timer) frequency in kHz. */
        res->b = 1000000ull / APIC_BUS_CYCLE_NS;
        res->c = 0;          /* Reserved */
        res->d = 0;          /* Reserved */
        break;
    }
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
