### 简介
Netlink是一种userspace和kernel通信的机制。仓库包含kernel-module和userspace-module，分别编译hello_netlink.ko内核模块和netlink_client用户程序。netlink_client将用户输入的字符通过netlink传到hello_netlink内核模块，并在kernel中将字符打印出来。

### 步骤
运行环境：VMware Linux ubuntu 4.15.0-142-generic
进入目录执行下面命令：
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
![](https://user-images.githubusercontent.com/30982520/189602356-0db152ce-8a47-4caa-abef-f1ac0544c82f.png)
