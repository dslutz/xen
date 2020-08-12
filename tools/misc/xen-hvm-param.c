/*
 * tools/misc/xen-hvm-param.c
 *
 * Copyright (C) 2014 Verizon Corporation
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

#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include <xenctrl.h>


int
main(int argc, char **argv)
{
    xc_interface *xch;
    int domid;
    int start_param = 0;
    int end_param = HVM_NR_PARAMS;
    int param;
    int ret = 0;
    int i;
    char hvm_param_name[HVM_NR_PARAMS][80];

    unsigned long hvm_param = -1;

    if ( (argc < 2) || (argc > 4) )
        errx(1, "usage: %s domid [param [new]]", argv[0]);

    for ( i = 0; i < HVM_NR_PARAMS; i++ )
        snprintf(hvm_param_name[i], sizeof(hvm_param_name[i]),
                 "Unknown %d", i);

    snprintf(hvm_param_name[HVM_PARAM_PAE_ENABLED],
             sizeof(hvm_param_name[HVM_PARAM_PAE_ENABLED]),
             "deprecated pae_enabled");
    snprintf(hvm_param_name[HVM_PARAM_DM_DOMAIN],
             sizeof(hvm_param_name[HVM_PARAM_DM_DOMAIN]),
             "deprecated dm_domain");
    snprintf(hvm_param_name[HVM_PARAM_MEMORY_EVENT_CR0],
             sizeof(hvm_param_name[HVM_PARAM_MEMORY_EVENT_CR0]),
             "deprecated memory_event_cr0");
    snprintf(hvm_param_name[HVM_PARAM_MEMORY_EVENT_CR3],
             sizeof(hvm_param_name[HVM_PARAM_MEMORY_EVENT_CR3]),
             "deprecated memory_event_cr3");
    snprintf(hvm_param_name[HVM_PARAM_MEMORY_EVENT_CR4],
             sizeof(hvm_param_name[HVM_PARAM_MEMORY_EVENT_CR4]),
             "deprecated memory_event_cr4");
    snprintf(hvm_param_name[HVM_PARAM_MEMORY_EVENT_INT3],
             sizeof(hvm_param_name[HVM_PARAM_MEMORY_EVENT_INT3]),
             "deprecated memory_event_int3");
    snprintf(hvm_param_name[HVM_PARAM_MEMORY_EVENT_SINGLE_STEP],
             sizeof(hvm_param_name[HVM_PARAM_MEMORY_EVENT_SINGLE_STEP]),
             "deprecated memory_event_single_step");
    snprintf(hvm_param_name[HVM_PARAM_BUFIOREQ_EVTCHN],
             sizeof(hvm_param_name[HVM_PARAM_BUFIOREQ_EVTCHN]),
             "deprecated bufioreq_evtchn");
    snprintf(hvm_param_name[HVM_PARAM_MEMORY_EVENT_MSR],
             sizeof(hvm_param_name[HVM_PARAM_MEMORY_EVENT_MSR]),
             "deprecated memory_event_msr");

    snprintf(hvm_param_name[HVM_PARAM_CALLBACK_IRQ],
             sizeof(hvm_param_name[HVM_PARAM_CALLBACK_IRQ]), "callback_irq");
    snprintf(hvm_param_name[HVM_PARAM_STORE_PFN],
             sizeof(hvm_param_name[HVM_PARAM_STORE_PFN]), "store_pfn");
    snprintf(hvm_param_name[HVM_PARAM_STORE_EVTCHN],
             sizeof(hvm_param_name[HVM_PARAM_STORE_EVTCHN]), "store_evtchn");
    snprintf(hvm_param_name[HVM_PARAM_VMPORT_DEBUG],
             sizeof(hvm_param_name[HVM_PARAM_VMPORT_DEBUG]),
             "vmport_debug");
    snprintf(hvm_param_name[HVM_PARAM_IOREQ_PFN],
             sizeof(hvm_param_name[HVM_PARAM_IOREQ_PFN]), "ioreq_pfn");
    snprintf(hvm_param_name[HVM_PARAM_BUFIOREQ_PFN],
             sizeof(hvm_param_name[HVM_PARAM_BUFIOREQ_PFN]), "bufioreq_pfn");
    snprintf(hvm_param_name[HVM_PARAM_VMPORT_REGS_PFN],
             sizeof(hvm_param_name[HVM_PARAM_VMPORT_REGS_PFN]),
             "vmport_regs_pfn");
    snprintf(hvm_param_name[HVM_PARAM_VIRIDIAN],
             sizeof(hvm_param_name[HVM_PARAM_VIRIDIAN]), "viridian");
    snprintf(hvm_param_name[HVM_PARAM_TIMER_MODE],
             sizeof(hvm_param_name[HVM_PARAM_TIMER_MODE]), "timer_mode");
    snprintf(hvm_param_name[HVM_PARAM_HPET_ENABLED],
             sizeof(hvm_param_name[HVM_PARAM_HPET_ENABLED]), "hpet_enabled");
    snprintf(hvm_param_name[HVM_PARAM_IDENT_PT],
             sizeof(hvm_param_name[HVM_PARAM_IDENT_PT]), "ident_pt");
    snprintf(hvm_param_name[HVM_PARAM_ACPI_S_STATE],
             sizeof(hvm_param_name[HVM_PARAM_ACPI_S_STATE]), "acpi_s_state");
    snprintf(hvm_param_name[HVM_PARAM_VM86_TSS],
             sizeof(hvm_param_name[HVM_PARAM_VM86_TSS]), "vm86_tss");
    snprintf(hvm_param_name[HVM_PARAM_VPT_ALIGN],
             sizeof(hvm_param_name[HVM_PARAM_VPT_ALIGN]), "vpt_align");
    snprintf(hvm_param_name[HVM_PARAM_CONSOLE_PFN],
             sizeof(hvm_param_name[HVM_PARAM_CONSOLE_PFN]), "console_pfn");
    snprintf(hvm_param_name[HVM_PARAM_CONSOLE_EVTCHN],
             sizeof(hvm_param_name[HVM_PARAM_CONSOLE_EVTCHN]),
             "console_evtchn");
    snprintf(hvm_param_name[HVM_PARAM_ACPI_IOPORTS_LOCATION],
             sizeof(hvm_param_name[HVM_PARAM_ACPI_IOPORTS_LOCATION]),
             "acpi_ioports_location");
    snprintf(hvm_param_name[HVM_PARAM_NESTEDHVM],
             sizeof(hvm_param_name[HVM_PARAM_NESTEDHVM]), "nestedhvm");
    snprintf(hvm_param_name[HVM_PARAM_PAGING_RING_PFN],
             sizeof(hvm_param_name[HVM_PARAM_PAGING_RING_PFN]),
             "paging_ring_pfn");
    snprintf(hvm_param_name[HVM_PARAM_MONITOR_RING_PFN],
             sizeof(hvm_param_name[HVM_PARAM_MONITOR_RING_PFN]),
             "monitor_ring_pfn");
    snprintf(hvm_param_name[HVM_PARAM_SHARING_RING_PFN],
             sizeof(hvm_param_name[HVM_PARAM_SHARING_RING_PFN]),
             "sharing_ring_pfn");
    snprintf(hvm_param_name[HVM_PARAM_TRIPLE_FAULT_REASON],
             sizeof(hvm_param_name[HVM_PARAM_TRIPLE_FAULT_REASON]),
             "triple_fault_reason");
    snprintf(hvm_param_name[HVM_PARAM_IOREQ_SERVER_PFN],
             sizeof(hvm_param_name[HVM_PARAM_IOREQ_SERVER_PFN]),
             "ioreq_server_pfn");
    snprintf(hvm_param_name[HVM_PARAM_NR_IOREQ_SERVER_PAGES],
             sizeof(hvm_param_name[HVM_PARAM_NR_IOREQ_SERVER_PAGES]),
             "nr_ioreq_server_pages");
    snprintf(hvm_param_name[HVM_PARAM_VM_GENERATION_ID_ADDR],
             sizeof(hvm_param_name[HVM_PARAM_VM_GENERATION_ID_ADDR]),
             "vm_generation_id_addr");
    snprintf(hvm_param_name[HVM_PARAM_ALTP2M],
             sizeof(hvm_param_name[HVM_PARAM_ALTP2M]),
             "altp2m");
    snprintf(hvm_param_name[HVM_PARAM_X87_FIP_WIDTH],
             sizeof(hvm_param_name[HVM_PARAM_X87_FIP_WIDTH]),
             "x87_fip_width");
    snprintf(hvm_param_name[HVM_PARAM_VM86_TSS_SIZED],
             sizeof(hvm_param_name[HVM_PARAM_VM86_TSS_SIZED]),
             "vm86_tss_sized");
    snprintf(hvm_param_name[HVM_PARAM_MCA_CAP],
             sizeof(hvm_param_name[HVM_PARAM_MCA_CAP]),
             "mca_cap");

    xch = xc_interface_open(0, 0, 0);
    if ( !xch )
        err(1, "failed to open control interface");

    domid = atoi(argv[1]);
    if ( argc > 2 )
    {
        start_param = strtol(argv[2], NULL, 0);
        end_param = start_param + 1;
    }

    for ( param = start_param; param < end_param; param++ )
    {
        ret = xc_get_hvm_param(xch, domid, param, &hvm_param);
        if ( ret )
            if ( (param >= 0) && (param < HVM_NR_PARAMS) )
                printf("hvm_param(%d:'%s') get failed for domid %d\n",
                       param, hvm_param_name[param], domid);
            else
                err(1, "failed to get hvm param %d for domid %d", param, domid);
        else
        {
            if ( argc == 4 )
            {
                long new = strtol(argv[3], NULL, 0);

                ret = xc_set_hvm_param(xch, domid, param, new);
                if ( ret )
                    err(1, "failed to set hvm param %d for domid %d", param,
                        domid);
                else if ( (param >= 0) && (param < HVM_NR_PARAMS) )
                    printf("hvm_param(%d:'%s')=0x%lx(%ld) was 0x%lx(%ld)\n",
                           param, hvm_param_name[param], new, new, hvm_param,
                           hvm_param);
                else
                    printf("hvm_param(%d)=0x%lx(%ld) was 0x%lx(%ld)\n",
                           param, new, new, hvm_param, hvm_param);
            }
            else
            {
                if ( (param >= 0) && (param < HVM_NR_PARAMS) )
                    printf("hvm_param(%d:'%s')=0x%lx(%ld)\n",
                           param, hvm_param_name[param], hvm_param, hvm_param);
                else
                    printf("hvm_param(%d)=0x%lx(%ld)\n", param, hvm_param,
                           hvm_param);
            }
        }
    }
    xc_interface_close(xch);

    return ret;
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
