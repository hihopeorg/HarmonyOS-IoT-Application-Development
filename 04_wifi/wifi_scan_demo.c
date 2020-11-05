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

static char* SecurityTypeName(WifiSecurityType type)
{
    switch (type)
    {
    case WIFI_SEC_TYPE_OPEN:
        return "OPEN";
    case WIFI_SEC_TYPE_WEP:
        return "WEP";
    case WIFI_SEC_TYPE_PSK:
        return "PSK";
    case WIFI_SEC_TYPE_SAE:
        return "SAE";
    default:
        break;
    }
    return "unkow";
}

void OnWifiConnectionChanged(int state, WifiLinkedInfo* info)
{
    (void) state;
    (void) info;
    printf("%s %d\r\n", __FUNCTION__, __LINE__);
}

void PrintScanResult(void)
{
    WifiScanInfo scanResult[WIFI_SCAN_HOTSPOT_LIMIT] = {0};
    uint32_t resultSize = WIFI_SCAN_HOTSPOT_LIMIT;

    memset(&scanResult, 0, sizeof(scanResult));
    WifiErrorCode errCode = GetScanInfoList(scanResult, &resultSize);
    if (errCode != WIFI_SUCCESS) {
        printf("GetScanInfoList failed: %d\r\n", errCode);
    }
    for (uint32_t i = 0; i < resultSize; i++) {
        static char macAddress[32] = {0};
        WifiScanInfo info = scanResult[i];
        unsigned char* mac = info.bssid;
        snprintf(macAddress, sizeof(macAddress), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        printf("Result[%d]: %s, % 4s, %d, %d, %d, %s\r\n", i, macAddress, SecurityTypeName(info.securityType), info.rssi, info.band, info.frequency, info.ssid);
    }
}

static int g_scanDone = 0;
void OnWifiScanStateChanged(int state, int size)
{
    printf("%s %d, state = %X, size = %d\r\n", __FUNCTION__, __LINE__, state, size);

    if (state == WIFI_STATE_AVALIABLE && size > 0) {
        g_scanDone = 1;
    }
}

static void WifiScanTask(void *arg)
{
    (void)arg;
    WifiErrorCode errCode;
    WifiEvent eventListener = {
        .OnWifiConnectionChanged = OnWifiConnectionChanged,
        .OnWifiScanStateChanged = OnWifiScanStateChanged
    };

    osDelay(10);
    errCode = RegisterWifiEvent(&eventListener);
    printf("RegisterWifiEvent: %d\r\n", errCode);

    while (1) {
        errCode = EnableWifi();
        printf("EnableWifi: %d\r\n", errCode);
        osDelay(100);

        g_scanDone = 0;
        errCode = Scan();
        printf("Scan: %d\r\n", errCode);

        // wait for scan done!
        while (!g_scanDone) {
            osDelay(5);
        }
        PrintScanResult();

        errCode = DisableWifi();
        printf("DisableWifi: %d\r\n", errCode);
        osDelay(500);
    }
}

static void WifiScanDemo(void)
{
    osThreadAttr_t attr;

    attr.name = "WifiScanTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 10240;
    attr.priority = osPriorityNormal;

    if (osThreadNew(WifiScanTask, NULL, &attr) == NULL) {
        printf("[WifiScanDemo] Falied to create WifiScanTask!\n");
    }
}

APP_FEATURE_INIT(WifiScanDemo);

