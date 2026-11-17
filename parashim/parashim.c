// SPDX-License-Identifier: GPL-2.0
/*
 * parashim —— 为 5.15 编译的 paradise.ko 在 5.4 内核上补齐缺失符号
 *
 * 目标内核：5.4.254-qgki-gd2a1055adeb0（ColorOS 14 线，CONFIG_MODULE_FORCE_LOAD=y）
 *
 * paradise_5.15.ko 导入 102 个内核符号，其中 13 个在 5.4 上根本不存在：
 *   _printk / __alloc_pages / close_fd / __find_vma / __cfi_slowpath_diag
 *   __mmap_lock_do_trace_{start_locking,acquire_returned,released}
 *   __tracepoint_mmap_lock_{start_locking,acquire_returned,released}
 *   kasan_flag_enabled / gic_nonsecure_priorities
 *
 * 本模块把它们补出来，让 paradise 通过 simplify_symbols()。
 *
 * 依赖的内核符号刻意压到最少（5.4 全部导出）：
 *   printk / __alloc_pages_nodemask / __close_fd / find_vma / current
 * 其余一律空实现或零内存，避免再次踩到"符号不存在"。
 */
#define pr_fmt(fmt) "parashim: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/sched/mm.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/file.h>
#include <linux/fdtable.h>
#include <linux/fs.h>
#include <linux/gfp.h>
#include <linux/vmalloc.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("5.15 -> 5.4 symbol shim for paradise.ko");
MODULE_VERSION("1.0");

/* ------------------------------------------------------------------ */
/* 1. _printk                                                          */
/*    5.15 把导出符号从 printk 改名成 _printk。5.4 导出的是 printk。    */
/*    这里用 vsnprintf + printk，不碰 vprintk_emit（5.4/5.10 签名不同）*/
/* ------------------------------------------------------------------ */
asmlinkage int _printk(const char *fmt, ...)
{
	char buf[512];
	va_list args;
	int n;

	va_start(args, fmt);
	n = vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	if (n > 0)
		printk("%s", buf);
	return n;
}
EXPORT_SYMBOL(_printk);

/* ------------------------------------------------------------------ */
/* 2. __alloc_pages                                                    */
/*    5.4 叫 __alloc_pages_nodemask；5.15 改名 __alloc_pages。签名一致 */
/* ------------------------------------------------------------------ */
struct page *__alloc_pages(gfp_t gfp, unsigned int order,
			   int preferred_nid, nodemask_t *nodemask)
{
	return __alloc_pages_nodemask(gfp, order, preferred_nid, nodemask);
}
EXPORT_SYMBOL(__alloc_pages);

/* ------------------------------------------------------------------ */
/* 3. close_fd                                                         */
/* ------------------------------------------------------------------ */
extern int __close_fd(struct files_struct *files, unsigned int fd);

int close_fd(unsigned int fd)
{
	if (!current->files)
		return -EBADF;
	return __close_fd(current->files, fd);
}
EXPORT_SYMBOL(close_fd);

/* ------------------------------------------------------------------ */
/* 4. __find_vma                                                       */
/* ------------------------------------------------------------------ */
struct vm_area_struct *__find_vma(struct mm_struct *mm, unsigned long addr)
{
	return find_vma(mm, addr);
}
EXPORT_SYMBOL(__find_vma);

/* ------------------------------------------------------------------ */
/* 5. __cfi_slowpath_diag                                              */
/*    5.4 只有 __cfi_slowpath，且不一定导出。做成空实现：              */
/*    它只在 CFI 间接调用校验失败时被调用，空转不会影响正常路径。       */
/* ------------------------------------------------------------------ */
void __cfi_slowpath_diag(u64 type_id, void *ptr, void *diag)
{
}
EXPORT_SYMBOL(__cfi_slowpath_diag);

/* ------------------------------------------------------------------ */
/* 6~8. mmap_lock trace 包装                                           */
/*    mmap_sem → mmap_lock 是 5.8 的事，5.4 没有这套设施。             */
/*    只在 tracepoint 打开时才会被调用，而下面 9~11 的 static_key=0，   */
/*    所以永远不会被执行。                                              */
/* ------------------------------------------------------------------ */
void __mmap_lock_do_trace_start_locking(struct mm_struct *mm, bool write) { }
EXPORT_SYMBOL(__mmap_lock_do_trace_start_locking);

void __mmap_lock_do_trace_acquire_returned(struct mm_struct *mm, bool write,
					   bool success) { }
EXPORT_SYMBOL(__mmap_lock_do_trace_acquire_returned);

void __mmap_lock_do_trace_released(struct mm_struct *mm, bool write) { }
EXPORT_SYMBOL(__mmap_lock_do_trace_released);

/* ------------------------------------------------------------------ */
/* 9~11. 三个 tracepoint 对象                                          */
/*    struct tracepoint 在 5.4 与 5.15 里 name 都在 +0、key 都在 +8，  */
/*    给零内存即可让 trace_*_enabled() 读到 0 = 关闭。                 */
/*    64 字节 > 两边尺寸（5.4=48 / 5.15=40）。                          */
/* ------------------------------------------------------------------ */
char __tracepoint_mmap_lock_start_locking[64];
EXPORT_SYMBOL(__tracepoint_mmap_lock_start_locking);

char __tracepoint_mmap_lock_acquire_returned[64];
EXPORT_SYMBOL(__tracepoint_mmap_lock_acquire_returned);

char __tracepoint_mmap_lock_released[64];
EXPORT_SYMBOL(__tracepoint_mmap_lock_released);

/* ------------------------------------------------------------------ */
/* 12. kasan_flag_enabled                                              */
/*    目标内核未编 KASAN。5.15 里它是 static_key，零值即 false。        */
/* ------------------------------------------------------------------ */
char kasan_flag_enabled[64];
EXPORT_SYMBOL(kasan_flag_enabled);

/* ------------------------------------------------------------------ */
/* 13. gic_nonsecure_priorities                                        */
/*    arm64 GICv3 内部数据，目标内核未导出。零内存 = 无特殊优先级。     */
/* ------------------------------------------------------------------ */
char gic_nonsecure_priorities[64];
EXPORT_SYMBOL(gic_nonsecure_priorities);

static int __init parashim_init(void)
{
	pr_info("loaded: 13 symbols shimmed for paradise.ko\n");
	return 0;
}
module_init(parashim_init);

static void __exit parashim_exit(void)
{
	pr_info("unloaded\n");
}
module_exit(parashim_exit);
