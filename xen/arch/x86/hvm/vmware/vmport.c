/*
 * HVM VMPORT emulation
 *
 * Copyright (C) 2012 Verizon Corporation
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

#include <xen/lib.h>
#include <asm/mc146818rtc.h>
#include <asm/hvm/hvm.h>
#include <asm/hvm/support.h>

#include "backdoor_def.h"

static int vmport_ioport(int dir, uint32_t port, uint32_t bytes, uint32_t *val)
{
    struct cpu_user_regs *regs = guest_cpu_user_regs();

#define port_overlap(p, n) \
    ((p + n > BDOOR_PORT) && (p + n <= BDOOR_PORT + 4) ? 1 : \
    (BDOOR_PORT + 4 > p) && (BDOOR_PORT + 4 <= p + n) ? 1 : 0)

    BUILD_BUG_ON(port_overlap(PIT_BASE, 4));
    BUILD_BUG_ON(port_overlap(0x61, 1));
    BUILD_BUG_ON(port_overlap(XEN_HVM_DEBUGCONS_IOPORT, 1));
    BUILD_BUG_ON(port_overlap(0xcf8, 4));
/* #define TMR_VAL_ADDR_V0  (ACPI_PM_TMR_BLK_ADDRESS_V0) */
    BUILD_BUG_ON(port_overlap(ACPI_PM_TMR_BLK_ADDRESS_V0, 4));
/* #define PM1a_STS_ADDR_V0 (ACPI_PM1A_EVT_BLK_ADDRESS_V0) */
    BUILD_BUG_ON(port_overlap(ACPI_PM1A_EVT_BLK_ADDRESS_V0, 4));
    BUILD_BUG_ON(port_overlap(RTC_PORT(0), 2));
    BUILD_BUG_ON(port_overlap(0x3c4, 2));
    BUILD_BUG_ON(port_overlap(0x3ce, 2));
/*
 * acpi_smi_cmd can not be checked at build time:
 *   xen/include/asm-x86/acpi.h:extern u32 acpi_smi_cmd;
 *   xen/arch/x86/acpi/boot.c: acpi_smi_cmd = fadt->smi_command;
 BUILD_BUG_ON(port_overlap(acpi_smi_cmd, 1));
*/
    BUILD_BUG_ON(port_overlap(0x20, 2));
    BUILD_BUG_ON(port_overlap(0xa0, 2));
    BUILD_BUG_ON(port_overlap(0x4d0, 1));
    BUILD_BUG_ON(port_overlap(0x4d1, 1));

    /*
     * While VMware expects only 32-bit in, they do support using
     * other sizes and out.  However they do require only the 1 port
     * and the correct value in eax.  Since some of the data
     * returned in eax is smaller the 32 bits and/or you only need
     * the other registers the dir and bytes do not need any
     * checking.  The caller will handle the bytes, and dir is
     * handled below for eax.
     */
    if ( port == BDOOR_PORT && regs->eax == BDOOR_MAGIC )
    {
        uint32_t new_eax = ~0u;
        uint64_t value;
        struct vcpu *curr = current;
        struct domain *currd = curr->domain;

        /*
         * VMware changes the other (non eax) registers ignoring dir
         * (IN vs OUT).  It also changes only the 32-bit part
         * leaving the high 32-bits unchanged, unlike what one would
         * expect to happen.
         */
        switch ( regs->ecx & 0xffff )
        {
        case BDOOR_CMD_GETMHZ:
            new_eax = currd->arch.tsc_khz / 1000;
            break;

        case BDOOR_CMD_GETVERSION:
            /* MAGIC */
            regs->ebx = BDOOR_MAGIC;
            /* VERSION_MAGIC */
            new_eax = 6;
            /* Claim we are an ESX. VMX_TYPE_SCALABLE_SERVER */
            regs->ecx = 2;
            break;

        case BDOOR_CMD_GETHWVERSION:
            /* vmware_hw */
            new_eax = currd->arch.hvm.vmware_hwver;
            /*
             * Returning zero is not the best.  VMware was not at
             * all consistent in the handling of this command until
             * VMware hardware version 4.  So it is better to claim
             * 4 then 0.  This should only happen in strange configs.
             */
            if ( !new_eax )
                new_eax = 4;
            break;

        case BDOOR_CMD_GETHZ:
        {
            struct segment_register sreg;

            hvm_get_segment_register(curr, x86_seg_ss, &sreg);
            if ( sreg.dpl == 0 )
            {
                value = currd->arch.tsc_khz * 1000;
                /* apic-frequency (bus speed) */
                regs->ecx = 1000000000ULL / APIC_BUS_CYCLE_NS;
                /* High part of tsc-frequency */
                regs->ebx = value >> 32;
                /* Low part of tsc-frequency */
                new_eax = value;
            }
            break;

        }
        case BDOOR_CMD_GETTIME:
            value = get_localtime_us(currd) -
                currd->time_offset.seconds * 1000000ULL;
            /* hostUsecs */
            regs->ebx = value % 1000000UL;
            /* hostSecs */
            new_eax = value / 1000000ULL;
            /* maxTimeLag */
            regs->ecx = 1000000;
            /* offset to GMT in minutes */
            regs->edx = currd->time_offset.seconds / 60;
            break;

        case BDOOR_CMD_GETTIMEFULL:
            /* BDOOR_MAGIC */
            new_eax = BDOOR_MAGIC;
            value = get_localtime_us(currd) -
                currd->time_offset.seconds * 1000000ULL;
            /* hostUsecs */
            regs->ebx = value % 1000000UL;
            /* hostSecs low 32 bits */
            regs->edx = value / 1000000ULL;
            /* hostSecs high 32 bits */
            regs->esi = (value / 1000000ULL) >> 32;
            /* maxTimeLag */
            regs->ecx = 1000000;
            break;

        default:
            /* Let backing DM handle */
            return X86EMUL_UNHANDLEABLE;
        }
        if ( dir == IOREQ_READ )
            *val = new_eax;
    }
    else if ( dir == IOREQ_READ )
        *val = ~0u;

    return X86EMUL_OKAY;
}

void vmport_register(struct domain *d)
{
    register_portio_handler(d, BDOOR_PORT, 4, vmport_ioport);
}

bool_t vmport_check_port(unsigned int port, unsigned int bytes)
{
    struct domain *currd = current->domain;

    return is_hvm_domain(currd) &&
           currd->arch.hvm.is_vmware_port_enabled &&
           (port >= BDOOR_PORT) && ((port + bytes) <= (BDOOR_PORT + 4));
}

/*
 * Local variables:
 * mode: C
 * c-set-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
