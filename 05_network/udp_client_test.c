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
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "net_demo.h"
#include "net_common.h"

static char request[] = "Hello.";
static char response[128] = "";

void UdpClientTest(const char* host, unsigned short port)
{
    ssize_t retval = 0;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0); // UDP socket

    struct sockaddr_in toAddr = {0};
    toAddr.sin_family = AF_INET;
    toAddr.sin_port = htons(port); // 端口号，从主机字节序转为网络字节序
    if (inet_pton(AF_INET, host, &toAddr.sin_addr) <= 0) { // 将主机IP地址从“点分十进制”字符串 转化为 标准格式（32位整数）
        printf("inet_pton failed!\r\n");
        goto do_cleanup;
    }

    // UDP socket 是 “无连接的” ，因此每次发送都必须先指定目标主机和端口，主机可以是多播地址
    retval = sendto(sockfd, request, sizeof(request), 0, (struct sockaddr *)&toAddr, sizeof(toAddr));
    if (retval < 0) {
        printf("sendto failed!\r\n");
        goto do_cleanup;
    }
    printf("send UDP message {%s} %ld done!\r\n", request, retval);

    struct sockaddr_in fromAddr = {0};
    socklen_t fromLen = sizeof(fromAddr);

    // UDP socket 是 “无连接的” ，因此每次接收时前并不知道消息来自何处，通过 fromAddr 参数可以得到发送方的信息（主机、端口号）
    retval = recvfrom(sockfd, &response, sizeof(response), 0, (struct sockaddr *)&fromAddr, &fromLen);
    if (retval <= 0) {
        printf("recvfrom failed or abort, %ld, %d!\r\n", retval, errno);
        goto do_cleanup;
    }
    response[retval] = '\0';
    printf("recv UDP message {%s} %ld done!\r\n", response, retval);
    printf("peer info: ipaddr = %s, port = %d\r\n", inet_ntoa(fromAddr.sin_addr), ntohs(fromAddr.sin_port));

do_cleanup:
    printf("do_cleanup...\r\n");
    close(sockfd);
}

CLIENT_TEST_DEMO(UdpClientTest);
