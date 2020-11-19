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
#ifndef NET_COMMON_H
#define NET_COMMON_H

// __arm__ and __aarch64__ for HarmonyOS with liteos-a kernel, __i386__ and __x86_64__ for Unix like OS
#if defined(__arm__) || defined(__aarch64__) || defined(__i386__) || defined(__x86_64__)
#define HAVE_BSD_SOCKET 1
#else
#define HAVE_BSD_SOCKET 0
#endif

#if defined(__riscv) // for wifiiot(HarmonyOS on Hi3861 with liteos-m kernel)
#define HAVE_LWIP_SOCKET 1
#else
#define HAVE_LWIP_SOCKET 0
#endif

#if HAVE_BSD_SOCKET
#include <sys/types.h>  // for AF_INET SOCK_STREAM
#include <sys/socket.h> // for socket
#include <netinet/in.h> // for sockaddr_in
#include <arpa/inet.h> // for inet_pton
#elif HAVE_LWIP_SOCKET
#include "lwip/sockets.h"
#ifndef close
#define close(fd) lwip_close(fd)
#endif
#else
#error "Unknow platform!"
#endif

#endif  // NET_COMMON_H