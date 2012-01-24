/*
 * linuxboothkvc_kernel.c
 * v24Jan2012_1943
 * HKVC, GPL, 04Jan2012_2105
 *
 * UPDATE: moving away from cunning/intelligent MVA based cache operations to
 * set/way based once solved the cache flush not reaching memory issue.
 *
 * NOTE: Partly Old comments below
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
#include <asm/cacheflush.h>
#include <asm/uaccess.h>
#include <linux/spinlock.h>
#include <asm/hardware/cache-l2x0.h>
#include <mach/omap4-common.h>
#include <plat/omap44xx.h>
#include <linux/io.h>

#include "gen_utils.h"
#include "uart_utils.h"

#include "hkvc.nirvana1.h"
#include "hkvc.nchild.h"

#define ENABLE_TESTBEFORE 1
#define ENABLE_PATCHUART 1
// NOTE: As beaglexm has only 256 or 512MB of ram and inturn as the
// kernel modules are stored some where around 0xbf00.0000 or beyond
// the setup_mm_for_reboot won't map this virtual address to equivalent
// physical address, as there is no memory to map to at this address.
// So when disabling mmu from within the kernel module before passing
// control to Nirvana code, it will fail, as there is non 1-to-1
// or same-to-same mapping at this address where mmu gets disabled.
// So undefing the MMU_DISABLE_BEFORELEAVING logic
//#define ENABLE_MMU_DISABLE_BEFORELEAVING 1
#undef ENABLE_MMU_DISABLE_BEFORELEAVING 

#define RSRVD_PHYS_ADDR1	0x9C000000 /* picked from fwram */
#define RSRVD_PHYS_ADDR2	0x9CF00000 /* Ducati baseimage physical address */
#define LBHKVC_MINOR 100
#define LBHKVC_VERSION "v24Jan2012_1942-"__TIME__

static DEFINE_SPINLOCK(main_lock);

#define NCHILD_BUFSIZE (32*1024)
#define NIRVANA_BUFSIZE (32*1024)

#define EXEC_TYPE_ALLOC 0
#define EXEC_TYPE_FIXED 1
static unsigned long mpExecType = 0;
module_param(mpExecType, ulong, 0);

static unsigned long mpFixedNirvanaVAddr = 0xc0008000;
module_param(mpFixedNirvanaVAddr, ulong, 0);

static int mpTestBefore = 0;
module_param(mpTestBefore, int, 0);

static int mpPatchUart1 = 0;
module_param(mpPatchUart1, int, 0);

