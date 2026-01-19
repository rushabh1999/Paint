#ifndef SERVICE_H
#define SERVICE_H

#include <windows.h>

#define SERVICE_NAME "InternetUptimeTracker"
#define SERVICE_DISPLAY_NAME "Internet Uptime Tracker Service"
#define SERVICE_DESCRIPTION "Monitors internet connectivity and logs uptime/downtime statistics"

void WINAPI service_main(DWORD argc, LPTSTR *argv);
void WINAPI service_ctrl_handler(DWORD ctrl_code);
bool install_service(void);
bool uninstall_service(void);
bool start_service_as_console(void);

#endif
