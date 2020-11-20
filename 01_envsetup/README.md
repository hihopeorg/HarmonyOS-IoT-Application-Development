# 从零开始搭建HarmonyOS开发环境



本节课程中向大家演示——如何从零搭建Hamony OS开发环境，具体包括：

* Linux系统——源码下载和编译环境

  * [Harmony OS 开发指南——源码下载和编译](hos_source_code_download_and_compile.md)

* Windows系统——源码编辑和烧录环境

  * [Harmony OS 开发指南——DevEco Device Tool 安装配置](hos_deveco_device_tool_install.md)
  * [Harmony OS 开发指南——使用HiBurn烧录固件到开发板](hos_use_hiburn_download_firmware.md)

* Windows主机访问Linux主机

  * 支持远程终端访问

    * 安装openssh-server：`sudo apt install openssh-server`，完成后可以使用远程终端登录MobaXterm（或者Putty）

  * 支持Windows系统“映射网络驱动器”

    * 安装samba：`sudo apt install samba`，
    * 添加samba账号并设置密码：`sudo smbpasswd -a $USER`（其中$USER为用户名），
    * 修改配置（修改完需要重启服务`sudo service smbd restart`）：

    ```ini
      [home]
        comment = User Homes
        path = /home
        browsable = yes
        writable  = yes
        guest ok = no
        read only = no
        create mask = 0755
    ```

  * 如果是VirtualBox里面安装的Linux

    * 添加一个HostOnly的虚拟网卡，混杂模式：全部允许
    * 虚拟网卡设置成功后，Windows主机上应该可以ping通虚拟机

