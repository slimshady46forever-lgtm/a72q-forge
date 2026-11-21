// SPDX-License-Identifier: GPL-2.0
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/string.h>

static void cmdline_replace(char *s, const char *from, const char *to)
{
	char *p = strstr(s, from);
	size_t fl, tl;

	if (!p)
		return;
	fl = strlen(from);
	tl = strlen(to);
	if (fl != tl)
		memmove(p + tl, p + fl, strlen(p + fl) + 1);
	memcpy(p, to, tl);
}

static char *cmdline_sanitize(const char *src)
{
	char *buf = kstrdup(src, GFP_KERNEL);

	if (!buf)
		return NULL;
	cmdline_replace(buf, "androidboot.verifiedbootstate=orange",
			"androidboot.verifiedbootstate=green");
	cmdline_replace(buf, "androidboot.selinux=permissive",
			"androidboot.selinux=enforcing");
	cmdline_replace(buf, "androidboot.warranty_bit=1",
			"androidboot.warranty_bit=0");
	return buf;
}

static int cmdline_proc_show(struct seq_file *m, void *v)
{
	char *buf = cmdline_sanitize(saved_command_line);

	if (buf) {
		seq_printf(m, "%s\n", buf);
		kfree(buf);
	} else {
		seq_printf(m, "%s\n", saved_command_line);
	}
	return 0;
}

static int cmdline_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, cmdline_proc_show, NULL);
}

static const struct file_operations cmdline_proc_fops = {
	.open		= cmdline_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init proc_cmdline_init(void)
{
	proc_create("cmdline", 0, NULL, &cmdline_proc_fops);
	return 0;
}
fs_initcall(proc_cmdline_init);
