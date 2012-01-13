/*
 * linuxboothkvc_kernel.c
 * v11Jan2012_2254
 * HKVC, GPL, 04Jan2012_2105
 *
 * Move to core CPU in the SMP setup 
 * - Disable the non_core/other CPU(s) in a SMP setup (Have to think thro further)
 * Disable interrupts
 * Try and find 1:1 map 
 * - even 1 page should do initially
 * Copy a minimal code to this 1:1 map
 * Switch off MMU
 * retain control in itself (1st phase fantabulus success)
 * 
 * Else see if a reserved physical memory can be mapped in 1 to 1.
 *
 */

#include <linux/sched.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <asm/highmem.h>
#include <asm/fixmap.h>
#include <asm/page.h>
#include <linux/spinlock.h>

#include "gen_utils.h"
#include "uart_utils.h"

#include "hkvc.dummy1.h"

#define ENABLE_TESTBEFORE 1
#define ENABLE_PATCHUART 1
#define DISABLE_MMU_BEFORE_LEAVING 1
//#undef DISABLE_MMU_BEFORE_LEAVING

#define RSRVD_PHYS_ADDR1	0x9C000000 /* picked from fwram */
#define RSRVD_PHYS_ADDR2	0x9CF00000 /* Ducati baseimage physical address */
#define LBHKVC_MINOR 100
#define LBHKVC_VERSION "v11Jan2012_2255"

static DEFINE_SPINLOCK(main_lock);

#define S2S_BUFSIZE (32*1024)
#define O2O_BUFSIZE (32*1024)

#define EXEC_TYPE_ALLOC 0
#define EXEC_TYPE_FIXED 1
static unsigned long mpExecType = 0;
module_param(mpExecType, ulong, 0);

static unsigned long mpFixedExecVAddr = 0xc0008000;
module_param(mpFixedExecVAddr, ulong, 0);

static int mpTestBefore = 0;
module_param(mpTestBefore, int, 0);

static int mpPatchUart1 = 0;
module_param(mpPatchUart1, int, 0);

char s2s_simpbuf[S2S_BUFSIZE];
char o2o_simpbuf[O2O_BUFSIZE];
char *o2o_km, *s2s_km;

static long lbhkvc_ioctl(struct file *filp, unsigned int command, unsigned long arg);
static int lbhkvc_mmap(struct file *filp, struct vm_area_struct *v);

static const struct file_operations lbhkvc_fops = {
	.unlocked_ioctl   = lbhkvc_ioctl,
	.mmap             = lbhkvc_mmap,
};

static struct miscdevice lbhkvc_dev = {
	LBHKVC_MINOR,
	"lbhkvc",
	&lbhkvc_fops
};

/* Get those hidden kernel functions */
/* Unable to find init_mm, have to check source deeper */
#ifdef DEVICE_NOOKTAB
#warning "************* kh_??? = 0x... For device NOOKTAB"
int (*kh_ioremap_page)(unsigned long, unsigned long, void *) = (void*)0xc00464bc;
void (*kh_setup_mm_for_reboot)(char mode) = 0xc00469e8;
void (*kh_cpu_v7_proc_fin)(void) = 0xc0048784;
void (*kh_cpu_v7_reset)(long) = 0xc00487c0;
void (*kh_v7_coherent_kern_range)(unsigned long, unsigned long) = 0xc00480f0;
int (*kh_disable_nonboot_cpus)(void) = 0xc0083674;
void (*kh_show_pte)(struct mm_struct *mm, unsigned long addr) = 0xc00453d8;
#else
#warning "************ kh_??? = 0x... For device BEAGLEXM"
int (*kh_ioremap_page)(unsigned long, unsigned long, void *) = (void*)0xc004e4a4;
void (*kh_setup_mm_for_reboot)(char mode) = 0xc004ecc8;
void (*kh_cpu_v7_proc_fin)(void) = 0xc0050584;
void (*kh_cpu_v7_reset)(long) = 0xc00505a0;
void (*kh_v7_coherent_kern_range)(unsigned long, unsigned long) = 0xc004ffdc;
int (*kh_disable_nonboot_cpus)(void) = 0x0;
void (*kh_show_pte)(struct mm_struct *mm, unsigned long addr) = 0xc004db70;
#endif


int call_ioremap_page(unsigned long arg1, unsigned long arg2, void *arg3) 
{

	printk("arg1 = %lx\n",arg1);
	printk("arg2 = %lx\n",arg2);
	return kh_ioremap_page(arg1,arg2,arg3);
}


// __set_fixmap
//  less arch/arm/kernel/machine_kexec.c

