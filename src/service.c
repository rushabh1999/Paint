#include "service.h"
#include "ping_checker.h"
#include "state_machine.h"
#include "logger.h"
#include "statistics.h"
#include <stdio.h>
#include <stdlib.h>

#define CHECK_INTERVAL_MS 60000  // 60 seconds
#define PING_HOST "8.8.8.8"

static SERVICE_STATUS service_status;
static SERVICE_STATUS_HANDLE service_status_handle;
static HANDLE stop_event = NULL;
static bool running_as_console = false;

static void report_service_status(DWORD current_state, DWORD exit_code, DWORD wait_hint) {
    static DWORD checkpoint = 1;
    
    service_status.dwCurrentState = current_state;
    service_status.dwWin32ExitCode = exit_code;
    service_status.dwWaitHint = wait_hint;
    
    if (current_state == SERVICE_START_PENDING) {
        service_status.dwControlsAccepted = 0;
    } else {
        service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    }
    
    if ((current_state == SERVICE_RUNNING) || (current_state == SERVICE_STOPPED)) {
        service_status.dwCheckPoint = 0;
    } else {
        service_status.dwCheckPoint = checkpoint++;
    }
    
    if (!running_as_console) {
        SetServiceStatus(service_status_handle, &service_status);
    }
}

void WINAPI service_ctrl_handler(DWORD ctrl_code) {
    switch (ctrl_code) {
        case SERVICE_CONTROL_STOP:
            report_service_status(SERVICE_STOP_PENDING, NO_ERROR, 0);
            SetEvent(stop_event);
            return;
        
        case SERVICE_CONTROL_INTERROGATE:
            break;
        
        default:
            break;
    }
}

static void run_tracking_loop(void) {
    char log_path[MAX_PATH];
    GetModuleFileNameA(NULL, log_path, MAX_PATH);
    char* last_slash = strrchr(log_path, '\\');
    if (last_slash) {
        *last_slash = '\0';
    }
    strcat(log_path, "\\logs");
    
    init_ping_checker();
    
    if (!init_logger(log_path)) {
        report_service_status(SERVICE_STOPPED, ERROR_SERVICE_SPECIFIC_ERROR, 0);
        return;
    }
    
    StateMachine state_machine;
    init_state_machine(&state_machine);
    
    int last_summary_day = -1;  // Track last day we generated a summary
    
    report_service_status(SERVICE_RUNNING, NO_ERROR, 0);
    
    while (WaitForSingleObject(stop_event, CHECK_INTERVAL_MS) == WAIT_TIMEOUT) {
        PingResult result = check_connectivity(PING_HOST);
        
        update_state(&state_machine, result.is_connected);
        
        LogEntry entry;
        entry.timestamp = time(NULL);
        entry.is_connected = result.is_connected;
        entry.latency_ms = result.latency_ms;
        entry.outage_duration_sec = 0;
        
        // If we just recovered from an outage, log the duration
        if (state_machine.outage_detected) {
            entry.outage_duration_sec = get_outage_duration(&state_machine);
            state_machine.outage_detected = false;
        }
        
        log_entry(&entry);
        
        // Generate daily summary at midnight (once per day)
        time_t now = time(NULL);
        struct tm* tm_info = localtime(&now);
        int current_day = tm_info->tm_yday;  // Day of year (0-365)
        
        if (tm_info->tm_hour == 0 && tm_info->tm_min == 0 && current_day != last_summary_day) {
            char summary_path[MAX_PATH];
            char date_str[32];
            strftime(date_str, sizeof(date_str), "%Y-%m-%d", tm_info);
            snprintf(summary_path, sizeof(summary_path), "%s\\summary_%s.txt", log_path, date_str);
            
            DailyStatistics stats;
            if (generate_daily_summary(get_current_log_file(), &stats)) {
                save_daily_summary(summary_path, &stats);
                last_summary_day = current_day;
            }
        }
    }
    
    close_logger();
    cleanup_ping_checker();
    report_service_status(SERVICE_STOPPED, NO_ERROR, 0);
}

void WINAPI service_main(DWORD argc, LPTSTR *argv) {
    service_status_handle = RegisterServiceCtrlHandlerA(SERVICE_NAME, service_ctrl_handler);
    
    if (!service_status_handle) {
        return;
    }
    
    service_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    service_status.dwServiceSpecificExitCode = 0;
    
    report_service_status(SERVICE_START_PENDING, NO_ERROR, 3000);
    
    stop_event = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (stop_event == NULL) {
        report_service_status(SERVICE_STOPPED, GetLastError(), 0);
        return;
    }
    
    run_tracking_loop();
}

bool install_service(void) {
    SC_HANDLE sc_manager = OpenSCManagerA(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (sc_manager == NULL) {
        return false;
    }
    
    char path[MAX_PATH];
    if (!GetModuleFileNameA(NULL, path, MAX_PATH)) {
        CloseServiceHandle(sc_manager);
        return false;
    }
    
    SC_HANDLE service = CreateServiceA(
        sc_manager,
        SERVICE_NAME,
        SERVICE_DISPLAY_NAME,
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        path,
        NULL, NULL, NULL, NULL, NULL
    );
    
    if (service == NULL) {
        CloseServiceHandle(sc_manager);
        return false;
    }
    
    SERVICE_DESCRIPTIONA description;
    description.lpDescription = SERVICE_DESCRIPTION;
    ChangeServiceConfig2A(service, SERVICE_CONFIG_DESCRIPTION, &description);
    
    CloseServiceHandle(service);
    CloseServiceHandle(sc_manager);
    return true;
}

bool uninstall_service(void) {
    SC_HANDLE sc_manager = OpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
    if (sc_manager == NULL) {
        return false;
    }
    
    SC_HANDLE service = OpenServiceA(sc_manager, SERVICE_NAME, SERVICE_STOP | DELETE);
    if (service == NULL) {
        CloseServiceHandle(sc_manager);
        return false;
    }
    
    SERVICE_STATUS status;
    ControlService(service, SERVICE_CONTROL_STOP, &status);
    
    bool result = DeleteService(service);
    
    CloseServiceHandle(service);
    CloseServiceHandle(sc_manager);
    return result;
}

bool start_service_as_console(void) {
    running_as_console = true;
    
    printf("Starting Internet Uptime Tracker in console mode...\n");
    printf("Press Ctrl+C to stop\n\n");
    
    stop_event = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (stop_event == NULL) {
        return false;
    }
    
    SetConsoleCtrlHandler(NULL, TRUE);
    
    run_tracking_loop();
    
    CloseHandle(stop_event);
    return true;
}
