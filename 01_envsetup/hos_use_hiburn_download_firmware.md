# 使用HiBurn烧录鸿蒙.bin文件到Hi3861开发板



鸿蒙官方文档的“Hi3861开发板第一个示例程序”中描述了——如何使用DevEco Device Tool工具烧录二进制文件到Hi3861开发板；

本文将介绍如何使用HiBurn工具烧录鸿蒙的.bin文件到Hi3861开发板。

 

**获取HiBurn工具**
通过鸿蒙官方文档我们知道DevEco Device Tool是一个VSCode插件，它以.vsix文件的形式向开发者提供。事实上，这个文件.vsix文件是一个zip格式的压缩文件。你可以在git bash或Linux环境中，使用file命令将它识别出来：

```sh
$ file DevEcoDeviceTool-1.0.0.vsix
DevEcoDeviceTool-1.0.0.vsix: Zip archive data, at least v2.0 to extract
```

 

在Windows上，你可以使用一般的压缩解压软件将它打开，并将它解压开。

解压之后，可以在extension\deveco\tools子目录中看到一个文件名为 HiBurn.exe 的文件，它就是我们今天要介绍的HiBurn工具（你也可以从本文的附件中直接下载 HiBurn.zip 解压）。

 

## 使用HiBurn烧写.bin文件到Hi3861
Win10系统执行前需要右键“属性”->解除锁定，否则会系统默认会报安全警告，不允许执行。

双击单开后，界面如下：

![img](https://dl-harmonyos.51cto.com/images/202010/62e2d14388024bc25ea071e678acc6db7e4af6.png)

 

### 操作步骤

1. 点界面左上角的Setting->Com settings进入串口参数设置界面，串口参数设置界面上，Baud为波特率，默认115200，可以选择921600，2000000，或者 3000000（实测最快支持的值），其他参数保持默认，点“确定”保存；

 ![img](https://dl-harmonyos.51cto.com/images/202010/3529fa7845bbfe985c05740eaa26b7ae5b4fa3.png)

2. 根据设备管理器，选择正确的COM口，例如我的开发板是COM8，如果是打开程序之后才插串口线的，可以点一下“Refresh”刷新串口下拉框的可选项；

  ![img](https://dl-harmonyos.51cto.com/images/202010/a8cd10f109e13ce9fee494013d969f27d908e4.png)![img](https://dl-harmonyos.51cto.com/images/202010/e563b8e72b63f37a2b86119887702351b7fd24.png)

3. 点“Select file”弹出文件选择对话框，选择编译生成的allinone.bin文件，这个bin其实是多个bin合并的文件，从命名上也能看得出来，例如，我选择的Z:\harmonyos\openharmony\out\wifiiot\Hi3861_wifiiot_app_allinone.bin
   勾选“Auto burn”，自动下载多个bin文件，到这里，配置完毕，应如下图；

 ![img](https://dl-harmonyos.51cto.com/images/202010/618209d31a6fdd6375550620829aa19585f646.png)

4. 点击Connect，连接串口设备，这时HiBurn会打开串口设备，并尝试开始烧写，需要确保没有其他程序占用串口设备（烧写之前可能正在用超级终端或串口助手查看串口日志，需要确保其他软件已经关闭了当前使用的串口）；
   复位设备，按开发板的RESET按键；
   等待输出框出现三个"=========================================="以及上方均出现successful，即说明烧录成功；

5. 烧录成功后，需要手动点“Disconnect”断开串口连接，否则会提示“Wait connect success flag (hisilicon) overtime.”；
   
   

### 和 DevEco Device Tool方式对比
使用HiBurn烧录相对于使用DevEco Device Tool烧录而言，好处主要有以下几点：

1. 不依赖VSCode，你可以不用安装VSCode、nodejs、JDK、以及一些npm包；
2. 下载速度更快，HiBurn.exe最大波特率可以设置到3000000，而DevEco Device Tool最大只能为921600，是它的三倍；

 

HiBurn方式烧录目前的缺点主要是：

1. 需要手动点Disconnect主动断开连接，否则默认会重复下载；烧录成功后，如果你不断开串口，并且再次按了一下RESET按键，你会发现，它又烧录了一遍。
2. HiBurn的串口参数无法保存，你关闭之后下次打开还需要重新设置，而DevEco则会可以保存串口参数；
3. HiBurn相对DevEco来说，操作步骤更多一些，也稍微复杂一点；

 

 

 

 

 

 

 

 

 

 