#include "drift_detector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#define CJSON_HIDE_SYMBOLS
#include "cJSON.h"

static void ensure_config_dir(void) {
    char path[MAX_PATH_LEN];
    char baselines_path[MAX_PATH_LEN];
    const char *home = getenv("HOME");
    
    if (!home) home = ".";
    
    snprintf(path, sizeof(path), "%s/.config/drift_detector", home);
    mkdir(path, 0755);
    
    snprintf(baselines_path, sizeof(baselines_path), "%s/.config/drift_detector/baselines", home);
    mkdir(baselines_path, 0755);
}

int load_config(Config *config) {
    char config_path[MAX_PATH_LEN];
    const char *home = getenv("HOME");
    FILE *fp;
    char *content = NULL;
    long length;
    cJSON *json, *files, *smtp, *item;
    int i;
    
    if (!home) home = ".";
    
    snprintf(config_path, sizeof(config_path), "%s/%s", home, CONFIG_FILE_PATH);
    
    // Initialize config
    memset(config, 0, sizeof(Config));
    config->smtp_port = 587; // Default SMTP port
    
    fp = fopen(config_path, "r");
    if (!fp) {
        fprintf(stderr, "No config file found, using defaults\n");
        return 0;
    }
    
    fseek(fp, 0, SEEK_END);
    length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    content = malloc(length + 1);
    if (!content) {
        fclose(fp);
        return -1;
    }
    
    size_t bytes_read = fread(content, 1, length, fp);
    content[bytes_read] = '\0';
    fclose(fp);
    
    json = cJSON_Parse(content);
    free(content);
    
    if (!json) {
        fprintf(stderr, "Error parsing config JSON\n");
        return -1;
    }
    
    // Parse files array
    files = cJSON_GetObjectItem(json, "files");
    if (files && cJSON_IsArray(files)) {
        int count = cJSON_GetArraySize(files);
        config->count = (count > MAX_FILES) ? MAX_FILES : count;
        
        for (i = 0; i < config->count; i++) {
            item = cJSON_GetArrayItem(files, i);
            if (cJSON_IsString(item)) {
                strncpy(config->files[i].path, item->valuestring, MAX_PATH_LEN - 1);
                config->files[i].path[MAX_PATH_LEN - 1] = '\0';
                config->files[i].monitoring = true;
                config->files[i].wd = -1;
            }
        }
    }
    
    // Parse SMTP settings
    smtp = cJSON_GetObjectItem(json, "smtp");
    if (smtp) {
        item = cJSON_GetObjectItem(smtp, "server");
        if (item && cJSON_IsString(item)) {
            strncpy(config->smtp_server, item->valuestring, 255);
            config->smtp_server[255] = '\0';
        }
        
        item = cJSON_GetObjectItem(smtp, "port");
        if (item && cJSON_IsNumber(item)) {
            config->smtp_port = item->valueint;
        }
        
        item = cJSON_GetObjectItem(smtp, "user");
        if (item && cJSON_IsString(item)) {
            strncpy(config->smtp_user, item->valuestring, 255);
            config->smtp_user[255] = '\0';
        }
        
        item = cJSON_GetObjectItem(smtp, "password");
        if (item && cJSON_IsString(item)) {
            strncpy(config->smtp_pass, item->valuestring, 255);
            config->smtp_pass[255] = '\0';
        }
        
        item = cJSON_GetObjectItem(smtp, "alert_email");
        if (item && cJSON_IsString(item)) {
            strncpy(config->alert_email, item->valuestring, 255);
            config->alert_email[255] = '\0';
        }
    }
    
    cJSON_Delete(json);
    return 0;
}

int save_config(const Config *config) {
    char config_path[MAX_PATH_LEN];
    const char *home = getenv("HOME");
    FILE *fp;
    cJSON *json, *files, *smtp;
    char *json_str;
    int i;
    
    if (!home) home = ".";
    
    ensure_config_dir();
    
    snprintf(config_path, sizeof(config_path), "%s/%s", home, CONFIG_FILE_PATH);
    
    json = cJSON_CreateObject();
    
    // Add files array
    files = cJSON_CreateArray();
    for (i = 0; i < config->count; i++) {
        cJSON_AddItemToArray(files, cJSON_CreateString(config->files[i].path));
    }
    cJSON_AddItemToObject(json, "files", files);
    
    // Add SMTP settings
    smtp = cJSON_CreateObject();
    cJSON_AddStringToObject(smtp, "server", config->smtp_server);
    cJSON_AddNumberToObject(smtp, "port", config->smtp_port);
    cJSON_AddStringToObject(smtp, "user", config->smtp_user);
    cJSON_AddStringToObject(smtp, "password", config->smtp_pass);
    cJSON_AddStringToObject(smtp, "alert_email", config->alert_email);
    cJSON_AddItemToObject(json, "smtp", smtp);
    
    json_str = cJSON_Print(json);
    
    fp = fopen(config_path, "w");
    if (!fp) {
        fprintf(stderr, "Error opening config file for writing: %s\n", strerror(errno));
        cJSON_Delete(json);
        free(json_str);
        return -1;
    }
    
    fprintf(fp, "%s", json_str);
    fclose(fp);
    
    // Set restrictive permissions to protect sensitive data
    chmod(config_path, 0600);
    
    cJSON_Delete(json);
    free(json_str);
    
    return 0;
}
