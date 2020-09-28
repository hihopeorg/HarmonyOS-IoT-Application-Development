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

#define AHT_I2C_IDX WIFI_IOT_I2C_IDX_0

#define AHT_SLEEP_20MS              (20)//20ms
#define AHT_SLEEP_50MS              (50)//5ms
#define AHT_SLEEP_1S                (1000)//1s
#define AHT_DELAY_10MS              (10000)//10ms
#define AHT_DELAY_40MS              (40000) //40ms
#define AHT_DELAY_100MS             (100000) //100ms
#define AHT_DEVICE_ADDR             (0x38) //device addr
#define AHT_DEVICE_READ_STATUS      (0x71) //befor read tem&humi data need to send cmd to comfir the              
#define AHT_DEVICE_INIT_CMD         (0xBE) //aht init cmd
#define AHT_DEVICE_TEST_CMD         (0xAC) // test cmd
#define AHT_DEVICE_PARAM_HIGH_BYTE  (0x33)
#define AHT_DEVICE_PARAM_LOW_BYTE   (0x00)
#define AHT_DEVICE_PARAM_INIT_HIGH  (0x08)
#define AHT_DEVICE_CALIBRATION      (0x80)
#define AHT_DEVICE_CALIBRATION_ERR  (0x1C)
#define AHT_DEVICE_CALIBRATION_ERR_R (0x18)
#define AHT_TASK_SLEEP_TIME         (20) //thread sleep 20ms
#define BAUDRATE_INIT               (400000) 


typedef enum{
    AHT_TEMPERATURE =1,
    AHT_HUMIDITY    =2,
} aht_serson_type;

#define  AHT_REG_ARRAY_LEN          (6)
#define  AHT_OC_ARRAY_LEN           (6)
#define  AHT_SNED_CMD_LEN           (3)
#define  AHT20_DEMO_TASK_STAK_SIZE  (1024*4)
#define  AHT20_DEMO_TASK_PRIORITY   (25)
#define  AHT_REG_ARRAY_INIT_LEN     (1)
#define  AHT_CALCULATION            (1048576)

float aht_temp = 0;
float aht_humi = 0;

/*
*Check whether the bit3 of the temperature and humidity sensor is initialized successfully, 
*otherwise send the setting of 0xbe to set the sensor initialization
*/
static uint32_t ath20_check_and_init(uint8_t initCmd, uint8_t initHighByte, uint8_t initLowByte)
{
    uint32_t status = 0;
    WifiIotI2cIdx id = AHT_I2C_IDX;
    WifiIotI2cData aht20WriteData ={0};
    WifiIotI2cData aht20ReadData = { 0 };
    uint8_t recvData[AHT_REG_ARRAY_INIT_LEN] = { 0 };
    uint8_t sendData[AHT_SNED_CMD_LEN] = {initCmd, initHighByte, initLowByte};

    memset(&recvData, 0x0, sizeof(recvData));
    memset(&aht20ReadData, 0x0, sizeof(WifiIotI2cData));

    aht20ReadData.receiveBuf = recvData;
    aht20ReadData.receiveLen = AHT_REG_ARRAY_INIT_LEN;
    aht20WriteData.sendBuf = sendData;
    aht20WriteData.sendLen = AHT_SNED_CMD_LEN;

    status = I2cRead(id, (AHT_DEVICE_ADDR<<1)|0x01, &aht20ReadData);
    if (status != WIFI_IOT_SUCCESS) return status;
    // printf("recvData[0] =0x%x\r\n", recvData[0]);
    if (((recvData[0] != AHT_DEVICE_CALIBRATION_ERR) && (recvData[0] != AHT_DEVICE_CALIBRATION_ERR_R)) || (recvData[0] == AHT_DEVICE_CALIBRATION)) {
        // printf("sensor need to calibration\r\n");
        status = I2cWrite(id, (AHT_DEVICE_ADDR<<1)|0x00, &aht20WriteData);
        sleep(1);
        if (status != WIFI_IOT_SUCCESS) {
            return status;
        }
    }

    return WIFI_IOT_SUCCESS;
}

