#ifndef __COMMON_H__
#define __COMMON_H__

#define TEST_GENL_NAME "hello_genl"
#define TEST_GENL_MC_NAME "hello_mc"
#define TEST_GENL_VERSION 1
#define TEST_GENL_GROUP_NAME "genl_grp"

/* commands */
enum {
	TEST_CMD_UNSPEC,
	TEST_CMD_ECHO,
	TEST_CMD_NOTIFY,
	__TEST_CMD_MAX,
};
#define TEST_CMD_MAX (__TEST_CMD_MAX - 1)

/* attributes */
enum {
	TEST_ATTR_UNSPEC,
	TEST_ATTR_MSG,
	TEST_ATTR_DATA,
	__TEST_ATTR_MAX,
};
#define TEST_ATTR_MAX (__TEST_ATTR_MAX - 1)

#endif /* !__COMMON_H__ */
