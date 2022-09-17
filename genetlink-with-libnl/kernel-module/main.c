#include <linux/module.h>
#include <net/netlink.h>
#include <net/genetlink.h>
#include <linux/ctype.h>

// #include <include/net/netlink.h>
// #include <include/net/genetlink.h>

#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/types.h>
#include <linux/version.h>
#include "common.h"

 static struct genl_family test_genl_family;



 /* attribute policy */
 static struct nla_policy test_genl_policy[TEST_ATTR_MAX + 1] = {
       [TEST_ATTR_MSG] = { .type = NLA_NUL_STRING },
       [TEST_ATTR_DATA] = { .type = NLA_U32 },
 };

static int test_send_genl(struct genl_info *info, u8 attr, u8 cmd, u8 *data)
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
    ret = nla_put_string(skb, TEST_ATTR_MSG, data);
    if (ret != 0)
        goto failure;
    /* finalize the message */
    genlmsg_end(skb, msg_head);

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

	if (!info->attrs[TEST_ATTR_MSG] /* && !info->attrs[TEST_ATTR_DATA] */) {
		printk(KERN_ERR "require message and data\n");
		return -EINVAL;
	}

	if (info->attrs[TEST_ATTR_MSG]) {
		message = (char *) nla_data(info->attrs[TEST_ATTR_MSG]);
	}

	// if (info->attrs[TEST_ATTR_DATA]) {
	// 	data = nla_get_u32(info->attrs[TEST_ATTR_DATA]);
	// }

	/* debug info */
	//dump_nlmsg(nlmsg_hdr(skb));

	printk(KERN_INFO "receive from user: message=%s.\n", message);
	// printk(KERN_INFO "receive from user: message=%s, data=%u.\n", message, data);

    /* send message to userspace */
    ret = test_send_genl(info, TEST_ATTR_MSG, TEST_CMD_ECHO, str);
    printk(KERN_INFO "send to user: message=%s\n", str);

    return ret;
// failure:
//     printk(KERN_ERR "%s failure:%d\n", __FUNCTION__, ret);
//     return ret;
 }

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

 /* family definition */
 static struct genl_family test_genl_family = {
    //    .id = GENL_ID_GENERATE,
       .hdrsize = 0,
       .name = TEST_GENL_NAME,
       .version = 1,
       .maxattr = TEST_ATTR_MAX,
       .ops = test_genl_ops,
       .n_ops = ARRAY_SIZE(test_genl_ops),
 };

static int __init test_genl_init(void)
{
	int ret;

    ret = genl_register_family(&test_genl_family);
    if (ret)
        printk(KERN_ERR "genl_register_family err:%d\n", ret);
    
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