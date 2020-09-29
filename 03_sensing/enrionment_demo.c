/*
 * Copyright (c) 2020, HiHope Community.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_i2c.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "wifiiot_adc.h"
#include "wifiiot_errno.h"
#include "oled_ssd1306.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) sizeof(a)/sizeof(a[0])
#endif

#define MS_PER_S 1000
#define BEEP_DURATION 100
#define BEEP_PWM_DUTY 30000
#define BEEP_PWM_FREQ 60000
#define BEEP_TIMES 3
#define BEEP_PIN_NAME WIFI_IOT_IO_NAME_GPIO_9
#define BEEP_PIN_FUNCTION WIFI_IOT_IO_FUNC_GPIO_9_PWM0_OUT

#define GAS_SENSOR_CHAN_NAME WIFI_IOT_ADC_CHANNEL_5
// #define GAS_SENSOR_PIN_NAME WIFI_IOT_IO_NAME_GPIO_11
#define ADC_RESOLUTION 2048

#define AHT20_I2C_IDX WIFI_IOT_I2C_IDX_0

#define AHT20_STARTUP_TIME 20*1000 // 上电启动时间
#define AHT20_CALIBRATION_TIME 40*1000 // 初始化（校准）时间
#define AHT20_MEASURE_TIME    75*1000 // 测量时间

#define AHT20_DEVICE_ADDR   0x38
#define AHT20_READ_ADDR     ((0x38<<1)|0x1)
#define AHT20_WRITE_ADDR    ((0x38<<1)|0x0)

#define AHT20_CMD_CALIBRATION       0xBE // 初始化（校准）命令
#define AHT20_CMD_CALIBRATION_ARG0  0x08
#define AHT20_CMD_CALIBRATION_ARG1  0x00

/**
 * 传感器在采集时需要时间,主机发出测量指令（0xAC）后，延时75毫秒以上再读取转换后的数据并判断返回的状态位是否正常。
 * 若状态比特位[Bit7]为0代表数据可正常读取，为1时传感器为忙状态，主机需要等待数据处理完成。
 **/
#define AHT20_CMD_TRIGGER       0xAC // 触发测量命令
#define AHT20_CMD_TRIGGER_ARG0  0x33
#define AHT20_CMD_TRIGGER_ARG1  0x00

// 用于在无需关闭和再次打开电源的情况下，重新启动传感器系统，软复位所需时间不超过20 毫秒
#define AHT20_CMD_RESET      0xBA // 软复位命令

#define AHT20_CMD_STATUS     0x71 // 获取状态命令

/**
 * STATUS 命令回复：
 * 1. 初始化后触发测量之前，STATUS 只回复 1B 状态值；
 * 2. 触发测量之后，STATUS 回复6B： 1B 状态值 + 2B 湿度 + 4b湿度 + 4b温度 + 2B 温度
 *      RH = Srh / 2^20 * 100%
 *      T  = St  / 2^20 * 200 - 50
 **/
#define AHT20_STATUS_BUSY_SHIFT 7       // bit[7] Busy indication
#define AHT20_STATUS_BUSY_MASK  (0x1<<AHT20_STATUS_BUSY_SHIFT)
#define AHT20_STATUS_BUSY(status) ((status & AHT20_STATUS_BUSY_MASK) >> AHT20_STATUS_BUSY_SHIFT)

#define AHT20_STATUS_MODE_SHIFT 5       // bit[6:5] Mode Status
#define AHT20_STATUS_MODE_MASK  (0x3<<AHT20_STATUS_MODE_SHIFT)
#define AHT20_STATUS_MODE(status) ((status & AHT20_STATUS_MODE_MASK) >> AHT20_STATUS_MODE_SHIFT)

                                        // bit[4] Reserved
#define AHT20_STATUS_CALI_SHIFT 3       // bit[3] CAL Enable
#define AHT20_STATUS_CALI_MASK  (0x1<<AHT20_STATUS_CALI_SHIFT)
#define AHT20_STATUS_CALI(status) ((status & AHT20_STATUS_CALI_MASK) >> AHT20_STATUS_CALI_SHIFT)
                                        // bit[2:0] Reserved

#define AHT20_STATUS_RESPONSE_MAX 6

#define AHT20_BAUDRATE             400*1000
#define AHT20_RESLUTION            (1<<20)  // 2^20

