# Harmony OS物联网应用开发实战 第二讲

本节课程中向大家演示如何使用Hamony OS控制外设，具体包括：

* 官方第一个应用程序示例演示
  * 熟悉使用DevEco Device Tool插件进行程序烧录
  * 熟悉串口调试工具sscom的使用

* 官方的控制核心板上LED的演示
	* GPIO_9 连接LED灯，输出低电平点亮LED灯
	* GPIO_5 连接按键，按键中断控制LED灯状态反转
* HiSpark Wi-Fi IoT 开发套件 交通灯板 演示
	* GPIO_8 连接蜂鸣器，输出PWM波控制蜂鸣器发出声音
	* GPIO_9 连接按键，按键中断切换亮的LED灯，同时切换蜂鸣器响或者不响
	* GPIO_10 连接红色LED，输出高电平点亮红色LED灯
	* GPIO_11 连接绿色LED，输出高电平点亮绿色LED灯
	* GPIO_12 连接黄色LED，输出高电平点亮黄色LED灯

## 如何编译

1. 将此目录下的 `traffic_light_demo.c` 和 `BUILD.gn` 复制到openharmony源码的`applications\sample\wifi-iot\app\iothardware`路径下，
2. 修改openharmony源码的`applications\sample\wifi-iot\app\BUILD.gn`文件，将其中的 `features` 改为：
```md
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