/*发送触犯测量命令*/
uint32_t aht20_write(uint8_t trigger_cmd, uint8_t high_byte_cmd, uint8_t low_byte_cmd)
{
    WifiIotI2cIdx id = AHT_I2C_IDX;
    WifiIotI2cData aht20WriteData ={0};
    uint8_t sendData[AHT_SNED_CMD_LEN] = {trigger_cmd, high_byte_cmd, low_byte_cmd};

    aht20WriteData.sendBuf = sendData;
    aht20WriteData.sendLen = AHT_SNED_CMD_LEN;

    return I2cWrite(id, (AHT_DEVICE_ADDR<<1)|0x00, &aht20WriteData);
}

/*读取 aht20 serson 数据*/
uint32_t aht20_read(uint32_t recv_len, uint8_t type)
{
    uint32_t status = 0;
    WifiIotI2cIdx id  = AHT_I2C_IDX;
    uint8_t recvData[AHT_REG_ARRAY_LEN] = { 0 };
    WifiIotI2cData aht20ReadData = { 0 };
    float temper =0;
    float humi =0;
    /* Request memory space */
    memset(&recvData, 0x0, sizeof(recvData));
    memset(&aht20ReadData, 0x0, sizeof(WifiIotI2cData));
    aht20ReadData.receiveBuf = recvData;
    aht20ReadData.receiveLen = recv_len;
    
    status = I2cRead(id, (AHT_DEVICE_ADDR<<1)|0x01, &aht20ReadData);
    if (status != WIFI_IOT_SUCCESS) return status;
    if (type == AHT_TEMPERATURE) {
        temper = (float)((recvData[3] &0x0f)<<16 | recvData[4]<<8 |recvData[5]);//温度拼接
        aht_temp = (temper/AHT_CALCULATION)*200-50;  // T= (S_t/2^20)*200-50
        return WIFI_IOT_SUCCESS; 
    } 
    if (type == AHT_HUMIDITY) {
        humi = (float)((recvData[1]<<12 | recvData[2]<<4) | ((recvData[3] & 0xf0)>>4));//湿度拼接
        aht_humi = humi/AHT_CALCULATION*100;
        return WIFI_IOT_SUCCESS;
    }
    return WIFI_IOT_SUCCESS;
}

static void EnvironmentTask(void *arg)
{
    (void)arg;
    uint32_t status = 0;
    static char line[32] = {0};

    OledInit();
    OledFillScreen(0);

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

    /*上电等待40ms*/
    usleep(AHT_DELAY_40MS);//40ms
    while(1) {
        /*check whethe the sensor  calibration*/
        while (WIFI_IOT_SUCCESS != ath20_check_and_init(AHT_DEVICE_INIT_CMD, AHT_DEVICE_PARAM_INIT_HIGH, AHT_DEVICE_PARAM_LOW_BYTE)) {
            // printf("aht20 sensor check init failed!\r\n");
            usleep(50 * 1000);
        }
        /* on hold master mode*/
        status = aht20_write(AHT_DEVICE_TEST_CMD, AHT_DEVICE_PARAM_HIGH_BYTE, AHT_DEVICE_PARAM_LOW_BYTE);//tempwerature
        usleep(AHT_DELAY_100MS);//100ms等待测量完成
        status = aht20_read(AHT_REG_ARRAY_LEN, AHT_TEMPERATURE);
        status = aht20_read(AHT_REG_ARRAY_LEN, AHT_HUMIDITY);
        if (status != WIFI_IOT_SUCCESS) {
            printf("get humidity data error!\r\n");
        }

        unsigned short data = 0;
        if (AdcRead(GAS_SENSOR_CHAN_NAME, &data, WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0)
                == WIFI_IOT_SUCCESS) {
            // ((float)data) / ADC_RESOLUTION;
        }

        OledShowString(0, 0, "Sensor values:", 1);

        snprintf(line, sizeof(line), "temp: %.2f", aht_temp);
        OledShowString(0, 1, line, 1);

        snprintf(line, sizeof(line), "humi: %.2f", aht_humi);
        OledShowString(0, 2, line, 1);

        snprintf(line, sizeof(line), "gas: %u", (unsigned)data);
        OledShowString(0, 3, line, 1);

        usleep(20 * 1000);
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