char simpbufNChild[NCHILD_BUFSIZE];
char simpbufNirvana[NIRVANA_BUFSIZE];
char *kmNirvana, *kmNChild;

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
#elif defined(DEVICE_BEAGLEXM)
#warning "************ kh_??? = 0x... For device BEAGLEXM"
int (*kh_ioremap_page)(unsigned long, unsigned long, void *) = (void*)0xc004e4a4;
void (*kh_setup_mm_for_reboot)(char mode) = 0xc004ecc8;
void (*kh_cpu_v7_proc_fin)(void) = 0xc0050584;
void (*kh_cpu_v7_reset)(long) = 0xc00505a0;
void (*kh_v7_coherent_kern_range)(unsigned long, unsigned long) = 0xc004ffdc;
int (*kh_disable_nonboot_cpus)(void) = 0x0;
void (*kh_show_pte)(struct mm_struct *mm, unsigned long addr) = 0xc004db70;
#elif defined(DEVICE_PANDA)
#warning "************ kh_??? = 0x... For device PANDA"
int (*kh_ioremap_page)(unsigned long, unsigned long, void *) = 0xc003ff7c;
void (*kh_setup_mm_for_reboot)(char mode) = 0xc00404b4;
void (*kh_cpu_v7_proc_fin)(void) = 0xc00422e4;
void (*kh_cpu_v7_reset)(long) = 0xc0042320;
void (*kh_v7_coherent_kern_range)(unsigned long, unsigned long) = 0xc0041c50;
int (*kh_disable_nonboot_cpus)(void) = 0xc0076528;
void (*kh_show_pte)(struct mm_struct *mm, unsigned long addr) = 0xc003ee50;
#else
#error "UNKNOWN DEVICE..."
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

	kmNChild = kmalloc(NCHILD_BUFSIZE,GFP_KERNEL);
	if(!kmNChild)
		printk(KERN_ERR "lbhkvc: Failed to allocate NChild\n");
	kmNirvana = kmalloc(NIRVANA_BUFSIZE,GFP_KERNEL);
	if(!kmNirvana)
		printk(KERN_ERR "lbhkvc: Failed to allocate Nirvana\n");

	hkvc_meminfo_vaddr((unsigned long)kmNChild,"kmNChild");
	hkvc_meminfo_vaddr((unsigned long)kmNirvana,"kmNirvana");

	kvaddr = (long*)kmNirvana;
	kpaddr = (long*)virt_to_phys(kmNirvana);
	klpaddr = kpaddr;
	*kvaddr = 0x1234f0f0;
	mt.prot_pte = L_PTE_PRESENT | L_PTE_USER | L_PTE_WRITE;
	call_ioremap_page((long)klpaddr,(long)kpaddr,&mt);
	printk(KERN_INFO "lbhkvc: *kvaddr = 0x%lx, *klpaddr = 0x%lx\n", *kvaddr, *klpaddr);

}

void dump_mymem(void)
{
	printk(KERN_INFO "lbhkvc: My simpbufNChild V:0x%p\n",simpbufNChild);
	hkvc_meminfo_vaddr((unsigned long)simpbufNChild,"Internal simpbufNChild");
	printk(KERN_INFO "lbhkvc: My simpbufNirvana V:0x%p\n",simpbufNirvana);
	hkvc_meminfo_vaddr((unsigned long)simpbufNirvana,"Internal simpbufNirvana");
}

unsigned long hkvc_mem_touch(void *kva, int len)
{
	unsigned long *pBuf = (unsigned long*)kva;
	int i;
	volatile unsigned long lTouch;
	for(i=0; i<(len/4); i++) {
		lTouch += pBuf[i];
	}
	return lTouch;
}

void print_v2p_mapping(char *sMsg, int iMsgLen, unsigned long addr)
{
	hkvc_uart_send(sMsg,iMsgLen);
	hkvc_uart_send_hex(addr);
	hkvc_uart_send(" v2p ",5);
	hkvc_uart_send_hex(va2pa_cpr(addr));
	hkvc_uart_send("\n\r",2);
}

void hkvc_flush_all(void);

/* Identified from
	kernel/machine_kexec.c (for now, most can be and will be replaced with internal code in a future version)
	mm/proc-v7.S
	mm/cache-v7.S
	and lot of others */
