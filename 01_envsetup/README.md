# Harmony OS物联网应用开发实战 第一讲



本节课程中向大家演示——如何从零搭建Hamony OS开发环境，具体包括：

* Linux系统——源码下载和编译环境
  
  * VirtualBox（使用VMWare或者Win10的WSL也可以） + Ubuntu 18.04
  
  * VirtualBox虚拟机添加一个HostOnly网口，混杂模式：全部允许
  
    * 设置后这个网口可以和主机互通
  
    * 安装：`sudo apt install openssh-server`，完成后可以使用远程终端登录MobaXterm（或者Putty）
  
    * 安装samba：`sudo apt install samba`，添加账号`sudo smbpasswd -a $USER`，修改配置：
  
      ```
      
      ```
  
    * 
  
  * [Harmony OS 开发指南——源码下载和编译](hos_source_code_download_and_compile.md)
  
* Windows系统——源码编辑和烧录环境
  * [Harmony OS 开发指南——DevEco Device Tool 安装配置](hos_source_code_download_and_compile.md)





