#include <linux/module.h>
#include <net/netlink.h>
#include <net/genetlink.h>
#include <linux/ctype.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include "common.h"

/* declare function */
 static int test_echo(struct sk_buff *skb, struct genl_info *info);

/* timer for multicast */
static struct timer_list timer;
static const int repeat_ms = 2000;
static struct genl_family test_genl_family;
 
#ifdef DEBUG
static void dump_nlmsg(struct nlmsghdr *nlh)
{
	int i, j, len, datalen;
	unsigned char *data;
	int col = 16;
	struct genlmsghdr *gnlh = nlmsg_data(nlh);
	struct nlattr *nla = genlmsg_data(gnlh);
	int remaining = genlmsg_len(gnlh);

	printk(KERN_DEBUG "===============DEBUG START===============\n");
	printk(KERN_DEBUG "nlmsghdr info (%d):\n", NLMSG_HDRLEN);
	printk(KERN_DEBUG
		"  nlmsg_len\t= %d\n" "  nlmsg_type\t= %d\n"
		"  nlmsg_flags\t= %d\n" "  nlmsg_seq\t= %d\n" "  nlmsg_pid\t= %d\n",
		nlh->nlmsg_len, nlh->nlmsg_type,
		nlh->nlmsg_flags, nlh->nlmsg_seq, nlh->nlmsg_pid);

	printk(KERN_DEBUG "genlmsghdr info (%ld):\n", GENL_HDRLEN);
	printk(KERN_DEBUG "  cmd\t\t= %d\n" "  version\t= %d\n" "  reserved\t= %d\n",
		gnlh->cmd, gnlh->version, gnlh->reserved);

	while (nla_ok(nla, remaining)) {
		printk(KERN_DEBUG "nlattr info (%d):\n", nla->nla_len);
		printk(KERN_DEBUG "  nla_len\t= %d\n" "  nla_type\t= %d\n", nla_len(nla), nla_type(nla));
		printk(KERN_DEBUG "  nla_data:\n");

		datalen = nla_len(nla);
		data = nla_data(nla);

		for (i = 0; i < datalen; i += col) {
			len = (datalen - i < col) ? (datalen - i) : col;

			printk("  ");
			for (j = 0; j < col; j++) {
				if (j < len)
					printk("%02x ", data[i + j]);
				else
					printk("   ");

			}
			printk("\t");
			for (j = 0; j < len; j++) {
				if (j < len)
					if (isprint(data[i + j]))
						printk("%c", data[i + j]);
					else
						printk(".");
				else
					printk(" ");
			}
			printk("\n");
		}

		len = nla_len(nla);
		if (nla_len(nla) < NLMSG_ALIGN(len)) {
			printk(KERN_DEBUG "nlattr pad (%d)\n", NLMSG_ALIGN(len) - len);
		}

		nla = nla_next(nla, &remaining);
	}
	printk(KERN_DEBUG "===============DEBUG END===============\n");
}
#else
static void dump_nlmsg(struct nlmsghdr *nlh)
{
	/* do nothing */
}
 #endif // DEBUG

 /* attribute policy */
 static struct nla_policy test_genl_policy[TEST_ATTR_MAX + 1] = {
       [TEST_ATTR_MSG] = { .type = NLA_NUL_STRING },
       [TEST_ATTR_DATA] = { .type = NLA_U32 },
 };

 /* operation definition */
 static struct genl_ops test_genl_ops[] = {
    {
       .cmd = TEST_CMD_ECHO,
       .flags = 0,
       .policy = test_genl_policy,
       .doit = test_echo,
       .dumpit = NULL,
    },
 };

/* multicast group. Choose a likely unique name. */
struct genl_multicast_group test_mcgrps[] = {
    { .name = "PotatoGroup" },
};
 /* family definition */
 static struct genl_family test_genl_family = {
       .hdrsize = 0,
       .name = TEST_GENL_NAME,
       .version = TEST_GENL_VERSION,
       .maxattr = TEST_ATTR_MAX,
       .ops = test_genl_ops,
       .n_ops = ARRAY_SIZE(test_genl_ops),
	   .mcgrps = test_mcgrps,
	   .n_mcgrps = ARRAY_SIZE(test_mcgrps),
 };

