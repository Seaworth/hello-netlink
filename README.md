### 简介
#### netlink-with-socket
Netlink是一种userspace和kernel通信的机制。`netlink-with-socket`文件夹包含kernel-module和userspace-module，分别编译hello_netlink.ko内核模块和netlink_client用户程序。netlink_client将用户输入的字符通过netlink传到hello_netlink内核模块，并在kernel中将字符打印出来。
#### genetlink-with-libnl
`genetlink-with-libnl`文件夹包含kernel-module和userspace-module，分别编译testgenl.ko内核模块和testgenl用户程序。testgenl将用户输入的字符和数字（日期）通过netlink传到testgenl.ko内核模块，并在kernel中将字符打印出来，testgenl.ko模块也会将字符串和数字（日期加一）返回给testgenl。

### 步骤
运行环境：VMware Linux ubuntu 4.15.0-142-generic
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
./testgenl
```

### 结果
#### netlink-with-socket
![](https://user-images.githubusercontent.com/30982520/189602356-0db152ce-8a47-4caa-abef-f1ac0544c82f.png)
#### genetlink-with-libnl
![](https://user-images.githubusercontent.com/30982520/190918036-a73ec806-9ff4-4e9c-9f19-18177a615ca7.png)