static unsigned long vaddr_to_paddr(unsigned long addr, int kernel)
{
	pgd_t *pgdep;
	pmd_t *pmdep;
	pte_t *ptep, pte;
	spinlock_t *ptl;
#if 0
	if (!kernel) {
#endif
		pgdep = pgd_offset(current->mm, addr);
		if pgd_none(*pgdep) {
			printk(KERN_ERR "lbhkvc: NO PGD entry\n");
			return 0;
		}
		if pgd_bad(*pgdep) {
			printk(KERN_ERR "lbhkvc: BAD PGD entry\n");
			return 0;
		}
#if 0
	} else {
		//pgdep = pgd_offset_k(addr);
		printk(KERN_ERR "lbhkvc: pgd_offset_k not supported\n");
		return 0;
		if pgd_none(*pgdep) {
			printk(KERN_ERR "lbhkvc: NO PGD entry\n");
			return 0;
		}
		if pgd_bad(*pgdep) {
			printk(KERN_ERR "lbhkvc: BAD PGD entry\n");
			return 0;
		}
	}
#endif
	pmdep = pmd_offset(pgdep, addr);
	if pmd_none(*pmdep) {
		printk(KERN_ERR "lbhkvc: NO PMD entry\n");
		return 0;
	}
	if pmd_bad(*pmdep) {
		printk(KERN_ERR "lbhkvc: BAD PMD entry\n");
		return 0;
	}

	if(kernel) {
		ptep = pte_offset_kernel(pmdep, addr);
		if (!ptep) {
			printk(KERN_ERR "lbhkvc: valid PTE entry NOT found\n");
			return INT_MAX;
		}
		pte = *ptep;
		return pte;
	} else {
		ptep = pte_offset_map_lock(current->mm,pmdep, addr, &ptl);
		if (!ptep) {
			printk(KERN_ERR "lbhkvc: valid PTE entry NOT found\n");
			return INT_MAX;
		}
		pte = *ptep;
		pte_unmap_unlock(ptep,ptl);
		return pte;
	}
}

static void hkvc_meminfo_vaddr(unsigned long addr, char *sInfo)
{
	struct page *cPage;

	printk(KERN_INFO "\n***\n");
	printk(KERN_INFO "lbhkvc:MEMADDRESS: 0x%lx\t\t%s\n", addr, sInfo);
	printk(KERN_INFO "\t virt_to_phy : 0x%lx\n", virt_to_phys((void*)addr));
	cPage = virt_to_page(addr);
	printk(KERN_INFO "\t page_address V: 0x%p\n", page_address(cPage));
	printk(KERN_INFO "\t page_to_pfn  P: 0x%lx\n", page_to_pfn(cPage)<<PAGE_SHIFT);
	printk(KERN_INFO "\t PageTableWalk P: \n");
	kh_show_pte(NULL,addr);
	//printk(KERN_INFO "\t PAddr K(Walk) P: 0x%lx\n", vaddr_to_paddr(addr,1));
	printk(KERN_INFO "\t PAddr U(Walk) P: 0x%lx\n", vaddr_to_paddr(addr,0));
	printk(KERN_INFO "\n***\n");

}

static void dump_kernel_mm(void)
{
	long iTotalMem = 1024*1024*1024;

	printk(KERN_INFO "lbhkvc: iTotalMem 0x%lx (Hardwired) \n",
				iTotalMem);
	printk(KERN_INFO "lbhkvc: PAGE_SHIFT %d ie %d Page Size on this target \n",
				PAGE_SHIFT,0x1<<PAGE_SHIFT);
	printk(KERN_INFO "lbhkvc: PHYS_OFFSET P:0x%lx(D:0x%lx) Begining of Platform ram \n\n",
				PHYS_OFFSET, virt_to_phys((void*)PHYS_OFFSET));

	printk(KERN_INFO "lbhkvc: TASK_SIZE V:0x%lx(D:0x%lx) End of userspace mapping \n",
				TASK_SIZE, virt_to_phys((void*)TASK_SIZE));
	printk(KERN_INFO "lbhkvc: PKMAP_BASE V:0x%lx(?:0x%lx) Permanent kernal mappings \n",
				PKMAP_BASE, virt_to_phys((void*)PKMAP_BASE));
	printk(KERN_INFO "NOTE: PAGE_OFFSET P:addr above should match PHY_OFFSET P:addr\n");


	hkvc_meminfo_vaddr(PAGE_OFFSET,"PAGE_OFFSET - Kernel direct 1:1 map platform RamBeg ");
	hkvc_meminfo_vaddr((unsigned long)high_memory,"high_memory - Kernel direct 1:1 map platform RamEnd ");
	hkvc_meminfo_vaddr(FIXADDR_START,"FIXADDR_START - fixmap space Begin ");
	hkvc_meminfo_vaddr(FIXADDR_TOP,"FIXADDR_TOP - fixmap space End ");
	hkvc_meminfo_vaddr(MODULES_VADDR,"MODULES_VADDR - Start address for Modules Space ");
	hkvc_meminfo_vaddr(MODULES_END,"MODULES_END - End address for Modules Space ");
	hkvc_meminfo_vaddr(VMALLOC_START,"VMALLOC_START - vmalloc/ioremap space Begin ");
	hkvc_meminfo_vaddr(VMALLOC_END,"VMALLOC_END - vmalloc/ioremap space End ");
}

