#include <linux/init.h>
#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>

#define NETLINK_ID 30

struct sock *nl_sock = NULL;

static void netlink_recv(struct sk_buff *skb)
{
	struct nlmsghdr *nlhead;
	struct sk_buff *skb_send;
	int pid;
	int msg_size;
	int res;
	char *msg = "hello, I am from kernel";


	nlhead = (struct nlmsghdr*)skb->data;
	printk(KERN_INFO "kernel recv msg from userspace: %s\n", (char*)nlmsg_data(nlhead));

	/* Sending process port ID */
	pid = nlhead->nlmsg_pid;

	msg_size = strlen(msg);
	skb_send = nlmsg_new(strlen(msg), 0);
	if (!skb_send)
	{
		printk(KERN_ERR "alloc new skb err\n");
		return;
	}
	
	nlhead = nlmsg_put(skb_send, 0, 0, NLMSG_DONE, msg_size, 0);
	NETLINK_CB(skb_send).dst_group = 0;
	strncpy(nlmsg_data(nlhead), msg, msg_size);
	res = nlmsg_unicast(nl_sock, skb_send, pid);
	if(res < 0)
		printk(KERN_INFO "Error while sending back to userspace\n");

}

static int __init hello_netlink_init(void)
{
	struct netlink_kernel_cfg cfg = {
		.input = netlink_recv,
	};
	nl_sock = netlink_kernel_create(&init_net, NETLINK_ID, &cfg);
	if (!nl_sock)
	{
		printk(KERN_ERR "Create netlink socket err!\n");
		return -10;
	}
	printk(KERN_INFO "Entering: %s, protocol family = %d \n", __FUNCTION__, NETLINK_ID);

	printk(KERN_INFO "%s ok\n", __FUNCTION__);
	return 0;
}
module_init(hello_netlink_init);

static void  __exit hello_netlink_exit(void)
{
	netlink_kernel_release(nl_sock);
	printk(KERN_INFO "%s ok\n", __FUNCTION__);
}
module_exit(hello_netlink_exit);

MODULE_AUTHOR("Seaworth");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("hello netlink kernel module");