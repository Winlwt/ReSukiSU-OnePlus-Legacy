/*
 * SUSFS v2 兼容层
 *
 * 说明：
 *   ReSukiSU 4.2.x 的 supercall 会调用一批 SUSFS v2 才有的函数，
 *   而当前可用的 5.4 内核侧实现（susfs4ksu kernel-5.4，2025-02）
 *   只有 v1 接口。这里给出空实现让内核能链接，行为如下：
 *
 *   - sus_path / sus_mount / sus_kstat / try_umount / set_uname /
 *     cmdline / open_redirect  —— 由 fs/susfs.c 的 v1 实现提供，功能正常
 *   - sus_map / avc_log_spoofing / sdcard monitor / no_su 系列
 *     —— 本文件空实现，命令返回成功但不产生任何隐藏效果
 */
#include <linux/types.h>
#include <linux/jump_label.h>
#include <linux/compiler_types.h>

/* ---- SUSFS v2 独有：空实现 ---- */

__weak void susfs_add_sus_map(void __user **user_info) { }
__weak void susfs_add_sus_path_loop(void __user **user_info) { }
__weak void susfs_set_hide_sus_mnts_for_non_su_procs(void __user **user_info) { }
__weak void susfs_set_avc_log_spoofing(void __user **user_info) { }
__weak void susfs_enable_log(void __user **user_info) { }
__weak void susfs_show_variant(void __user **user_info) { }
__weak void susfs_show_version(void __user **user_info) { }
__weak void susfs_get_enabled_features(void __user **user_info) { }
__weak void susfs_start_sdcard_monitor_fn(void) { }
__weak void susfs_extra_works(void) { }

__weak bool susfs_is_current_proc_umounted(void)
{
	return false;
}

__weak void susfs_set_current_proc_umounted(void) { }
__weak void susfs_set_current_proc_umounted_for_zygote_next(void) { }
__weak void susfs_set_current_proc_no_su(void) { }

__weak bool susfs_is_current_proc_no_su(void)
{
	return false;
}

__weak void susfs_clear_current_proc_no_su(void) { }

/* ---- 其余两个链接期缺失符号 ---- */

__weak struct static_key_true ksu_init_rc_hook = STATIC_KEY_TRUE_INIT;

__weak void oplus_exec_block(void) { }
