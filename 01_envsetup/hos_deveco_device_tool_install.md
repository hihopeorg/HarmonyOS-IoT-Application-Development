# Harmony OS 开发指南——DevEco Device Tool 安装配置

本文介绍如何在Windows主机上安装DevEco Device Tool工具。

**坑点总结**：

1. 国内部分网络环境下，安装npm包可能会很慢或者超时，推荐使用国内npm源（如淘宝源、华为源等）；
2. serialport这个npm包安装的过程中需要编译源码，需要先安装windows-build-tools再安装这个包才会成功；
3. DevEco Device Tool中打包了预编译的.exe文件，只能在Windows版的VSCode中使用（暂不支持Linux和Mac OS）；

## 准备环境

本节介绍下载、编译鸿蒙源码需要的软硬件环境。

### 硬件环境

![开发环境](https://communityfile-drcn.op.hicloud.com/FileServer/getFile/cmtyPub/011/111/111/0000000000011111111.20200910235406.07666083971622121425650879377172:50510911011517:2800:A260456A9C92CDA9B13E2E3A942A5A015A20E4EC9CAEF0760A683198DDAB5578.png?needInitFileName=true?needInitFileName=true)

其中，Linux主机用于源码下载和编译，Windows主机用于烧写程序到单板以及源码编辑。



### 软件环境

| 硬件              | 说明                                                         |
| ----------------- | ------------------------------------------------------------ |
| Linux主机         | 推荐实际物理机器，Ubuntu16.04及以上64位系统，Shell使用bash，安装有samba、vim等常用软件，需要做适当适配以支持linux文件共享，（Samba配置需要注意，除了修改配置文件，还需要使用 `smbpasswd` 命令创建密码）；使用虚拟机也可以，但需要配置正确虚拟网络，这里不做详细介绍。 |
| Windows主机       | Windows XP/Windows7/Windows10 系统                           |
| USB转UART芯片驱动 | 对单板进行烧录和调试需要安装串口驱动程序，AI Camera和DIY IPC套件附赠的USB串口线内置PL2302芯片，需要安装USB转UART驱动，下载链接： http://www.hihope.org/download  页面，USB-to-Serial Comm Port.exe  文件。WiFi IoT主控芯片上集成的是CH340G芯片，可以选择联网自动安装驱动程序。 |
| 远程终端          | 推荐MobaXterm（PuTTY、SecureCRT等其他远程终端也可以），用于在Windows主机上登录Linux主机，进行源码下载、编译等工作 |



DevEco Device Tool以Visual Studio Code插件方式提供，安装分为如下几步：

1. 安装Visual Studio Code
2. 安装Node.js
3. 安装JDK
4. 安装hpm和**其他npm包**
5. 安装DevEco Device Tool插件
6. 安装C/C++插件

其中，下载nodejs、和JDK包可能较慢，安装hpm包


## 安装VSCode

DevEco Device Tool（简称DevEco插件）是一个Visual Studio Code（简称VSCode）的插件，需要先安装VSCode；

VSCode下载页面：https://code.visualstudio.com/Download，例如选择 System Installer 64 bit 下载

安装过程中，请勾选“添加到PATH（重启后生效）”，其他按照默认选项进行即可；



## 安装Node.js

DevEco插件的烧录功能依赖nodejs和相关插件，需要先安装nodejs和相关插件；

Node.js下载页面：https://nodejs.org/en/download/ 例如选择 64 bit Windows Installer (.msi) 下载

安装完成后，设置环境变量 `NODE_PATH` 为 `C:\Users\%USERNAME%\AppData\Roaming\npm\node_modules` 其中`%USERNAME%`为实际用户名；

Node.js 安装过程中提示，“Tools for Native Modules” 界面的复选框可以不用勾选（勾选该选项后需要从npm官网下载安装vs编译工具包和Python，耗时较长，后续安装serial插件之前，使用国内镜像单独安装速度较快）；



## 安装JDK

**说明** 需要安装JDK 1.8，如果已安装JDK，打开命令行工具，输入**java -version**命令，检查版本号是否1.8版本。

请自行下载并安装JDK，版本要求为：1.8版本。

Windows版的OpenJDK可以在JetBrains下载：https://confluence.jetbrains.com/display/JBR/JetBrains+Runtime

或者从红帽下载（有CDN加速，下载速度较快）： https://developers.redhat.com/products/openjdk/download

下载压缩包版本可以免安装，解压后配置bin目录到PATH 即可使用（CLASSPATH 可以不配置）；



## 安装hpm和其他npm包

1. 使用国内镜像源，加速模块下载速度：`npm config set registry https://registry.npm.taobao.org`

2. 安装hpm 鸿蒙组件管理器：`npm install -g @ohos/hpm-cli`

3. 安装Windows编译工具包，serialport 包需要编译本地模块，必须先安装此包才能安装 serialport 包：

   `npm install -g windows-build-tools`

4. 安装 tftp 包，用于启动tftp服务器，通过网络方式向单板烧录映像：

   `npm install -g tftp`

5. 安装 serialport 包，用于通过串口烧写映像：`npm install -g serialport`
   `npm install -g @serialport/parser-readline`

```sh
npm config set registry https://registry.npm.taobao.org
npm install -g @ohos/hpm-cli
npm install -g windows-build-tools
npm install -g tftp
npm install -g serialport
npm install -g @serialport/parser-readline
```



## 下载DevEco插件

1. 登录[HarmonysOS设备开发门户](https://device.harmonyos.com/cn/home)，点击右上角**注册**按钮，注册开发者帐号，注册指导参考[注册华为帐号](https://developer.huawei.com/consumer/cn/doc/20300)。如果已有华为开发者帐号，请直接点击**登录**按钮。

2. 进入[HUAWEI DevEco Device Tool产品页](https://device.harmonyos.com/cn/ide)，下载DevEco Device Tool安装包。

3. 打开Visual Studio Code软件。

4. 采用从本地磁盘安装方式，安装DevEco Device Tool。

   ![点击放大](https://communityfile-drcn.op.hicloud.com/FileServer/getFile/cmtyPub/011/111/111/0000000000011111111.20200914093622.93560046166375321457328824276949:50510914020140:2800:6D71CB138B0590368E613ED5A7B302262DE31733629047F66500DF27757F2853.gif?needInitFileName=true?needInitFileName=true)

   安装成功后，界面如下图所示。

   ![点击放大](https://communityfile-drcn.op.hicloud.com/FileServer/getFile/cmtyPub/011/111/111/0000000000011111111.20200914093622.37832007141604464627097125460649:50510914020140:2800:F6288E3BA1FDA293B52D1BA2F6FFBF4F6DDB53DD792FB42E66F234A75AA1BB85.png?needInitFileName=true?needInitFileName=true)



## 安装C/C++插件

HarmonysOS设备开发采用C/C++语言，Visual Studio Code工具对C/C++语言的支持插件非常丰富，可以安装“C/C++”插件来实现C/C++代码智能提示、代码高亮、格式化等功能，帮助您更高效、便利的开发更规范和优美的代码。

在Visual Studio Code的插件市场，输入“C/C++”，然后点击“Install”。

![点击放大](https://communityfile-drcn.op.hicloud.com/FileServer/getFile/cmtyPub/011/111/111/0000000000011111111.20200914093623.01024343525247339263582244765607:50510914020140:2800:8CECD4F3E002EEED38F29A6832DBA7A98F78ACCA9AA2FD36341147DA52300C96.png?needInitFileName=true?needInitFileName=true)

如果在线安装失败，可点击右下角弹框的“Go to Download Page”，下载离线插件（选择“cpptools-win32.vsix”版本，Windows下，64位和32位系统均为同一个安装包）。然后再参考[安装DevEco Device Tool插件的方式](https://device.harmonyos.com/cn/docs/ide/user-guides/tool_install-0000001050164976#ZH-CN_TOPIC_0000001050164976__section4336315185716)，离线安装C/C++插件。

![img](https://communityfile-drcn.op.hicloud.com/FileServer/getFile/cmtyPub/011/111/111/0000000000011111111.20200914093623.16985116067731605372818769568785:50510914020140:2800:0B1E3CA592229336927B6953172125E21AE192B930A3BE4A977382875E06BAD2.png?needInitFileName=true?needInitFileName=true)