void hkvc_kexec_minimal(unsigned long kpaNirvana, unsigned long kpaNChild, unsigned long nchildLen)
{
	unsigned long lTemp;
	unsigned long flags;

	spin_lock_irqsave(&main_lock,flags);
	hkvc_uart_send("A1IrqSave\n",10);
	__asm__("DSB \n");
	__asm__("ISB \n");
	kh_cpu_v7_proc_fin();
	__asm__("ISB \n");
	/* ICIALLU, Instruction cache invalidate all to PoU. Ignores Rt value. */
	__asm__("MCR p15, 0, r0, c7, c5, 0 \n");
	hkvc_uart_send("A2ProcFin\n",10);
	hkvc_flush_all();

	print_v2p_mapping("kpaNirvana: ",12,kpaNirvana);
	print_v2p_mapping("kpaNChild : ",12,kpaNChild);
	print_v2p_mapping("NChildELoc: ",12,0x40304350);

	kh_setup_mm_for_reboot(0);
	__asm__("DSB \n");
	__asm__("ISB \n");
	hkvc_uart_send("A3SetupMm\n",10);

	print_v2p_mapping("kpaNirvana: ",12,kpaNirvana);
	print_v2p_mapping("kpaNChild : ",12,kpaNChild);
	print_v2p_mapping("NChildELoc: ",12,0x40304350);
	//hkvc_uart_send("\n\r",2);

	__asm__("mrc p15, 0, %0, c1, c0, 0\n"
		:"=r"(lTemp)
		);
	hkvc_uart_send_hex(lTemp);
	hkvc_uart_send("\n\r",2);
	hkvc_uart_send_hex(lTemp);
	hkvc_uart_send("\n\r",2);
	//kh_cpu_v7_reset(kpaNirvana);
	__asm__ ("mov r0,r0\n"
		 "mov r0,r0\n"
		 //"bkpt #1\n"	// qemu and gdb target remote combination doesn't trap this, have to check with openocd yet
		 "mov r0,r0\n"
#ifdef ENABLE_MMU_DISABLE_BEFORELEAVING
#warning "********* MMU Disabled in preperation for Nirvana code\n"
		 "mrc p15, 0, r2, c1, c0, 0\n"
		 "bic r2, r2, #1\n"
		 "mcr p15, 0, r2, c1, c0, 0\n"
		 "mov r0,r0\n"
		 "ISB \n"
		 "mov r0,r0\n"
#else
#warning "********* MMU NOT Disabled in preperation for Nirvana code\n"
#endif
		 "mov r0,r0\n"
		 "mov r0,%1\n"
		 "mov r1,%2\n"
		 "mov pc,%0\n"
		 "mov r0,r0\n"
		 "mov r0,r0\n"
		 "mov r5,r5\n"
		 "mov r5,r5\n"
		:
		: "r"(kpaNirvana), "r"(kpaNChild), "r"(nchildLen)
		);
}
EXPORT_SYMBOL(hkvc_kexec_minimal);

#ifdef ENABLE_TESTBEFORE

void (*test_end_code)(void) = (void*) 0x0;

#endif

#ifdef CONFIG_CACHE_L2X0
void __iomem *myL2CacheBase = NULL;
#endif

void hkvc_flush_init(void)
{
#ifdef CONFIG_CACHE_L2X0
	myL2CacheBase=ioremap(OMAP44XX_L2CACHE_BASE,SZ_4K);
	BUG_ON(!myL2CacheBase);
	printk(KERN_INFO "lbhkvc: l2cache base = 0x%p\n", myL2CacheBase);
#endif

	printk( KERN_INFO "lbhkvc: get_fs() = 0x%lx AND KERNEL_DS = 0x%lx\n",get_fs(),(unsigned long)KERNEL_DS);
	hkvc_sleep(0x20000000);

	// set_fs - doesn't seem to affect flush_icache_range based on reading of the code, but not sure why generic
	// module loading uses it. May be in some other architecture there is a reference to this. Also haven't checked
	// set/way based logic fully yet beyond my immidiate need (again dont see need for this), so keeping it for now.
	set_fs(KERNEL_DS);
	// The set/way based logic which walks thro multiple cache levels and flushes all cached data which includes
	// among others the properly resolved/linked kernel module code as well as my Nirvana and NChild code.
}

void hkvc_flush_all(void)
{
#warning "Executing Set/Way based Cache flush"
	__cpuc_flush_kern_all();
#ifdef CONFIG_CACHE_L2X0
#warning "Executing L2X0 cache flush"
	__asm__("dsb\n"
		"dmb\n"
		"isb\n"
		"nop\n"
		"nop\n"
		"ldr     r0, =0xffff\n"
		"str     r0, [%0, %2]\n"
		"2:\n"
		"ldr     r0, [%0, %2]\n"
		"cmp     r0, #0\n"
		"bne     2b\n"
		"nop\n"
		"nop\n"
		"mov     r0, #0x0\n"
		"str     r0, [%0, %1]\n"
		"1:\n"
		"ldr     r0, [%0, %1]\n"
		"ands    r0, r0, #0x1\n"
		"bne     1b\n"
		"nop\n"
		"nop\n"
		:
		: "r"(myL2CacheBase), "J"(L2X0_CACHE_SYNC), "J"(L2X0_CLEAN_INV_WAY)
		: "r0");
#else
#warning "WARN: NO NO NO L2X0 cache flush"
#endif
	__cpuc_flush_kern_all();
	__asm__("dsb\n");

}

