# Harmony OS物联网应用开发实战

《Harmony OS物联网应用开发实战》课程资料（包含代码）

**持续更新中，欢迎点右上角的 Star 关注本项目**



## 课程目录

* 第一讲 [从零开始搭建HarmonyOS开发环境](01_envsetup/README.md)
  * [Harmony OS 开发指南——源码下载和编译](01_envsetup/hos_source_code_download_and_compile.md)
  * [Harmony OS 开发指南——DevEco Device Tool 安装配置](01_envsetup/hos_source_code_download_and_compile.md)
* 第二讲 [使用Harmony OS控制外设——输入输出](02_device_control/README.md)
  * Harmony OS IoT内核（liteos-m）接口——[CMSIS-RTOS API v2](https://arm-software.github.io/CMSIS_5/RTOS2/html/rtos_api2.html)
  * Harmony OS GPIO相关接口的使用（具体参考`wifiiot_gpio.h`中的注释）
  * Harmony OS PWM相关接口的使用（具体参考`wifiiot_pwm.h`中的注释）
* 第三讲 [使用Harmony OS控制外设——读取传感器](03_sensing/README.md)
  * Harmony OS ADC相关接口的使用（具体参考`wifiiot_adc.h`中的注释）
  * Harmony OS I2C相关接口的使用（具体参考`wifiiot_i2c.h`中的注释）
* 第四讲 [使用Harmony OS控制WiFi——连接热点、创建热点](04_wifi/README.md)
* 第五讲 使用Harmony OS进行网络编程——使用TCP/IP协议
* 第六讲 使用Harmony OS进行物联网应用开发——使用MQTT、CoAP协议



## 套件简介

HiSpark WiFi IoT套件，整个套件有一个核心板和七个扩展模块：

**购买链接**：https://item.taobao.com/item.htm?id=626113480729

1. 核心板
   * 主控芯片：Hi3861V100
   * 串口芯片：CH340G
2. 交通灯板
   * 红黄绿三个LED
   * 蜂鸣器
   * 按键
3. 炫彩灯板
   * 人体红外检测
   * 光敏电阻
   * 三色LED
4. 环境监测板
   * AHT20温湿度传感器
   * MQ-2燃气传感器
   * 蜂鸣器
5. OLED板
   * SSD1306芯片
   * 0.96吋OLED屏（分辨率：128x64）
6. NFC板
   * FM11NC08I 芯片
7. 机器人扩展板
   * L9110S 电机驱动 * 2



## 许可与版权

* 本课程所有代码使用[**BSD 3-Clause License**](https://opensource.org/licenses/BSD-3-Clause)许可证发布；
* 本课程所有文档使用[**CC BY-NC-SA**](https://creativecommons.org/licenses/by-nc-sa/4.0/)许可证发布，即《知识共享许可证》的“**署名-非商业性使用-相同方式共享**”版本；



## 反馈与改进

* **问题反馈**
  * 如果您发现了本项目中的错误或不当之处，可以在本项目的issues页面进行反馈：https://gitee.com/hihopeorg/HarmonyOS-IoT-Application-Development/issues

* **参与改进**
  * 您也可以通过fork本仓，修改后向本仓发起`Pull Request`请求合并；或者使用码云的轻量级PR，直接在本仓修改，修改完发起 PR；



## 支持我们

本课程配套的 HiSpark WiFi IoT 正在火热销售中，您可以只购买核心板，也可以购买各种套件：

**购买链接**：https://item.taobao.com/item.htm?id=626113480729



## 参考链接

* 鸿蒙OS联接类模组API: https://gitee.com/openharmony/docs/blob/master/api/api-LinkIoT/Readme-CN.md
* 鸿蒙官方文档对核心板和底板的介绍:  https://gitee.com/openharmony/docs/blob/master/quick-start/Hi3861%E5%BC%80%E5%8F%91%E6%9D%BF%E4%BB%8B%E7%BB%8D.md

