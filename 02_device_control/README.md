# 使用Harmony OS控制外设——输入输出

**本节课程主要介绍如何在HiSpark WiFi IoT套件上使用Hamony OS进行编程，以及如何使用GPIO输入输出功能。**



## 相关知识点

*  Hi3861开发板第一个示例程序演示
  * 熟悉使用DevEco Device Tool插件进行程序烧录
  * 熟悉串口调试工具sscom的使用
  
* 官方文档中控制核心板上LED的`led_example.c`讲解及演示
	* 源码路径：`applications/sample/wifi-iot/app/iothardware/led_example.c`
	* Harmony OS IoT内核（liteos-m）接口——[CMSIS-RTOS API v2](https://arm-software.github.io/CMSIS_5/RTOS2/html/rtos_api2.html)
	  * 线程创建`osThreadNew`
	* 鸿蒙系统启动注册接口——`ohos_init.h`
	  * `SYS_RUN`注册启动阶段自动运行的函数
	  * 不同宏对应不同阶段，应用代码推荐使用：
	    * `APP_FEATURE_INIT`注册宏
	* 使用GPIO接口输出高低电平，实现控制LED灯亮灭
	  * 设置管脚功能`IoSetFunc`
	  * 设置GPIO引脚方向`GpioSetDir`
	* 使用GPIO接口注册中断处理函数，实现USER按键响应（输入）
	  * 设置内部上拉下拉状态`IoSetPull`（没有外部上拉电阻时需要设置上拉，才能保证能够检测到下降沿）
	  * 注册中断处理函数`GpioRegisterIsrFunc`，具体参数使用参考头文件说明
	* 具体使用参考`wifiiot_gpio.h`和`wifiiot_gpio_ex.h`文件里的注释

* HiSpark Wi-Fi IoT 开发套件 交通灯板 控制
	* 使用PWM接口输出PWM方波，实现蜂鸣器控制
	* 使用GPIO接口的部分和上一个示例类似
	* 具体使用参考`wifiiot_pwm.h`文件里的注释



## 核心板和交通灯板上外设与主控芯片（Hi3861）引脚的对应关系

* 核心板
  * GPIO_9 连接LED灯，输出低电平点亮LED灯
  * GPIO_5 连接按键，按键中断控制LED灯状态反转
* 交通灯板
  * GPIO_8 连接蜂鸣器，输出PWM波控制蜂鸣器发出声音
  * GPIO_9 连接按键，按键中断切换亮的LED灯，同时切换蜂鸣器响或者不响
  * GPIO_10 连接红色LED，输出高电平点亮红色LED灯
  * GPIO_11 连接绿色LED，输出高电平点亮绿色LED灯
  * GPIO_12 连接黄色LED，输出高电平点亮黄色LED灯



## 如何编译

1. 将此目录下的 `traffic_light_demo.c` 和 `BUILD.gn` 复制到openharmony源码的`applications\sample\wifi-iot\app\iothardware`目录下，
2. 修改openharmony源码的`applications\sample\wifi-iot\app\BUILD.gn`文件，将其中的 `features` 改为：
```python
    features = [
        "iothardware:traffic_light_demo",
    ]
```
3. 在openharmony源码顶层目录执行：`python build.py wifiiot`

### 报错解决

1. 编译过程中报错：undefined reference to `hi_pwm_init` 等几个 `hi_pwm_`开头的函数，
	* **原因：** 因为默认情况下，hi3861_sdk中，PWM的CONFIG选项没有打开
	* **解决：** 修改`vendor\hisi\hi3861\hi3861\build\config\usr_config.mk`文件中的`CONFIG_PWM_SUPPORT`行：
	  * `# CONFIG_PWM_SUPPORT is not set`修改为`CONFIG_PWM_SUPPORT=y`