static uint32_t AHT20_Read(uint8_t* buffer, size_t buffLen)
{
    WifiIotI2cData data = { 0 };
    data.receiveBuf = buffer;
    data.receiveLen = buffLen;
    uint32_t retval = I2cRead(AHT20_I2C_IDX, AHT20_READ_ADDR, &data);
    if (retval != WIFI_IOT_SUCCESS) {
        printf("I2cRead() failed, %0X!\n", retval);
        return retval;
    }
    return WIFI_IOT_SUCCESS;
}

static uint32_t AHT20_Write(uint8_t* buffer, size_t buffLen)
{
    WifiIotI2cData data = { 0 };
    data.sendBuf = buffer;
    data.sendLen = buffLen;
    uint32_t retval = I2cWrite(AHT20_I2C_IDX, AHT20_WRITE_ADDR, &data);
    if (retval != WIFI_IOT_SUCCESS) {
        printf("I2cWrite(%02X) failed, %0X!\n", buffer[0], retval);
        return retval;
    }
    return WIFI_IOT_SUCCESS;
}

// 发送获取状态命令
static uint32_t AHT20_StatusCommand(void)
{
    uint8_t statusCmd[] = { AHT20_CMD_STATUS };
    return AHT20_Write(statusCmd, sizeof(statusCmd));
}

// 发送软复位命令
static uint32_t AHT20_ResetCommand(void)
{
    uint8_t resetCmd[] = {AHT20_CMD_RESET};
    return AHT20_Write(resetCmd, sizeof(resetCmd));
}

// 发送初始化校准命令
static uint32_t AHT20_CalibrateCommand(void)
{
    uint8_t clibrateCmd[] = {AHT20_CMD_CALIBRATION, AHT20_CMD_CALIBRATION_ARG0, AHT20_CMD_CALIBRATION_ARG1};
    return AHT20_Write(clibrateCmd, sizeof(clibrateCmd));
}

// 读取温湿度值之前， 首先要看状态字的校准使能位Bit[3]是否为 1(通过发送0x71可以获取一个字节的状态字)，
// 如果不为1，要发送0xBE命令(初始化)，此命令参数有两个字节， 第一个字节为0x08，第二个字节为0x00。
static uint32_t AHT20_Initialize(void)
{
    uint32_t retval = 0;
    uint8_t buffer[AHT20_STATUS_RESPONSE_MAX] = { AHT20_CMD_STATUS };
    memset(&buffer, 0x0, sizeof(buffer));

    retval = AHT20_StatusCommand();
    if (retval != WIFI_IOT_SUCCESS) {
        return retval;
    }

    retval = AHT20_Read(buffer, sizeof(buffer));
    if (retval != WIFI_IOT_SUCCESS) {
        return retval;
    }

    if (AHT20_STATUS_BUSY(buffer[0]) || !AHT20_STATUS_CALI(buffer[0])) {
        retval = AHT20_ResetCommand();
        if (retval != WIFI_IOT_SUCCESS) {
            return retval;
        }
        usleep(AHT20_STARTUP_TIME);
        retval = AHT20_CalibrateCommand();
        usleep(AHT20_CALIBRATION_TIME);
        return retval;
    }

    return WIFI_IOT_SUCCESS;
}

// 发送 触发测量 命令，开始测量
uint32_t AHT20_StartMeasure(void)
{
    uint8_t triggerCmd[] = {AHT20_CMD_TRIGGER, AHT20_CMD_TRIGGER_ARG0, AHT20_CMD_TRIGGER_ARG1};
    return AHT20_Write(triggerCmd, sizeof(triggerCmd));
}

// 接收测量结果，拼接转换为标准值
uint32_t AHT20_ReadMeasureResult(float* temp, float* humi)
{
    uint32_t retval = 0;
    if (temp == NULL || humi == NULL) {
        return WIFI_IOT_FAILURE;
    }

    uint8_t buffer[AHT20_STATUS_RESPONSE_MAX] = { 0 };
    memset(&buffer, 0x0, sizeof(buffer));
    retval = AHT20_Read(buffer, sizeof(buffer));  // recv status command result
    if (retval != WIFI_IOT_SUCCESS) {
        return retval;
    }

    for (int i = 0; AHT20_STATUS_BUSY(buffer[0]) && i < 10; i++) {
        printf("AHT20 device busy, retry %d!\r\n", i);
        usleep(AHT20_MEASURE_TIME);
        retval = AHT20_Read(buffer, sizeof(buffer));  // recv status command result
        if (retval != WIFI_IOT_SUCCESS) {
            return retval;
        }
    }

    uint32_t humiRaw = buffer[1];
    humiRaw = (humiRaw << 8) | buffer[2];
    humiRaw = (humiRaw << 4) | ((buffer[3] & 0xF0) >> 4);
    *humi = humiRaw / (float)AHT20_RESLUTION * 100;

    uint32_t tempRaw = buffer[3] & 0x0F;
    tempRaw = (tempRaw << 8) | buffer[4];
    tempRaw = (tempRaw << 8) | buffer[5];
    *temp = tempRaw / (float)AHT20_RESLUTION * 200 - 50;
    // printf("humi: %05X, %f, %05X, %\r\n", humiRaw, *humi, tempRaw, *temp);
    return WIFI_IOT_SUCCESS;
}

