/*
 * linuxboothkvc_kernel.c
 * v04Jan2012_2105
 * HKVC, GPL
 * Disable interrupts
 * Move to core CPU in the SMP setup 
 * - Disable the non_core/other CPU(s) in a SMP setup (Have to think thro further)
 * Try and find 1:1 map 
 * - even 1 page should do initially
 * Copy a minimal code to this 1:1 map
 * Switch off MMU
 * retain control in itself (1st phase fantabulus success)
 * 
 * Else see if a reserved physical memory can be mapped in 1 to 1.
 *
 */

#include <linux/err.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <asm/highmem.h>
#include <asm/fixmap.h>
#include <linux/spinlock.h>

#define RSRVD_PHYS_ADDR1	0x9C000000 /* picked from fwram */
#define RSRVD_PHYS_ADDR2	0x9CF00000 /* Ducati baseimage physical address */
#define LBHKVC_MINOR 100
#define LBHKVC_VERSION "v05Jan2012_1257"

static DEFINE_SPINLOCK(my_lock);

#define S2S_BUFSIZE (32*1024)
#define O2O_BUFSIZE (32*1024)

char s2s_buf[S2S_BUFSIZE];
char o2o_buf[O2O_BUFSIZE];

static s32 lbhkvc_mmap(struct file *f, struct vm_area_struct *v);

static const struct file_operations lbhkvc_fops = {
	.mmap    = lbhkvc_mmap,
};

static struct miscdevice lbhkvc_dev = {
	LBHKVC_MINOR,
	"lbhkvc",
	&lbhkvc_fops
};

static void dump_kernel_mm(void)
{
	long iTotalMem = 1024*1024*1024;

	printk(KERN_INFO "INFO: iTotalMem 0x%lx (Hardwired) \n",
				iTotalMem);
	printk(KERN_INFO "INFO: PAGE_SHIFT %d ie %d Page Size on this target \n",
				PAGE_SHIFT,0x1<<PAGE_SHIFT);
	printk(KERN_INFO "INFO: PHYS_OFFSET P:0x%lx(D:0x%lx) Begining of Platform ram \n\n",
				PHYS_OFFSET, virt_to_phys((void*)PHYS_OFFSET));

	printk(KERN_INFO "INFO: TASK_SIZE V:0x%lx(D:0x%lx) End of userspace mapping \n",
				TASK_SIZE, virt_to_phys((void*)TASK_SIZE));
	printk(KERN_INFO "INFO: MODULES_VADDR V:0x%lx(D:0x%lx) Start address for Modules Space \n",
				MODULES_VADDR, virt_to_phys((void*)MODULES_VADDR));
	printk(KERN_INFO "INFO: MODULES_END V:0x%lx(D:0x%lx) End address for Modules Space \n",
				MODULES_END, virt_to_phys((void*)MODULES_END));
	printk(KERN_INFO "INFO: PKMAP_BASE V:0x%lx(?:0x%lx) Permanent kernal mappings \n",
				PKMAP_BASE, virt_to_phys((void*)PKMAP_BASE));
	printk(KERN_INFO "INFO: PAGE_OFFSET V:0x%lx(P:0x%lx) Kernel direct 1:1 map platform RamBeg \n",
				PAGE_OFFSET, virt_to_phys((void*)PAGE_OFFSET));
	printk(KERN_INFO "NOTE: PAGE_OFFSET P:addr above should match PHY_OFFSET P:addr\n");
	printk(KERN_INFO "INFO: high_memory V:0x%p(P:0x%lx) Kernel direct 1:1 map platform RamEnd \n",
				high_memory, virt_to_phys((void*)high_memory));
	printk(KERN_INFO "INFO: VMALLOC_START V:0x%lx(D:0x%lx) vmalloc/ioremap space Begin \n",
				VMALLOC_START, virt_to_phys((void*)VMALLOC_START));
	printk(KERN_INFO "INFO: VMALLOC_END V:0x%lx(D:0x%lx) vmalloc/ioremap space End \n",
				VMALLOC_END, virt_to_phys((void*)VMALLOC_END));

	printk(KERN_INFO "INFO: FIXADDR_START V:0x%lx(D:0x%lx) fixmap space Begin \n",
				FIXADDR_START, virt_to_phys((void*)FIXADDR_START));
	printk(KERN_INFO "INFO: FIXADDR_TOP V:0x%lx(D:0x%lx) fixmap space End \n",
				FIXADDR_TOP, virt_to_phys((void*)FIXADDR_TOP));
}

static void dump_mymem(void)
{
	printk(KERN_INFO "My s2s_buf V:0x%p\n",s2s_buf);
	printk(KERN_INFO "My o2o_buf V:0x%p\n",o2o_buf);

}

static s32 __init lbhkvc_init(void)
{
	int ret;
	unsigned long flags;

	ret = misc_register(&lbhkvc_dev);
	if (ret) {
		printk(KERN_ERR "lbhkvc: misc_register failed on minor=%d\n",
			LBHKVC_MINOR);
		return ret;
	}
	printk(KERN_INFO "LBHKVC driver " LBHKVC_VERSION "\n");

	spin_lock_irqsave(&my_lock,flags);
	dump_kernel_mm();
	dump_mymem();
	spin_unlock_irqrestore(&my_lock,flags);
	return 0;
}

static s32 lbhkvc_mmap(struct file *f, struct vm_area_struct *vma)
{
	unsigned long pfn = RSRVD_PHYS_ADDR1 >> PAGE_SHIFT;
	unsigned long vsize = vma->vm_end - vma->vm_start;
	unsigned long prot;

	if (vma->vm_pgoff != 0)
		return -EINVAL;

	vma->vm_flags &= ~VM_MAYREAD;
	vma->vm_flags &= ~VM_READ;

	/* the protection requested for the new vma */
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	prot = pgprot_val(vma->vm_page_prot);
	vma->vm_page_prot = __pgprot(prot);

	if (remap_pfn_range(vma, vma->vm_start,
			pfn,
			vsize, vma->vm_page_prot)) {
		printk(KERN_ERR "remap_pfn_range failed in lbhkvc_mmap\n");
		return -EAGAIN;
	}

	return 0;
}

static void __exit lbhkvc_exit(void)
{
	misc_deregister(&lbhkvc_dev);
}

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("hkvc@hkvc.hkvc");
module_init(lbhkvc_init);
module_exit(lbhkvc_exit);