struct hkvc_mem_type {
        unsigned int prot_pte;
        unsigned int prot_l1;
        unsigned int prot_sect;
        unsigned int domain;
};


void get_mymem(void)
{
	long *kvaddr;
	long *kpaddr;
	long *klpaddr;
	struct hkvc_mem_type mt;

	s2s_km = kmalloc(S2S_BUFSIZE,GFP_KERNEL);
	if(!s2s_km)
		printk(KERN_ERR "lbhkvc: Failed to allocate s2s\n");
	o2o_km = kmalloc(O2O_BUFSIZE,GFP_KERNEL);
	if(!o2o_km)
		printk(KERN_ERR "lbhkvc: Failed to allocate o2o\n");

	hkvc_meminfo_vaddr((unsigned long)s2s_km,"s2s_km");
	hkvc_meminfo_vaddr((unsigned long)o2o_km,"o2o_km");

	kvaddr = (long*)o2o_km;
	kpaddr = (long*)virt_to_phys(o2o_km);
	klpaddr = kpaddr;
	*kvaddr = 0x1234f0f0;
	mt.prot_pte = L_PTE_PRESENT | L_PTE_USER | L_PTE_WRITE;
	call_ioremap_page((long)klpaddr,(long)kpaddr,&mt);
	printk(KERN_INFO "lbhkvc: *kvaddr = 0x%lx, *klpaddr = 0x%lx\n", *kvaddr, *klpaddr);

}

void dump_mymem(void)
{
	printk(KERN_INFO "lbhkvc: My s2s_simpbuf V:0x%p\n",s2s_simpbuf);
	hkvc_meminfo_vaddr((unsigned long)s2s_simpbuf,"Internal s2s_simpbuf");
	printk(KERN_INFO "lbhkvc: My o2o_simpbuf V:0x%p\n",o2o_simpbuf);
	hkvc_meminfo_vaddr((unsigned long)o2o_simpbuf,"Internal o2o_simpbuf");
}

/* Identified from kernel/machine_kexec.c and inturn looking at mm/proc-v7.S */
void hkvc_kexec_minimal(unsigned long kpaddr)
{
	unsigned long lTemp;

	hkvc_uart_send("A1\n",3);
	__asm__("DSB \n");
	__asm__("ISB \n");
	kh_cpu_v7_proc_fin();
	__asm__("ISB \n");
	hkvc_uart_send("A2\n",3);
	kh_setup_mm_for_reboot(0);
	__asm__("DSB \n");
	__asm__("ISB \n");
	hkvc_uart_send("A3\n",3);
	__asm__("mrc p15, 0, %0, c1, c0, 0\n"
		:"=r"(lTemp)
		);
	hkvc_uart_send_hex(lTemp);
	//kh_cpu_v7_reset(kpaddr);
	__asm__ ("mov r5,r5\n"
		 "mov r5,r5\n"
		 "mov r5,r5\n"
#ifdef DISABLE_MMU_BEFORE_LEAVING
#warning "********* ENABLED MMU Disable Before LEAVING\n"
		 "mrc p15, 0, r2, c1, c0, 0\n"
		 "bic r2, r2, #1\n"
		 "mrc p15, 0, r2, c1, c0, 0\n"
		 "mov r5,r5\n"
		 "ISB \n"
		 "mov r5,r5\n"
#else
#warning "********* DISABLED MMU Disable Before LEAVING\n"
#endif
		 "mov r5,r5\n"
		 "mov pc,%0\n"
		 "mov r5,r5\n"
		 "mov r5,r5\n"
		 "mov r5,r5\n"
		 "mov r5,r5\n"
		:
		: "r"(kpaddr)
		);
}
EXPORT_SYMBOL(hkvc_kexec_minimal);

#ifdef ENABLE_TESTBEFORE

void (*test_end_code)(void) = (void*) 0x0;

#endif

void hkvc_kexec_minimal_ext(unsigned long kpaddr, unsigned long kvaddr)
{
        kh_v7_coherent_kern_range( kvaddr, kvaddr+PAGE_SIZE);
#ifdef ENABLE_TESTBEFORE
	if(mpTestBefore) {
		printk(KERN_WARNING "Could CRASH\n");
		test_end_code = (void*)kvaddr;
		test_end_code();
	}
#endif
        printk(KERN_INFO "Bye!\n");
	hkvc_kexec_minimal(kpaddr);
}
EXPORT_SYMBOL(hkvc_kexec_minimal_ext);

