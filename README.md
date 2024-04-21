### 简介

#### genetlink-with-libnl

`genetlink-with-libnl`文件夹包含kernel-module和userspace-module。实现了userspace-module向kernel-module发送消息，同时userspace-module可以监听kernel-module的multicast消息。

##### testgenl用户程序

- 参数-c：表示作为client，**testgenel向kernel发送msg**，testgenl将用户输入的字符和数字（日期）通过netlink传到testgenl.ko内核模块，并在kernel中将字符打印出来，testgenl.ko模块也会将字符串和数字（日期加一）返回给testgenl。
- 参数-s：表示作为server，**testgenel接收kernel netlink multicast的msg**，并打印接收到的msg。

##### testgenl.ko内核模块

- 当insmod testgenl.ko时，创建并注册family和multicast group；
- 启动timer，每隔10s发送multicast消息，testgenl用户程序绑定了对应的family和multicast gourp，就会收到消息；
- 可以接收testgenl用户程序发送的netlink消息；

#### netlink-with-socket

Netlink是一种userspace和kernel通信的机制。`netlink-with-socket`文件夹包含kernel-module和userspace-module，分别编译hello_netlink.ko内核模块和netlink_client用户程序。netlink_client将用户输入的字符通过netlink传到hello_netlink内核模块，并在kernel中将字符打印出来。

### 步骤

运行环境：VMware Linux ubuntu 4.15.0-142-generic

#### genetlink-with-libnl

进入`genetlink-with-libnl`目录执行下面命令：

```
sudo make
```

在根目录下面会产生testgenl.ko内核模块和testgenl用户程序。
在终端1中输入：

```
sudo insmod testgenl.ko
dmesg -Hw
```

ps:`dmesg -Hw`显示kernel log。
在终端2中输入：

```
./testgenl -c -s
```

#### netlink-with-socket

进入`netlink-with-socket`目录执行下面命令：

```
sudo make
```

在根目录下面会产生hello_netlink.ko内核模块和netlink_client用户程序。
在终端1中输入：

```
sudo insmod hello_netlink.ko
dmesg -Hw
```

ps:`dmesg -Hw`显示kernel log。
在终端2中输入：

```
./netlink_client
```

输入文本和kernel通信！

### 结果

#### genetlink-with-libnl

testgenl log:

```
root@ubuntu:~# ./testgenl -c -s
[INFO] [main.c:213] (main) - client mode:

[INFO] [main.c:128] (run_client) - start to send message
[INFO] [main.c:137] (run_client) - msg=Hello, I am userspace!, data=20220917
[INFO] [main.c:138] (run_client) - start to recv message
[INFO] [main.c:56] (recv_genl_msg) - reply from kernel
[INFO] [main.c:44] (recv_genl) - receive from kernel: message=I am message from kernel!, data=20220918
[INFO] [main.c:217] (main) - server mode:

[INFO] [main.c:183] (run_server) - The group is 4.

[INFO] [main.c:59] (recv_genl_msg) - multicast from kernel
[INFO] [main.c:44] (recv_genl) - receive from kernel: message=MULTICAST TEST, data=12345
```

kernel log:

```
[  +5.081569] receive from user: message=Hello, I am userspace!, data=20220917.
[  +0.000004] send to user: message=I am message from kernel!, data=20220918
[4月21 11:15] ----- Running timer -----
[  +0.000063] Newing message.
[  +0.000011] Adding Generic Netlink header to the message.
[  +0.000008] Nla_putting MSG(string) attribute.
[  +0.000005] Nla_putting DATA(u32) attribute.
[  +0.000005] Ending message.
[  +0.000005] Multicasting message.
[  +0.000006] The group ID is 4.
[  +0.000026] Success.
```

#### netlink-with-socket

netlink_client log:

```
root@ubuntu:~# ./netlink_client 
Sending message to kernel
Sendmsg ret: 1040
Waiting for message from kernel
Received message payload: hello, I am from kernel
Input your msg for sending to kernel: 123456              
Sending msg "123456" to kernel
send ret: 1040
Received message payload: hello, I am from kernel
Input your msg for sending to kernel: netlink
Sending msg "netlink" to kernel
send ret: 1040
Received message payload: hello, I am from kernel
Input your msg for sending to kernel: 
```

kernel log:

```
[  +1.072065] genetlink module exit successful
[4月21 11:22] Entering: hello_netlink_init, protocol family = 30 
[  +0.000001] hello_netlink_init ok
[ +14.754175] kernel recv msg from userspace: hello kernel, I am from userspace
[4月21 11:23] kernel recv msg from userspace: 123456
[ +21.328643] kernel recv msg from userspace: netlink
```