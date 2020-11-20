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
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifi_device.h"

#include "lwip/netifapi.h"
#include "lwip/api_shell.h"

static void PrintLinkedInfo(WifiLinkedInfo* info)
{
    if (!info) return;

    static char macAddress[32] = {0};
    unsigned char* mac = info->bssid;
    snprintf(macAddress, sizeof(macAddress), "%02X:%02X:%02X:%02X:%02X:%02X",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    printf("bssid: %s, rssi: %d, connState: %d, reason: %d, ssid: %s\r\n",
        macAddress, info->rssi, info->connState, info->disconnectedReason, info->ssid);
}

static int g_connected = 0;
static void OnWifiConnectionChanged(int state, WifiLinkedInfo* info)
{
    if (!info) return;

    printf("%s %d, state = %d, info = \r\n", __FUNCTION__, __LINE__, state);
    PrintLinkedInfo(info);

    if (state == WIFI_STATE_AVALIABLE) {
        g_connected = 1;
    } else {
        g_connected = 0;
    }
}

static void OnWifiScanStateChanged(int state, int size)
{
    printf("%s %d, state = %X, size = %d\r\n", __FUNCTION__, __LINE__, state, size);
}

static void WifiConnectTask(void *arg)
{
    (void)arg;
    WifiErrorCode errCode;
    WifiEvent eventListener = {
        .OnWifiConnectionChanged = OnWifiConnectionChanged,
        .OnWifiScanStateChanged = OnWifiScanStateChanged
    };
    WifiDeviceConfig apConfig = {};
    int netId = -1;

    osDelay(10);
    errCode = RegisterWifiEvent(&eventListener);
    printf("RegisterWifiEvent: %d\r\n", errCode);

    // setup your AP params
    strcpy(apConfig.ssid, "ABCD");
    strcpy(apConfig.preSharedKey, "12345678");
    apConfig.securityType = WIFI_SEC_TYPE_PSK;

    while (1) {
        errCode = EnableWifi();
        printf("EnableWifi: %d\r\n", errCode);
        osDelay(10);

        errCode = AddDeviceConfig(&apConfig, &netId);
        printf("AddDeviceConfig: %d\r\n", errCode);

        g_connected = 0;
        errCode = ConnectTo(netId);
        printf("ConnectTo(%d): %d\r\n", netId, errCode);

        while (!g_connected) {
            osDelay(10);
        }
        printf("g_connected: %d\r\n", g_connected);
        osDelay(50);

        // 联网业务开始
        struct netif* iface = netifapi_netif_find("wlan0");
        if (iface) {
            err_t ret = netifapi_dhcp_start(iface);
            printf("netifapi_dhcp_start: %d\r\n", ret);

            osDelay(200); // wait DHCP server give me IP
            ret = netifapi_netif_common(iface, dhcp_clients_info_show, NULL);
            printf("netifapi_netif_common: %d\r\n", ret);
        }

        // 模拟一段时间的联网业务
        int timeout = 60;
        while (timeout--) {
            osDelay(100);
            printf("after %d seconds, I'll disconnect WiFi!\n", timeout);
        }

        // 联网业务结束
        err_t ret = netifapi_dhcp_stop(iface);
        printf("netifapi_dhcp_stop: %d\r\n", ret);

        Disconnect(); // disconnect with your AP

        RemoveDevice(netId); // remove AP config

        errCode = DisableWifi();
        printf("DisableWifi: %d\r\n", errCode);
        osDelay(200);
    }
}

static void WifiConnectDemo(void)
{
    osThreadAttr_t attr;

    attr.name = "WifiConnectTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 10240;
    attr.priority = osPriorityNormal;

    if (osThreadNew(WifiConnectTask, NULL, &attr) == NULL) {
        printf("[WifiConnectDemo] Falied to create WifiConnectTask!\n");
    }
}

APP_FEATURE_INIT(WifiConnectDemo);
