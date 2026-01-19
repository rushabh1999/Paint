#include "ping_checker.h"
#include <windows.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <stdio.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_HOST "8.8.8.8"
#define TIMEOUT_MS 5000

void init_ping_checker(void) {
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);
}

void cleanup_ping_checker(void) {
    WSACleanup();
}

PingResult check_connectivity(const char* host) {
    PingResult result = {false, -1};
    
    if (host == NULL) {
        host = DEFAULT_HOST;
    }
    
    HANDLE icmp_file = IcmpCreateFile();
    if (icmp_file == INVALID_HANDLE_VALUE) {
        return result;
    }
    
    unsigned long ip_addr = inet_addr(host);
    if (ip_addr == INADDR_NONE) {
        IcmpCloseHandle(icmp_file);
        return result;
    }
    
    char send_data[32] = "Internet Uptime Tracker Ping";
    DWORD reply_size = sizeof(ICMP_ECHO_REPLY) + sizeof(send_data);
    void* reply_buffer = malloc(reply_size);
    
    if (reply_buffer == NULL) {
        IcmpCloseHandle(icmp_file);
        return result;
    }
    
    DWORD start_time = GetTickCount();
    DWORD reply_count = IcmpSendEcho(
        icmp_file,
        ip_addr,
        send_data,
        sizeof(send_data),
        NULL,
        reply_buffer,
        reply_size,
        TIMEOUT_MS
    );
    DWORD end_time = GetTickCount();
    
    if (reply_count > 0) {
        PICMP_ECHO_REPLY echo_reply = (PICMP_ECHO_REPLY)reply_buffer;
        if (echo_reply->Status == IP_SUCCESS) {
            result.is_connected = true;
            result.latency_ms = echo_reply->RoundTripTime;
            if (result.latency_ms == 0) {
                result.latency_ms = end_time - start_time;
            }
        }
    }
    
    free(reply_buffer);
    IcmpCloseHandle(icmp_file);
    
    return result;
}
