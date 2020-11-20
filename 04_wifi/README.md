# Harmony OS WiFi编程——连接热点、创建热点

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



鸿蒙WiFi API参考：https://gitee.com/openharmony/docs/blob/master/api/api-LinkIoT/Wifiservice.md

想要简单易用WiFi接口，可以使用我们封装的：https://gitee.com/hihopeorg/easy_wifi

## STA模式编程指南

### STA模式API

| API                                                          | 功能说明                                |
| ------------------------------------------------------------ | --------------------------------------- |
| `WifiErrorCode EnableWifi(void);`                            | 开启STA                                 |
| `WifiErrorCode DisableWifi(void);`                           | 关闭STA                                 |
| `int IsWifiActive(void);`                                    | 查询STA是否已开启                       |
| `WifiErrorCode Scan(void);`                                  | 触发扫描                                |
| `WifiErrorCode GetScanInfoList(WifiScanInfo* result, unsigned int* size);` | 获取扫描结果                            |
| `WifiErrorCode AddDeviceConfig(const WifiDeviceConfig* config, int* result);` | 添加热点配置，成功会通过result传出netId |
| `WifiErrorCode GetDeviceConfigs(WifiDeviceConfig* result, unsigned int* size);` | 获取本机所有热点配置                    |
| `WifiErrorCode RemoveDevice(int networkId);`                 | 删除热点配置                            |
| `WifiErrorCode ConnectTo(int networkId);`                    | 连接到热点                              |
| `WifiErrorCode Disconnect(void);`                            | 断开热点连接                            |
| `WifiErrorCode GetLinkedInfo(WifiLinkedInfo* result);`       | 获取当前连接热点信息                    |
| `WifiErrorCode RegisterWifiEvent(WifiEvent* event);`         | 注册事件监听                            |
| `WifiErrorCode UnRegisterWifiEvent(const WifiEvent* event);` | 解除事件监听                            |
| `WifiErrorCode GetDeviceMacAddress(unsigned char* result);`  | 获取Mac地址                             |
| `WifiErrorCode AdvanceScan(WifiScanParams *params);`         | 高级搜索                                |



### 扫描WiFi热点

在Harmony OS上STA模式扫描其他WiFi热点，需要注意以下事项

1. 功能相关接口都有`WifiErrorCode`类型的返回值：

   * 需要接收并判断返回值是否为`WIFI_SUCCESS`，用于确认是否调用成功；
   * 不为`WIFI_SUCCESS`表示失败，通过枚举值查找错误原因；

2. `EnableWifi`接口使能STA模式之前：
   
   * 需要使用`RegisterWifiEvent`接口，向系统注册状态监听函数，用于接收状态通知，STA模式需要绑定如下两个回调函数；
     * `OnWifiScanStateChanged`用于绑定扫描状态监听函数，该回调函数有两个参数`state`和`size`；
       * `state`表示扫描状态，取值为0和1，1表示扫描动作完成；编程时可以与`WifiEventState`枚举值的`WIFI_STATE_NOT_AVALIABLE`，`WIFI_STATE_AVALIABLE`进行比较，避免魔法数字；
    * `size`表示扫描到的热点个数；
     * `OnWifiConnectionChanged`用于绑定WiFi连接状态监听函数，该回调函数有两个参数`state`和`info`；
       * `state`表示WiFi连接状态，连接成功是`WIFI_STATE_AVALIABLE`值为1，连接失败时`WIFI_STATE_NOT_AVALIABLE`值为0；
       * `info`类型为`WifiLinkedInfo*`，`WifiLinkedInfo`有多个成员，包括`ssid`，`bssid`，`rssi`，`connState`，`disconnectedReason`；
   
3. `Scan`接口只是触发扫描动作，并不会等到扫描完成才返回；

4. 调用`Scan`接口进行扫描之前，

   * 需要确保已经调用`EnableWifi`接口，并成功使能了STA模式；

5. 扫描状态监听回调函数内，不能直接调用`GetScanInfoList`函数（否则会有运行时异常报错），正确用法：

   * 可以在状态更新回调函数中更新全局状态变量，另外一个线程中轮训状态变量，这种方式实现起来比较简单；但需要保证更新和查询操作的原子性（可以使用gcc内置原子操作函数），逻辑才是严格正确的；
   * 或者使用信号量进行通知，这种方式更好一些，更优雅；

6. 扫描完成后要及时调用`GetScanInfoList`函数获取扫描结果；

   * 如果间隔时间太长（例如5秒以上），可能会无法获得上次扫描结果；

7. `GetScanResult`函数有两个参数：
   
   * 第一个参数`result`指向用于存放结果的数组，需要大于等于`WIFI_SCAN_HOTSPOT_LIMIT`，
   * 第二个参数`size`类型为指针是为了内部能够修改它的值，返回后size指向的值是实际搜索到的热点个数；
* 调用`GetScanResult`函数前，第二个参数`size`指向的实际值不能为0，否则会包参数错误；
  
8. `DisableWifi`关闭STA模式；

   