static float ConvertToVoltage(unsigned short data)
{
    return (float)data * 1.8 * 4 / 4096;
}

static void EnvironmentTask(void *arg)
{
    (void)arg;
    uint32_t retval = 0;
    float humidity = 0.0f;
    float temperature = 0.0f;
    float gasSensorResistance = 0.0f;
    static char line[32] = {0};

    OledInit();
    OledFillScreen(0);
    I2cInit(AHT20_I2C_IDX, AHT20_BAUDRATE);

    // set BEEP pin as PWM function
    IoSetFunc(BEEP_PIN_NAME, BEEP_PIN_FUNCTION);
    GpioSetDir(BEEP_PIN_NAME, WIFI_IOT_GPIO_DIR_OUT);
    PwmInit(WIFI_IOT_PWM_PORT_PWM0);

    for (int i = 0; i < BEEP_TIMES; i++) {
        snprintf(line, sizeof(line), "beep %d/%d", (i+1), BEEP_TIMES);
        OledShowString(0, 0, line, 1);

        PwmStart(WIFI_IOT_PWM_PORT_PWM0, BEEP_PWM_DUTY, BEEP_PWM_FREQ);
        usleep(BEEP_DURATION * 1000);
        PwmStop(WIFI_IOT_PWM_PORT_PWM0);
        usleep((1000 - BEEP_DURATION) * 1000);
    }

    while (WIFI_IOT_SUCCESS != AHT20_Initialize()) {
        printf("AHT20 sensor init failed!\r\n");
        usleep(1000);
    }

    while(1) {
        retval = AHT20_StartMeasure();
        if (retval != WIFI_IOT_SUCCESS) {
            printf("trigger measure failed!\r\n");
        }

        usleep(AHT20_MEASURE_TIME);
        retval = AHT20_ReadMeasureResult(&temperature, &humidity);
        if (retval != WIFI_IOT_SUCCESS) {
            printf("get humidity data failed!\r\n");
        }

        unsigned short data = 0;
        if (AdcRead(GAS_SENSOR_CHAN_NAME, &data, WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0)
                == WIFI_IOT_SUCCESS) {
            float Vx = ConvertToVoltage(data);

            // Vcc            ADC            GND
            //  |     ______   |     ______   |
            //  +---| MG-2 |---+---| 1kom |---+
            //       ------         ------
            // 查阅原理图，ADC 引脚位于 1K 电阻和燃气传感器之间，燃气传感器另一端接在 5V 电源正极上
            // 串联电路电压和阻止成正比：
            // Vx / 5 == 1kom / (1kom + Rx)
            //   => Rx + 1 == 5/Vx
            //   =>  Rx = 5/Vx - 1
            gasSensorResistance = 5 / Vx - 1;
        }

        OledShowString(0, 0, "Sensor values:", 1);

        snprintf(line, sizeof(line), "temp: %.2f", temperature);
        OledShowString(0, 1, line, 1);

        snprintf(line, sizeof(line), "humi: %.2f", humidity);
        OledShowString(0, 2, line, 1);

        snprintf(line, sizeof(line), "gas: %.2f kom", gasSensorResistance);
        OledShowString(0, 3, line, 1);

        sleep(2);
    }
}

static void EnvironmentDemo(void)
{
    osThreadAttr_t attr;

    GpioInit();

    IoSetFunc(BEEP_PIN_NAME, BEEP_PIN_FUNCTION);
    GpioSetDir(BEEP_PIN_NAME, WIFI_IOT_GPIO_DIR_OUT);
    PwmInit(WIFI_IOT_PWM_PORT_PWM0);

    attr.name = "EnvironmentTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    if (osThreadNew(EnvironmentTask, NULL, &attr) == NULL) {
        printf("[EnvironmentDemo] Falied to create EnvironmentTask!\n");
    }
}

APP_FEATURE_INIT(EnvironmentDemo);