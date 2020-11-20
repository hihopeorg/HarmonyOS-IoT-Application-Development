# Harmony OS 开发指南——源码下载和编译

本文介绍了如何下载鸿蒙系统源码，如何一次性配置可以编译三个目标平台（`Hi3516`，`Hi3518`和`Hi3861`）的编译环境，以及如何将源码编译为三个目标平台的二进制文件。

**坑点总结**：

1. 下载源码基本上没有太多坑，可以很顺利的进行
2. 编译源码主要的一个大坑是，默认版本的`scons`依赖python 3.7+，鸿蒙基础编译代码依赖`python3`，需要安装python 3.7+，并和当前系统上的python2.7/python3.6和谐共处！解决方法一般有两种：
   * 使用`virtualenv`管理多个版本的python环境，这种方式好处是只在当前shell会话生效不影响系统环境、其他用户和其他shell会话，缺点是使用前需要source active脚本稍微有点麻烦；
   * 使用`update-alternatives`管理系统的多个版本python命令，这种方式需要切换系统的链接`/usr/bin/python`和`/usr/bin/python3`，可能会影响依赖python的应用程序（例如`apt`）的正常使用、其他用户和其他shell会话；

## 准备环境

本节介绍下载、编译和烧写鸿蒙系统所需的软硬件环境。

### 硬件环境

