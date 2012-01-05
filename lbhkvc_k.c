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
#include <linux/spinlock.h>

#define RSRVD_PHYS_ADDR1    0x9C000000 /* picked from fwram */
#define LBHKVC_MINOR 100
#define LBHKVC_VERSION "v04Jan2012_2110"

static DEFINE_SPINLOCK(my_lock);

static s32 lbhkvc_mmap(struct file *f, struct vm_area_struct *v);

static const struct file_operations lbhkvc_fops = {
	.mmap    = lbhkvc_mmap,
};

static struct miscdevice lbhkvc_dev = {
	LBHKVC_MINOR,
	"lbhkvc",
	&lbhkvc_fops
};

static s32 __init lbhkvc_init(void)
{
	int ret;
	long iTotalMem = 1024*1024*1024;
	unsigned long flags;

	ret = misc_register(&lbhkvc_dev);
	if (ret) {
		printk(KERN_ERR "lbhkvc: misc_register failed on minor=%d\n",
			LBHKVC_MINOR);
		return ret;
	}
	spin_lock_irqsave(&my_lock,flags);
	printk(KERN_INFO "LBHKVC driver " LBHKVC_VERSION "\n");
	printk(KERN_INFO "INFO: Total memory is 			iTotalMem 0x%lx\n",iTotalMem);
	printk(KERN_INFO "INFO: Page Size ???				PAGE_SHIFT %d ie %d\n",PAGE_SHIFT,0x1<<PAGE_SHIFT);
	printk(KERN_INFO "INFO: Begining of Platform ram		PHYS_OFFSET 0x%lx(0x%lx)\n\n", PHYS_OFFSET, virt_to_phys((void*)PHYS_OFFSET));
	printk(KERN_INFO "INFO: End of userspace mapping		TASK_SIZE 0x%lx(0x%lx)\n", TASK_SIZE, virt_to_phys((void*)TASK_SIZE));
	printk(KERN_INFO "INFO: Start address for Modules Space 	MODULES_VADDR 0x%lx(0x%lx)\n", MODULES_VADDR, virt_to_phys((void*)MODULES_VADDR));
	printk(KERN_INFO "INFO: End address for Modules Space 		MODULES_END 0x%lx(0x%lx)\n", MODULES_END, virt_to_phys((void*)MODULES_END));
	printk(KERN_INFO "INFO: Permanent kernal mappings		PKMAP_BASE 0x%lx(0x%lx)\n", PKMAP_BASE, virt_to_phys((void*)PKMAP_BASE));
	printk(KERN_INFO "INFO: Kernel direct 1:1 map platform RamBeg	PAGE_OFFSET 0x%lx(0x%lx)\n", PAGE_OFFSET, virt_to_phys((void*)PAGE_OFFSET));
	printk(KERN_INFO "INFO: Kernel direct 1:1 map platform RamEnd	high_memory 0x%p(0x%lx)\n", high_memory, virt_to_phys((void*)high_memory));
	printk(KERN_INFO "INFO: vmalloc/ioremap space Begin		VMALLOC_START 0x%lx(0x%lx)\n", VMALLOC_START, virt_to_phys((void*)VMALLOC_START));
	printk(KERN_INFO "INFO: vmalloc/ioremap space End		VMALLOC_END 0x%lx(0x%lx)\n", VMALLOC_END, virt_to_phys((void*)VMALLOC_END));
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

