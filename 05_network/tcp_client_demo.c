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

#include "net_params.h"
#include "wifi_connecter.h"

#include "lwip/sockets.h"


static char request[] = "Hello";
static char response[128] = "";

static ssize_t TcpClientTest(void)
{
    ssize_t retval = 0;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket

    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PARAM_SERVER_PORT);
    if (inet_pton(AF_INET, PARAM_SERVER_ADDR, &serverAddr.sin_addr) <= 0) {
        printf("inet_pton failed!\r\n");
        goto do_cleanup;
    }

    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("connect failed!\r\n");
        goto do_cleanup;
    }
    printf("connect to server %s success!\r\n", PARAM_SERVER_ADDR);

    retval = send(sockfd, request, sizeof(request), 0);
    if (retval < 0) {
        printf("lwip_send request failed!\r\n");
        goto do_cleanup;
    }
    printf("send request{%s} to server done!\r\n", request);

    retval = recv(sockfd, &response, sizeof(response), 0);
    if (retval <= 0) {
        printf("lwip_recv response from server failed or done, %d!\r\n", retval);
        goto do_cleanup;
    }
    response[retval] = '\0';
    printf("recv response{%s} %d from server done!\r\n", response, retval);

do_cleanup:
    printf("do_cleanup...\r\n");
    lwip_close(sockfd);
    return retval;
}

static void TcpClientTask(void *arg)
{
    (void)arg;
    WifiDeviceConfig config = {0};

    // 准备AP的配置参数
    strcpy(config.ssid, PARAM_HOTSPOT_SSID);
    strcpy(config.preSharedKey, PARAM_HOTSPOT_PSK);
    config.securityType = PARAM_HOTSPOT_TYPE;

    osDelay(10);

    int netId = ConnectToHotspot(&config);

    int timeout = 10;
    while (timeout--) {
        printf("After %d seconds, I will start TCP client test!\r\n", timeout);
        osDelay(100);
    }

    TcpClientTest();

    printf("disconnect to AP ...\r\n");
    DisconnectWithHotspot(netId);
    printf("disconnect to AP done!\r\n");
}

static void TcpClientDemo(void)
{
    osThreadAttr_t attr;

    attr.name = "TcpClientTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 10240;
    attr.priority = osPriorityNormal;

    if (osThreadNew(TcpClientTask, NULL, &attr) == NULL) {
        printf("[TcpClientDemo] Falied to create TcpClientTask!\n");
    }
}

SYS_RUN(TcpClientDemo);