void hkvc_kexec_minimal_ext(unsigned long kpaNirvana, unsigned long kvaNirvana,
				unsigned long kpaNChild, unsigned long kvaNChild, unsigned long childLen)
{

	hkvc_flush_init();
	hkvc_flush_all();

#ifdef OLD_DIRECT_FLUSH_ICACHE_RANGE_LOGIC
#warning "Executing DIRECT flush_icache_range"
	flush_icache_range( kvaNirvana, kvaNirvana+PAGE_SIZE); // the MVA based logic which doesn't flush till memory
	flush_icache_range( kvaNChild, kvaNChild+childLen);
#endif
#ifdef OLD_INDIRECT_FLUSH_ICACHE_RANGE_LOGIC
#warning "Executing INDIRECT flush_icache_range"
        kh_v7_coherent_kern_range( kvaNirvana, kvaNirvana+PAGE_SIZE);
        kh_v7_coherent_kern_range( kvaNChild, kvaNChild+childLen);
#endif

#ifdef OLD_FLUSH_KERNEL_MODULE_REQUIRED_CODE
	/* kernel load_module code takes care of flushing - have to check the strangeness related to this still not
	   reflecting in memory ????
	   UPDATE: Have identified the issue. flush_icache_range uses MVA based Cache maintainance operation which
	   syncs across all the processing masters but not necessarily to the level of memory, depending on the SOC
	   architecture. This is fine because kernel doesn't disable cache for its normal operation. But in our case
	   as we fully disable cache, the kernel module code it self appears stale, as the linking related resolution
	   done by module loader gets lost once cache is disabled, many a times. the flush_kern_all takes care of
	   handling this.

        kh_v7_coherent_kern_range( (unsigned long)hkvc_kexec_minimal, (unsigned long)hkvc_kexec_minimal_ext);
        kh_v7_coherent_kern_range( (unsigned long)hkvc_sleep, (unsigned long)hkvc_sleep+PAGE_SIZE);
        kh_v7_coherent_kern_range( (unsigned long)hkvc_uart_send, (unsigned long)hkvc_uart_send+PAGE_SIZE);
        kh_v7_coherent_kern_range( (unsigned long)hkvc_uart_send_hex, (unsigned long)hkvc_uart_send_hex+PAGE_SIZE);
        kh_v7_coherent_kern_range( (unsigned long)hkvc_uart_wait_on_tx_busy, (unsigned long)hkvc_uart_wait_on_tx_busy+PAGE_SIZE);
	*/
#endif

#ifdef ENABLE_TESTBEFORE
	if(mpTestBefore) {
		printk(KERN_WARNING "Could CRASH\n");
		test_end_code = (void*)kvaNirvana;
		test_end_code();
	}
#endif
        printk(KERN_INFO "Bye!\n");
	hkvc_kexec_minimal(kpaNirvana, kpaNChild, childLen);
}
EXPORT_SYMBOL(hkvc_kexec_minimal_ext);