![开发环境](https://communityfile-drcn.op.hicloud.com/FileServer/getFile/cmtyPub/011/111/111/0000000000011111111.20200910235406.07666083971622121425650879377172:50510911011517:2800:A260456A9C92CDA9B13E2E3A942A5A015A20E4EC9CAEF0760A683198DDAB5578.png?needInitFileName=true?needInitFileName=true)

其中，Linux主机用于源码下载和编译，Windows主机用于烧写程序到单板以及源码编辑。



### 软件环境

| 硬件              | 说明                                                         |
| ----------------- | ------------------------------------------------------------ |
| Linux主机         | 推荐实际物理机器，Ubuntu16.04及以上64位系统，Shell使用bash，使用虚拟机也可以，如何安装这里不做详细介绍。 |
| Windows主机       | Windows XP/Windows7/Windows10 系统                           |
| 远程终端          | 推荐MobaXterm（PuTTY、SecureCRT等其他远程终端也可以），用于在Windows主机上登录Linux主机，进行源码下载、编译等工作 |
| USB转串口芯片驱动 | 下载链接： http://www.hihope.org/download  页面，USB-to-Serial Comm Port.exe  文件（AI Camera和DIY IPC套件附赠的USB串口线中集成了PL2302芯片，需要安装此驱动才能识别）。WiFi IoT主控芯片上集成的是CH340G芯片，可以选择联网自动安装驱动程序。 |



**本文所有操作均基于Ubuntu 18.04进行，其他发行版和Ubuntu版本上的命令操作可能不完全一致，但说明的原理可以互通。**



## 下载源码

本节介绍如何在Linux主机上下载鸿蒙源码，所有命令均通过远程终端在Linux主机上执行。

这里使用的是“从代码仓库获取”的方式，这种方式方便后续更新代码；你也可以从官网下载源码压缩包（这里不再介绍）。

### 配置 repo 工具 

如果你的Linux系统上还没有配置repo命令，需要先下载并配置repo命令行工具：

```sh
mkdir ~/bin/
# sudo apt install curl # 如果没有curl命令需要先下载
curl https://gitee.com/oschina/repo/raw/fork_flow/repo-py3 > ~/bin/repo
chmod +x ~/bin/repo
echo 'export PATH=~/bin:$PATH'  >> ~/.bashrc
source ~/.bashrc
```

### 下载Harmony OS源码

```sh
mkdir -p ~/harmonyos/openharmony && cd ~/harmonyos/openharmony
sudo apt install --no-install-recommends git python # repo工具本身是python脚本，它会调用git命令下载单个代码仓
# 开始前需要配置`user.name`和`user.email`，如果没有配置，使用如下命令进行配置：
# git config --global user.name "yourname"
# git config --global user.email "your-email-address"
repo init -u https://gitee.com/openharmony/manifest.git -b master --no-repo-verify
repo sync -c  # 以后每天同步远程仓的修改，只需要执行这一条命令即可
```



## 配置环境

本节介绍如何在Linux主机上配置鸿蒙源码的编译构建环境，所有命令均通过远程终端在Linux主机上执行。

### 安装文件系统打包工具

1. 运行"mkfs.vfat"，如果未找到该命令，需要安装

2. 运行"mcopy "，如果未找到该命令，需要安装


```sh
sudo apt-get install dosfstools mtools  # 官方文档说明的两个文件系统打包工具
sudo apt-get install zip       # 官方文档虽然没有写，但是打包rootfs过程中需要使用
sudo apt install mtd-utils     # 3518需要打包 jffs2 镜像的mkfs.jffs2 命令，需要安装这个包
sudo apt install build-essential # 安装gcc/g++/make等工具，ffmpeg的测试过程中需要使用链接器ld，需要这个包
```

###  下载、配置编译工具链

使用如下命令，分别下载 gn、ninja、LLVM、hc-gen包，根据官方文档修改，一步到位，不用反复复制粘贴！

```sh
# 下载gn/ninja/LLVM/hc-gen包
URL_PREFIX=https://repo.huaweicloud.com/harmonyos/compiler
DOWNLOAD_DIR=~/Downloads                # 下载目录，可自行修改
TOOLCHAIN_DIR=~/harmonyos/toolchain     # 工具链存放目录，可自行修改

[ -e $DOWNLOAD_DIR ] || mkdir $DOWNLOAD_DIR
[ -e $TOOLCHAIN_DIR ] || mkdir -p $TOOLCHAIN_DIR

wget -P $DOWNLOAD_DIR $URL_PREFIX/gn/1523/linux/gn.1523.tar
wget -P $DOWNLOAD_DIR $URL_PREFIX/ninja/1.9.0/linux/ninja.1.9.0.tar
wget -P $DOWNLOAD_DIR $URL_PREFIX/clang/9.0.0-34042/linux/llvm-linux-9.0.0-34042.tar
wget -P $DOWNLOAD_DIR $URL_PREFIX/hc-gen/0.65/linux/hc-gen-0.65-linux.tar

# 编译 hi3861 需要 riscv 编译工具链
wget -P $DOWNLOAD_DIR $URL_PREFIX/gcc_riscv32/7.3.0/linux/gcc_riscv32-linux-7.3.0.tar.gz

# 解压gn/ninja/LLVM/hc-gen包：
tar -C $TOOLCHAIN_DIR/ -xvf $DOWNLOAD_DIR/gn.1523.tar
tar -C $TOOLCHAIN_DIR/ -xvf $DOWNLOAD_DIR/ninja.1.9.0.tar
tar -C $TOOLCHAIN_DIR/ -xvf $DOWNLOAD_DIR/llvm-linux-9.0.0-34042.tar
tar -C $TOOLCHAIN_DIR/ -xvf $DOWNLOAD_DIR/hc-gen-0.65-linux.tar
tar -C $TOOLCHAIN_DIR/ -xvf $DOWNLOAD_DIR/gcc_riscv32-linux-7.3.0.tar.gz

# 向 ~/.bashrc 中追加gn/ninja/LLVM/hc-gen路径配置：
cat <<EOF >> ~/.bashrc
TOOLCHAIN_DIR=$TOOLCHAIN_DIR
export PATH=\$TOOLCHAIN_DIR/gn:\$PATH
export PATH=\$TOOLCHAIN_DIR/ninja:\$PATH
export PATH=\$TOOLCHAIN_DIR/llvm/bin:\$PATH
export PATH=\$TOOLCHAIN_DIR/hc-gen:\$PATH
export PATH=\$TOOLCHAIN_DIR/gcc_riscv32/bin:\$PATH
export PATH=~/.local/bin:\$PATH       # 用户pip二进制工具目录
EOF

# 生效环境变量
source ~/.bashrc
```

### 安装python3.8和pip包管理器

* 安装python3.8和pip（编译鸿蒙源码`hi3861`目标平台需要使用scons，默认安装的scons版本需要python版本>=3.7）：

  * `sudo apt-get install --no-install-recommends python3.8 python3-pip`
  
* 将默认python切换为3.8（因为18.04 上默认Python3版本是 3.6），通常有两种方式切换：

  * 使用`virtualenv`管理，单独source相应的active脚本切换当前shell会话的默认python版本（推荐，下面均已这种方式进行操作）；
  * 使用 `update-alternatives`管理，并切换全局Python脚本配置（不推荐，用完需要切换回去，否则会影响apt等依赖python3的软件包使用）；

* 配置pip包下载源，加速国内安装pip包：

  ```sh
  mkdir ~/.pip/
  cat <<EOF > ~/.pip/pip.conf
  [global]
  index-url = https://mirrors.huaweicloud.com/repository/pypi/simple
  trusted-host = mirrors.huaweicloud.com
  timeout = 120
  EOF
  ```

### 准备virtualenv

```sh
# 安装virtualenv
pip3 install virtualenv

# 创建使用python3.8为默认python解释器的virtualenv
mkdir ~/harmonyos/venv && virtualenv -p python3.8 ~/harmonyos/venv

# 激活 virtualenv，激活后的pip3 install会将包文件缓存到相应的子目录中
source ~/harmonyos/venv/bin/activate

# 安装 setuptools 和 kconfiglib
pip3 install setuptools kconfiglib

# 安装编译hi3861需要的pip包
pip3 install scons ecdsa pycryptodome
# pip3 install --upgrade --ignore-installed six

# 可选：将激活脚本添加到 bashrc 中，下次登录默认自动激活此python虚拟环境，可以使用 deactivate 使虚拟环境无效
cat <<EOF >> ~/.bashrc
source ~/harmonyos/venv/bin/activate
EOF
```



## 编译源码

激活刚刚建立的python虚拟环境后，在源码根目录下（`openharmony`目录）执行：

```sh
$ source ~/harmonyos/venv/bin/activate # 激活虚拟环境，激活后 deactivate 命令可使当前虚拟环境无效
$ python build.py -h # 查看支持哪些目标平台
usage:
  python build.py ipcamera_hi3516dv300
  python build.py ipcamera_hi3518ev300
  python build.py wifiiot

  Quickstart: https://device.harmonyos.com/cn/docs/start/introduce/oem_start_guide-0000001054913231

positional arguments:
  product               Name of the product

optional arguments:
  -h, --help            show this help message and exit
  -b BUILD_TYPE, --build_type BUILD_TYPE
                        release or debug version.
  -t [TEST [TEST ...]], --test [TEST [TEST ...]]
                        Compile test suit
  -n, --ndk             Compile ndk
```

可以查看支持哪些目标平台，目前可选的有三个。

### 切换默认shell为bash

`vendor`目录下的部分Makefile里面使用了bash特有的`pushd`/`popd`命令，需要使用bash环境才能编译通过，有两种方法切换默认shell：

1. `sudo dpkg-reconfigure dash`弹出对话框之后选择No；
2. `sudo rm /bin/sh && sudo ln -s /bin/bash /bin/sh`；

### 编译3516目标平台的命令

```sh
python build.py ipcamera_hi3516dv300 -b debug
```

编译生成的kernel、rootfs、userfs映像文件会生成到`out/ipcamera_hi3516dv300`目录下，u-boot二进制会生成在`vendor`目录（可以通过`find vendor -name u-boot*.bin`命令进行查找）。

### 编译3518目标平台的命令

```sh
python build.py ipcamera_hi3516dv300 -b debug
```

编译生成的kernel、rootfs、userfs映像文件会生成到`out/ipcamera_hi3518ev300`目录下，u-boot二进制会生成在`vendor`目录（可以通过`find vendor -name u-boot*.bin`命令进行查找）。

### 编译3861目标平台的命令

```sh
python build.py wifiiot
```

编译生成的二进制文件位于`out/wifiiot/`子目录下，刷机需要使用`Hi3861_wifiiot_app_allinone.bin`文件。



### ffmpeg相关编译失败问题定位

编译过程中可能会遇到ffmpeg相关的编译失败，可能是因为`configure`过程中出现了问题，可以通过`vendor/hisi/hi35xx/middleware/source/third_party/ffmpeg/ffmpeg-y/ffbuild/config.log`文件查看。



