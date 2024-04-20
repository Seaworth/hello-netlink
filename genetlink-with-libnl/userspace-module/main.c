#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>
#include <netlink/netlink.h>
#include <linux/types.h>
#include <linux/socket.h>
#include <netlink/netlink.h>
#include <stdio.h>
#include <unistd.h>
#include "logger.h"
#include "../kernel-module/common.h"

#define UNUSED __attribute__((unused))

static int recv_genl(struct nlmsghdr *nlh)
{
	char *message;
	uint32_t data;
	int err;
	struct nlattr *attrs[TEST_ATTR_MAX + 1];

	err = genlmsg_parse(nlh, 0, attrs, TEST_ATTR_MAX, NULL);
	if (err) {
		LOG_ERROR("genlmsg_parse: couldn't parse genlmsg");
		LOG_ERROR("%s (%d)\n", nl_geterror(err), err);
		return NL_SKIP;
	}

	if (!attrs[TEST_ATTR_MSG] || !attrs[TEST_ATTR_DATA]) {
		LOG_ERROR("message or data is NULL");
		return NL_SKIP;
	}

	if (attrs[TEST_ATTR_MSG]) {
		message = nla_get_string(attrs[TEST_ATTR_MSG]);
	}

	if (attrs[TEST_ATTR_DATA]) {
		data = nla_get_u32(attrs[TEST_ATTR_DATA]);
	}

	LOG_INFO("receive from kernel: message=%s, data=%u", message, data);

	return NL_OK;
}

static int recv_genl_msg(struct nl_msg *msg, UNUSED void *arg)
{
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct genlmsghdr *gnlh = genlmsg_hdr(nlh);

	switch (gnlh->cmd) {
		case TEST_CMD_ECHO:
			LOG_INFO("reply from kernel");
			return recv_genl(nlh);
		case TEST_CMD_NOTIFY:
			LOG_INFO("multicast from kernel");
			return recv_genl(nlh);
		default:
			LOG_INFO("skip");
			return NL_SKIP;
	}
}

static int send_echo_info(struct nl_sock *sock, int family_id,
		const char *str, uint32_t data)
{
	struct nl_msg *msg;
	void *head;

	msg = nlmsg_alloc();
	if (!msg) {
		LOG_ERROR("nlmsg_alloc: couldn't alloc nlmsg");
		return -1;
	}

	head = genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, family_id, 0, 0, TEST_CMD_ECHO, 0);
	if (!head) {
		LOG_ERROR("genlmsg_put: build nlmsg header error");
		nlmsg_free(msg);
		return -1;
	}

	NLA_PUT_STRING(msg, TEST_ATTR_MSG, str);
	NLA_PUT_U32(msg, TEST_ATTR_DATA, data);

	return nl_send_auto(sock, msg);

nla_put_failure:
	LOG_ERROR("put attr error");
	nlmsg_free(msg);
	return -1;
}

/* userspace send msg to kernel */
static int run_client()
{
	struct nl_sock *sock;
	int family_id;
	int ret;
	char *str = "Hello, I am userspace!";
	uint32_t data = 20220917;

	sock = nl_socket_alloc();
	if (!sock) {
		LOG_ERROR("nl_socket_alloc: couldn't alloc nl_sock");
		return EXIT_FAILURE;
	}

	ret = genl_connect(sock);
	if (ret < 0) {
		LOG_ERROR("genl_connect: couldn't connect the sock");
		nl_socket_free(sock);
		return EXIT_FAILURE;
	}

	family_id = genl_ctrl_resolve(sock, TEST_GENL_NAME);
	if (family_id < 0) {
		LOG_ERROR("genl_ctrl_resolve: couldn't resolve family id");
		nl_socket_free(sock);
		return EXIT_FAILURE;
	}

	nl_socket_modify_cb(sock, NL_CB_MSG_IN, NL_CB_CUSTOM, &recv_genl_msg, sock);
	
	LOG_INFO("start to send message");

	ret = send_echo_info(sock, family_id, str, data);
	if (ret < 0) {
		LOG_ERROR("send_echo_info: send info error");
		nl_socket_free(sock);
		return EXIT_FAILURE;
	}

	LOG_INFO("msg=%s, data=%d", str, data);
	LOG_INFO("start to recv message");

	nl_recvmsgs_default(sock);
	nl_socket_free(sock);
	return EXIT_SUCCESS;
}

/* userspace recv msg from kernel by multicast */
static int run_server()
{
	struct nl_sock *sk = NULL;
    int group;
    int error;
	int cnt = 10;

    sk = nl_socket_alloc();
	if (!sk) {
		LOG_ERROR("nl_socket_alloc: couldn't alloc nl_sock");
		return EXIT_FAILURE;
	}

    nl_socket_disable_seq_check(sk);

    error = nl_socket_modify_cb(sk, NL_CB_VALID, NL_CB_CUSTOM, recv_genl_msg, NULL);
	if (error) {
		LOG_ERROR("nl_socket_modify_cb err:%s (%d)\n", nl_geterror(error), error);
		nl_socket_free(sk);
		return EXIT_FAILURE;
	}

    error = genl_connect(sk);
	if (error) {
		LOG_ERROR("genl_connect err:%s (%d)\n", nl_geterror(error), error);
		nl_socket_free(sk);
		return EXIT_FAILURE;
	}

    /* Find the multicast group identifier and register ourselves to it. */
    group = genl_ctrl_resolve_grp(sk, TEST_GENL_NAME, TEST_GENL_MC_NAME);
	if (group < 0) {
		LOG_ERROR("genl_ctrl_resolve_grp err:%s (%d)\n", nl_geterror(group), group);
		nl_socket_free(sk);
		return EXIT_FAILURE;
	}

    LOG_INFO("The group is %u.\n", group);
    error = nl_socket_add_memberships(sk, group, 0);
    if (error) {
		LOG_ERROR("nl_socket_add_memberships err:%s (%d)\n", nl_geterror(error), error);
		nl_socket_free(sk);
		return EXIT_FAILURE;
    }

    /* Finally, receive the message. */
	while (cnt--)
    	nl_recvmsgs_default(sk);

    if (sk)
		nl_socket_free(sk);

    return 0;
}

int main(int argc, char *argv[])
{

	int index;
	int c;

	// -c: client send msg to kernel
	// -s: server recv msg from kernel by multicast
	while ((c = getopt (argc, argv, "cs")) != -1) {
		switch (c)
		{
			case 'c':
				LOG_INFO("client mode:\n");
				run_client();
				break;
			case 's':
				LOG_INFO("server mode:\n");
				run_server();
				break;
			default:
				LOG_INFO("option is valid\n");
		}
	}

	for (index = optind; index < argc; index++)
		LOG_INFO("Non-option argument %s\n", argv[index]);
	return 0;
}