void hkvc_kexec_alloc(void)
{
	long kpaddr;

	o2o_km = kmalloc(O2O_BUFSIZE,GFP_KERNEL);
	if(!o2o_km) {
		printk(KERN_ERR "lbhkvc: Failed to allocate o2o_km\n");
		return;
	} else {
		kpaddr = virt_to_phys(o2o_km);
		printk(KERN_INFO "lbhkvc: kmalloced at V:0x%p P:0x%lx\n", o2o_km, kpaddr);
	}
	//hkvc_meminfo_vaddr((unsigned long)o2o_km,"o2o_km");
        memcpy(o2o_km, hkvc_dummy1_bin, hkvc_dummy1_bin_len);
	hkvc_kexec_minimal_ext(kpaddr, (unsigned long)o2o_km);
}
EXPORT_SYMBOL(hkvc_kexec_alloc);

void hkvc_kexec_fixed(unsigned long kvaddr)
{
	unsigned long kpaddr;

	o2o_km = (void*)kvaddr;
	kpaddr = virt_to_phys(o2o_km);
	printk(KERN_INFO "lbhkvc: fixed kernel addr V:0x%p P:0x%lx\n", o2o_km, kpaddr);
	//hkvc_meminfo_vaddr((unsigned long)o2o_km,"o2o_km");
        memcpy(o2o_km, hkvc_dummy1_bin, hkvc_dummy1_bin_len);
	hkvc_kexec_minimal_ext(kpaddr, (unsigned long)o2o_km);
}
EXPORT_SYMBOL(hkvc_kexec_fixed);

static s32 __init lbhkvc_init(void)
{
	int ret;
	unsigned long flags;
	long *patchUartAddr;

	ret = misc_register(&lbhkvc_dev);
	if (ret) {
		printk(KERN_ERR "lbhkvc: misc_register failed on minor=%d\n",
			LBHKVC_MINOR);
		return ret;
	}
	printk(KERN_INFO "LBHKVC driver " LBHKVC_VERSION "\n");
	hkvc_uart_init();
	hkvc_uart_send("ABCD\n",5);
	printk(KERN_INFO "lbhkvc: cpu_possible_map = 0x%lx\n",*cpu_possible_map.bits);
	printk(KERN_INFO "lbhkvc: cpu_online_map = 0x%lx\n",*cpu_online_map.bits);
	printk(KERN_INFO "lbhkvc: smp_processor_id = %d\n",smp_processor_id());
#ifdef DEVICE_NOOKTAB
	printk(KERN_INFO "lbhkvc: hard_smp_processor_id = %d\n",hard_smp_processor_id());
#endif
	hkvc_sleep(0x20000000);

#ifdef ENABLE_PATCHUART
	patchUartAddr = (long*)&hkvc_dummy1_bin[hkvc_dummy1_bin_len-4];
	if (mpPatchUart1) {
		printk(KERN_WARNING "lbhkvc: Patching ASSUMED Uartaddress at end of code to run\n");
		printk(KERN_INFO "lbhkvc: Current UartAddress = 0x%lx\n",*patchUartAddr);
		*patchUartAddr = (long)uartPort;
		printk(KERN_INFO "lbhkvc: Patched UartAddress = 0x%lx\n",*patchUartAddr);
	}
#endif

	printk(KERN_INFO "lbhkvc: Enter hkvc_kexec\n");
	if(kh_disable_nonboot_cpus != NULL) {
		ret = kh_disable_nonboot_cpus();
		if (ret < 0) {
			printk(KERN_ERR "lbhkvc: disable_nonboot_cpus FAILED with %d(0x%x)\n", ret, ret);
			return 1;
		}
	}
	if(mpExecType == EXEC_TYPE_FIXED)
		hkvc_kexec_fixed(mpFixedExecVAddr);
	else
		hkvc_kexec_alloc();
	printk(KERN_INFO "lbhkvc: Left hkvc_kexec - WILL BE SURPRISED\n");

	dump_kernel_mm();
	get_mymem(); /* Before acquiring lock, otherwise can deadlock in future with larger memory requirements/swapping/... etc */
	spin_lock_irqsave(&main_lock,flags);
	dump_mymem();
	spin_unlock_irqrestore(&main_lock,flags);

	return 0;
}

static long lbhkvc_ioctl(struct file *filp, unsigned int command, unsigned long arg)
{
	return 0;
}

static int lbhkvc_mmap(struct file *filp, struct vm_area_struct *vma)
{
	return 0;
}

static void __exit lbhkvc_exit(void)
{
	misc_deregister(&lbhkvc_dev);
}

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("hkvc.india@gmail.com");
module_init(lbhkvc_init);
module_exit(lbhkvc_exit);

