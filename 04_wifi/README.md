# Harmony OS物联网应用开发实战 第四讲

**本节主要介绍如何在HiSpark WiFi IoT套件上使用Hamony OS的WiFi相关编程接口。**




## 相关知识点

* WiFi的工作模式
  * AP模式：热点模式，提供无线接入服务，允许其它无线设备接入，提供数据访问，一般的无线路由/网桥工作在该模式。
  * STA模式：类似于无线终端，本身并不接受其他设备的接入，它可以连接到AP，一般无线网卡即工作在该模式。
* Harmony OS的WiFi相关API头文件位于`foundation\communication\interfaces\kits\wifi_lite\wifiservice`目录，该目录下有9个文件；
  * `wifi_device.h`中定义的是STA模式的主要接口，例如扫描其他热点、添加热点配置（热点名称、密码等）、连接其他热点；
  * `wifi_hotspot.h`中定义的是AP模式的主要接口，例如设置热点信息（热点名称、密码等）、查询连接的设备列表；
  * `wifi_hotsport_config.h`中定义了设置和获取当前工作在2.4G或者5G频段的接口`SetBand`和`GetBand`；
  * 另外6个文件中定义了上述接口相关的类型，例如扫描结果、热点配置、热点连接状态等；



## STA模式编程指南

### 扫描WiFi热点

经实际试验，发现在Harmony OS上扫描其他WiFi热点，需要注意以下事项

1. 功能相关接口都有`WifiErrorCode`类型的返回值：

   * 需要接收并判断返回值是否为`WIFI_SUCCESS`，用于确认是否调用成功；

   * 不为`WIFI_SUCCESS`表示失败，通过枚举值查找错误原因；

2. `EnableWifi`接口使能STA模式，`DisableWifi`关闭STA模式；

3. `Scan`接口只是触发扫描动作，并不会等到扫描完成才返回；

4. 调用`Scan`接口进行扫描之前，

   * 需要确保已经调用`EnableWifi`接口，并成功使能了STA模式；

   * 需要通过`RegisterWifiEvent`接口向系统注册扫描状态监听函数，用于获知扫描状态，如扫描动作是否完成等；
     * `OnWifiScanStateChanged`用于绑定扫描状态监听函数；
     * 回调函数有两个参数`state`和`size`；
     * `state`表示扫描状态，取值为0和1，1表示扫描动作完成；编程时可以与`WifiEventState`枚举值的`WIFI_STATE_NOT_AVALIABLE`，`WIFI_STATE_AVALIABLE`进行比较，避免魔法数字；
     * `size`表示扫描到的热点个数；

5. 扫描状态监听函数内部不能调用`GetScanInfoList`函数；

   * 可以在状态更新回调函数中更新全局状态变量，另外一个线程中轮训状态变量；
   * 或者，使用信号量进行通知；

6. 扫描完成后要及时调用`GetScanInfoList`函数获取扫描结果；

   * 如果间隔时间太长（例如5秒以上），可能会无法获得上次扫描结果；

   


## 如何编译

有两种方法可以编译此目录下的样例程序：

1. 前两讲中的方法——将当前目录下的`*.c`文件和`BUILD.gn`拷贝到openharmony源码的`applications\sample\wifi-iot\app\iothardware`目录下；

   1. 再修改openharmony源码的`applications\sample\wifi-iot\app\BUILD.gn`文件，将其中的 `features` 改为：`iothardware:wifi_demo`；
   2. 再修改`applications\sample\wifi-iot\app\iothardware\BUILD.gn`文件，决定需要编译哪个`.c`文件；
   3. 在openharmony源码顶层目录执行：`python build.py wifiiot`

2. 可以将本仓整体拷贝到openharmony源码树下，和`applications`同级；

   1. 修改openharmony源码的`build\lite\product\wifiiot.json`文件，将其中的:

      `//applications/sample/wifi-iot/app`替换为：`//HarmonyOS-IoT-Application-Development:app`

   2. 在openharmony源码顶层目录执行：`python build.py wifiiot`


