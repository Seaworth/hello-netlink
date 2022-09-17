#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <errno.h>

#define NETLINK_ID 30 // keep same with kernel module
#define MAX_PAYLOAD 1024

struct msghdr recv_msg;
struct msghdr send_msg; /* Question: when send_msg is in main func, running is err */

int main()
{
    int sock_fd;
    char user_msg[MAX_PAYLOAD];
    int ret;
    struct sockaddr_nl src_addr;
    struct sockaddr_nl des_addr;
    struct nlmsghdr *recv_msghdr = NULL;
    struct nlmsghdr *send_msghdr = NULL;
    struct iovec recv_iov;
    struct iovec send_iov;

    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_ID);
    if (sock_fd < 0) {
        return -1;
    }

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();

    ret = bind(sock_fd, (struct sockaddr *) &src_addr, sizeof(src_addr));
	if (ret < 0) {
		printf("bind: %s\n", strerror(errno));
		close(sock_fd);
		return ret;
	}

    memset(&des_addr, 0, sizeof(des_addr));
    des_addr.nl_family = AF_NETLINK;
	des_addr.nl_pid = 0; /* For Linux Kernel */
    des_addr.nl_groups = 0; /* unicast */

    /* send_msghdr contains "hello" msg */
    send_msghdr = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(send_msghdr, 0, NLMSG_SPACE(MAX_PAYLOAD));
    send_msghdr->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    send_msghdr->nlmsg_pid = getpid(); /* self pid */
    send_msghdr->nlmsg_flags = 0;

    /* recv_msghdr contains received msg */
    recv_msghdr = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(recv_msghdr, 0, NLMSG_SPACE(MAX_PAYLOAD));
    recv_msghdr->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    recv_msghdr->nlmsg_pid = getpid();
    recv_msghdr->nlmsg_flags = 0;

    /* set send_iov and send_msg */
    strcpy(NLMSG_DATA(send_msghdr), "hello kernel, I am from userspace");
    
    send_iov.iov_base = (void *)send_msghdr;
    send_iov.iov_len = send_msghdr->nlmsg_len;
    send_msg.msg_name = (void *)&des_addr;
    send_msg.msg_namelen = sizeof(des_addr);
    send_msg.msg_iov = &send_iov;
    send_msg.msg_iovlen = 1;

    /* set recv_iov and recv_msg */
    recv_iov.iov_base = (void *)recv_msghdr;
    recv_iov.iov_len = recv_msghdr->nlmsg_len;
    recv_msg.msg_name = (void *)&des_addr;
    recv_msg.msg_namelen = sizeof(des_addr);
    recv_msg.msg_iov = &recv_iov;
    recv_msg.msg_iovlen = 1;

    printf("Sending message to kernel\n");
	ret = sendmsg(sock_fd, &send_msg, 0);
	if (ret < 0) {
		printf("Sendmsg err: %d %s\n", ret, strerror(errno));
	} else {
        printf("Sendmsg ret: %d\n", ret);
    }

    printf("Waiting for message from kernel\n");
    recvmsg(sock_fd, &recv_msg, 0);
    printf("Received message payload: %s\n", (char *)NLMSG_DATA(recv_msghdr));

    while (1) {
        printf("Input your msg for sending to kernel: ");
        scanf("%s", user_msg);
        strcpy(NLMSG_DATA(send_msghdr), user_msg);
        printf("Sending msg \"%s\" to kernel\n", user_msg);
        
        ret = sendmsg(sock_fd, &send_msg, 0);
        printf("send ret: %d\n", ret);

        recvmsg(sock_fd, &recv_msg, 0);
        printf("Received message payload: %s\n", (char *)NLMSG_DATA(recv_msghdr));
    }

    close(sock_fd);

    return 0;
}
