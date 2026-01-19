#include "service.h"
#include <stdio.h>
#include <string.h>

void print_usage(const char* program_name) {
    printf("Internet Uptime Tracker Service\n\n");
    printf("Usage: %s [command]\n\n", program_name);
    printf("Commands:\n");
    printf("  install     Install the service\n");
    printf("  uninstall   Uninstall the service\n");
    printf("  console     Run in console mode (for testing)\n");
    printf("  (no args)   Run as Windows service\n\n");
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "install") == 0) {
            if (install_service()) {
                printf("Service installed successfully.\n");
                printf("Use 'sc start %s' to start the service.\n", SERVICE_NAME);
                return 0;
            } else {
                printf("Failed to install service. Error: %lu\n", GetLastError());
                printf("Make sure to run as Administrator.\n");
                return 1;
            }
        } else if (strcmp(argv[1], "uninstall") == 0) {
            if (uninstall_service()) {
                printf("Service uninstalled successfully.\n");
                return 0;
            } else {
                printf("Failed to uninstall service. Error: %lu\n", GetLastError());
                printf("Make sure to run as Administrator.\n");
                return 1;
            }
        } else if (strcmp(argv[1], "console") == 0) {
            return start_service_as_console() ? 0 : 1;
        } else {
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // Run as Windows service
    SERVICE_TABLE_ENTRYA service_table[] = {
        { SERVICE_NAME, (LPSERVICE_MAIN_FUNCTIONA)service_main },
        { NULL, NULL }
    };
    
    if (!StartServiceCtrlDispatcherA(service_table)) {
        DWORD error = GetLastError();
        if (error == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT) {
            printf("This program is designed to run as a Windows service.\n");
            printf("Use '%s console' to run in console mode, or '%s install' to install as a service.\n", 
                   argv[0], argv[0]);
            return 1;
        }
        return 1;
    }
    
    return 0;
}