### 连接WiFi热点

在Harmony OS上STA模式连接其他WiFi热点，需要注意的事项和扫描WiFi热点类似，一下介绍差异的部分：

1. 首先，需要使用`RegisterWifiEvent`接口，向系统注册状态监听函数，STA模式需要绑定两个回调；
2. 然后，通过`AddDeviceConfig`接口，向系统添加热点配置，它有两个参数；
   * 第一个参数`config`，类型为`const WifiDeviceConfig*`，用于指定热点配置；
   * 第二个参数`result`，类型为`int*`，用于操作成功时返回`netId`；
3. 连接热点使用`ConnectTo`接口，他有一个参数；
   * `netId`，类型为`int`，应该使用`AddDeviceConfig`接口调用成功之后`result`参数得到值填充；
4. `ConnectTo`是同步的，连接成功失败会通过返回值体现；
   * 同时，系统也会通过回调函数通知应用代码；
5. 连接成功后，需要调用DHCP客户端，从热点获取IP地址；
   * `struct netif* iface = netifapi_netif_find("wlan0");` 获取STA模式的网络接口
   * `err_t netifapi_dhcp_start(struct netif*)`接口，启动DHCP客户端；
6. 断开热点使用`Disconnect`接口，无需参数，端口之前需要：
   * `err_t netifapi_dhcp_stop(struct netif*)`接口，停止DHCP客户端；
7. 删除热点配置使用`RemoveDevice`，参数和`ConnectTo`类似，
   * 需要使用`AddDeviceConfig`接口调用成功之后`result`参数得到值填充；



## AP模式编程指南

### AP模式API接口

| API                                                          | 说明                 |
| ------------------------------------------------------------ | -------------------- |
| `WifiErrorCode EnableHotspot(void);`                         | 打开AP模式           |
| `WifiErrorCode DisableHotspot(void);`                        | 关闭AP模式           |
| `WifiErrorCode SetHotspotConfig(const HotspotConfig* config);` | 设置当前热点配置参数 |
| `WifiErrorCode GetHotspotConfig(HotspotConfig* result);`     | 获取当前热点配置参数 |
| `int IsHotspotActive(void);`                                 | 查询AP是否已开启     |
| `WifiErrorCode GetStationList(StationInfo* result, unsigned int* size);` | 获取接入的设备列表   |
| `int GetSignalLevel(int rssi, int band);`                    | 获取信号强度等级     |
| `WifiErrorCode SetBand(int band);`                           | 设置当前频段         |
| `WifiErrorCode GetBand(int* result);`                        | 获取当前频段         |



### 创建WiFi热点

在Harmony OS上创建WiFi热点，通用有一些事项需要注意：

1. 首先，需要使用`RegisterWifiEvent`接口，向系统注册扫描状态监听函数，用于接收扫描状态通知，AP模式需要绑定三个函数；
   * `OnHotspotStaJoin`回调函数，其他设备连上当前热点时会被调用，参数`StationInfo* info`，其中包含`macAddress`和`disconnectedReason`；
   * `OnHotspotStaLeave`回调函数，其他设备断开当前热点时会被调用，参数`StationInfo* info`；
   * `OnHotspotStateChanged`回调函数，当热点本身状态变化时会被调用，参数`int state`表示热点状态；
2. 然后，通过`SetHotspotConfig`接口，向系统设置当前热点配置信息；
3. 是用`EnableHotspot`接口，开启热点，无需参数；
4. 热点开启成功之后，需要启动DHCP服务端，Hi3861上使用如下SDK接口；
   * `g_iface = netifapi_netif_find("ap0");`，获取AP模式的网口
   * 使用`netifapi_netif_set_addr`接口设置热点本身的IP地址、网关、子网掩码；
   * 使用`netifapi_dhcps_start`接口启动DHCP服务端；
   * 使用`netifapi_dhcps_stop`接口停止DHCP服务端；
5. 使用`DisableHotspot`接口关闭热点；
6. 使用`UnRegisterWifiEvent`接口解除事件监听；



## 如何编译

有两种方法可以编译此目录下的样例程序：

1. 前两章中的方法——将当前目录下的`*.c`文件和`BUILD.gn`拷贝到openharmony源码的`applications\sample\wifi-iot\app\iothardware`目录下；

   1. 再修改openharmony源码的`applications\sample\wifi-iot\app\BUILD.gn`文件，将其中的 `features` 改为：`iothardware:wifi_demo`；
   2. 再修改`applications\sample\wifi-iot\app\iothardware\BUILD.gn`文件，决定需要编译哪个`.c`文件；
   3. 在openharmony源码顶层目录执行：`python build.py wifiiot`

2. 可以将本仓整体拷贝到openharmony源码树下，和`applications`同级；

   1. 修改openharmony源码的`build\lite\product\wifiiot.json`文件，将其中的:

      `//applications/sample/wifi-iot/app`替换为：`//HarmonyOS-IoT-Application-Development:app`

   2. 在openharmony源码顶层目录执行：`python build.py wifiiot`