static int test_send_genl(struct genl_info *info, u8 cmd, u8 *str, u32 data)
{
    struct sk_buff *skb;
    int ret;
    void *msg_head;

    skb = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
    if (skb == NULL)
        return -ENOMEM;

    /* create the message headers */
    msg_head = genlmsg_put(skb, info->snd_portid, info->snd_seq, &test_genl_family, 0, cmd);
    if (msg_head == NULL) {
        ret = -ENOMEM;
        goto failure;
    }
    /* add a TEST_ATTR_MSG attribute */
    ret = nla_put_string(skb, TEST_ATTR_MSG, str);
    if (ret != 0)
        goto failure;

    /* add a TEST_ATTR_DATA attribute */
    ret = nla_put_u32(skb, TEST_ATTR_DATA, data);
    if (ret != 0)
        goto failure;

    /* finalize the message */
    genlmsg_end(skb, msg_head);

	/* debug info */
	dump_nlmsg(nlmsg_hdr(skb));

    ret = genlmsg_reply(skb, info);
    if (ret != 0)
        goto failure;
    
    return 0;
failure:
    printk(KERN_ERR "%s failure:%d\n", __FUNCTION__, ret);
    return ret;
}

 /* handler */
 static int test_echo(struct sk_buff *skb, struct genl_info *info)
 {
    /* message handling code goes here; return 0 on success, negative
       values on failure */
    int ret;
	// struct sk_buff *msg;
	char *message;
	u32 data; /* 32bit integer */
    char *str = "I am message from kernel!";

	if (!info->attrs[TEST_ATTR_MSG] || !info->attrs[TEST_ATTR_DATA] ) {
		printk(KERN_ERR "require message and data\n");
		return -EINVAL;
	}

	if (info->attrs[TEST_ATTR_MSG]) {
		message = (char *) nla_data(info->attrs[TEST_ATTR_MSG]);
	}

	if (info->attrs[TEST_ATTR_DATA]) {
		data = nla_get_u32(info->attrs[TEST_ATTR_DATA]);
	}

	/* debug info */
	dump_nlmsg(nlmsg_hdr(skb));

	printk(KERN_INFO "receive from user: message=%s, data=%u.\n", message, data);

    /* send message(str) and data(int) to userspace */
    data++;
    ret = test_send_genl(info, TEST_CMD_ECHO, str, data);
    printk(KERN_INFO "send to user: message=%s, data=%d\n", str, data);

    return ret;
 }

void send_multicast(struct timer_list *t)
{
    struct sk_buff *skb;
    void *msg_head;
    unsigned char *msg = "MULTICAST TEST";
    int error;

    pr_info("----- Running timer -----\n");

    pr_info("Newing message.\n");
    skb = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
    if (!skb) {
            pr_err("genlmsg_new() failed.\n");
            goto end;
    }

    pr_info("Adding Generic Netlink header to the message.\n");
    msg_head = genlmsg_put(skb, 0, 0, &test_genl_family, 0, TEST_CMD_NOTIFY);
    if (!msg_head) {
            pr_err("genlmsg_put() failed.\n");
            kfree_skb(skb);
            goto end;
    }

    pr_info("Nla_putting MSG(string) attribute.\n");
    error = nla_put_string(skb, TEST_ATTR_MSG, msg);
    if (error) {
            pr_err("nla_put_string() failed: %d\n", error);
            kfree_skb(skb);
            goto end;
    }

    pr_info("Nla_putting DATA(u32) attribute.\n");
    error = nla_put_u32(skb, TEST_ATTR_DATA, 12345);
    if (error) {
            pr_err("nla_put_u32() failed: %d\n", error);
            kfree_skb(skb);
            goto end;
    }

    pr_info("Ending message.\n");
    genlmsg_end(skb, msg_head);

    pr_info("Multicasting message.\n");
    /*
     * The family has only one group, so the group ID is just the family's
     * group offset.
     * mcgrp_offset is supposed to be private, so use this value for debug
     * purposes only.
     */
    pr_info("The group ID is %u.\n", test_genl_family.mcgrp_offset);
    error = genlmsg_multicast_allns(&test_genl_family, skb, 0, 0, GFP_KERNEL);
    if (error) {
            pr_err("genlmsg_multicast_allns() failed: %d\n", error);
            pr_err("(This can happen if nobody is listening. "
				   "Because it's not that unexpected, "
                   "you might want to just ignore this error.)\n");
            goto end;
    }

    pr_info("Success.\n");
end:
    // Reschedule the timer to call this function again in repeat_ms
    // milliseconds
    mod_timer(t, jiffies + msecs_to_jiffies(repeat_ms));
}

static void init_timer(void)
{
    pr_info("Starting timer.\n");

    // Initialize the timer and assign the callback function
    timer_setup(&timer, send_multicast, 0);

    // Set the timer to expire in repeat_ms milliseconds from now
    mod_timer(&timer, jiffies + msecs_to_jiffies(repeat_ms));
}

static int __init test_genl_init(void)
{
	int ret;

    ret = genl_register_family(&test_genl_family);
    if (ret)
        printk(KERN_ERR "genl_register_family err:%d\n", ret);

	init_timer();
    printk(KERN_INFO "genetlink module init successful\n");

	return 0;
}

static void __exit test_genl_exit(void)
{
    int ret;
    ret = genl_unregister_family(&test_genl_family);
    if (ret)
        printk(KERN_ERR "genl_unregister_family err:%d\n", ret);

    printk(KERN_INFO "genetlink module exit successful\n");
}

module_init(test_genl_init);
module_exit(test_genl_exit);

MODULE_AUTHOR("Seaworth");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("genetlink test module");