void hkvc_kexec_alloc(void)
{
	long kpaNirvana, kpaNChild;

	kmNirvana = kmalloc(NIRVANA_BUFSIZE,GFP_KERNEL);
	if(!kmNirvana) {
		printk(KERN_ERR "lbhkvc: NirvanaCode - Failed to allocate kmNirvana\n");
		return;
	} else {
		kpaNirvana = virt_to_phys(kmNirvana);
		printk(KERN_INFO "lbhkvc: NirvanaCode - kmalloced location at V:0x%p P:0x%lx\n", kmNirvana, kpaNirvana);
	}
	//hkvc_meminfo_vaddr((unsigned long)kmNirvana,"kmNirvana");
        memcpy(kmNirvana, hkvc_nirvana1_bin, hkvc_nirvana1_bin_len);

	kmNChild = kmalloc(NCHILD_BUFSIZE,GFP_KERNEL);
	if(!kmNChild) {
		printk(KERN_ERR "lbhkvc: NirvanaChild - Failed to allocate kmNChild\n");
		return;
	} else {
		kpaNChild = virt_to_phys(kmNChild);
		printk(KERN_INFO "lbhkvc: NirvanaChild - kmalloced location at V:0x%p P:0x%lx\n", kmNChild, kpaNChild);
	}
	//hkvc_meminfo_vaddr((unsigned long)kmNChild,"kmNChild");
        memcpy(kmNChild, hkvc_nchild_bin, hkvc_nchild_bin_len);
	//hkvc_mem_touch(hkvc_nchild_bin,hkvc_nchild_bin_len);


	hkvc_kexec_minimal_ext(kpaNirvana, (unsigned long)kmNirvana, kpaNChild, (unsigned long)kmNChild, hkvc_nchild_bin_len+8);
}
EXPORT_SYMBOL(hkvc_kexec_alloc);

void hkvc_kexec_fixed(unsigned long kvaNirvana)
{
	unsigned long kpaNirvana, kpaNChild;

	kmNirvana = (void*)kvaNirvana;
	kpaNirvana = virt_to_phys(kmNirvana);
	printk(KERN_INFO "lbhkvc: NirvanaCode - fixed (kernel) addr V:0x%p P:0x%lx\n", kmNirvana, kpaNirvana);
	//hkvc_meminfo_vaddr((unsigned long)kmNirvana,"kmNirvana");
        memcpy(kmNirvana, hkvc_nirvana1_bin, hkvc_nirvana1_bin_len);
	// FIXME: Have to take care of providing a proper address for kmNChild also and then call minimal_ext
	//printk(KERN_INFO "lbhkvc: Nirvana's Child VAddr: 0x%p PAddr: 0x%lx", hkvc_nchild_bin, kpaNChild);
	//hkvc_kexec_minimal_ext(kpaNirvana, (unsigned long)kmNirvana);
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
#ifndef DEVICE_BEAGLEXM
	printk(KERN_INFO "lbhkvc: hard_smp_processor_id = %d\n",hard_smp_processor_id());
#endif
#ifdef ENABLE_MMU_DISABLE_BEFORELEAVING
	printk(KERN_INFO "lbhkvc: DANGER: MMU will be disabled before switching to new env(Nirvana)\n");
	printk(KERN_INFO "lbhkvc: NOTE: This won't work unless the system has atleast 1GB ram\n");
	printk(KERN_INFO "lbhkvc: NOTE: also assumes kernel modules load around 0xbf00.0000\n");
	printk(KERN_INFO "lbhkvc: NOTE: which is within 1GB memory so a same-to-same map can exist\n");
#else
	printk(KERN_INFO "lbhkvc: SAFE: MMU will NOT be disabled before switching to new env(Nirvana)\n");
	printk(KERN_INFO "lbhkvc: SAFE: Nirvana Code is required to disable mmu \n");
#endif
	printk(KERN_INFO "lbhkvc: hkvc_kexec_minimal addr = 0x%p\n",hkvc_kexec_minimal);
	printk(KERN_INFO "lbhkvc: hkvc_nirvana1_bin addr = 0x%p\n",hkvc_nirvana1_bin);

#ifdef ENABLE_PATCHUART
	patchUartAddr = (long*)&hkvc_nirvana1_bin[hkvc_nirvana1_bin_len-4];
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
		hkvc_kexec_fixed(mpFixedNirvanaVAddr);
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

