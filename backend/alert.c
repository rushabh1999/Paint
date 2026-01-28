#include "drift_detector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

int send_email_alert(const Config *config, const char *filepath, const char *diff) {
    char command[8192];
    char time_str[64];
    char temp_file[256];
    FILE *fp;
    time_t now;
    
    // Check if SMTP settings are configured
    if (strlen(config->smtp_server) == 0 || strlen(config->alert_email) == 0) {
        fprintf(stderr, "SMTP settings not configured, skipping email alert\n");
        return -1;
    }
    
    time(&now);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    // Create temporary file for email body
    snprintf(temp_file, sizeof(temp_file), "/tmp/drift_alert_%ld.txt", (long)now);
    
    fp = fopen(temp_file, "w");
    if (!fp) {
        perror("fopen");
        return -1;
    }
    
    fprintf(fp, "Subject: Configuration Drift Detected - %s\n", filepath);
    fprintf(fp, "From: %s\n", config->smtp_user);
    fprintf(fp, "To: %s\n\n", config->alert_email);
    fprintf(fp, "Configuration drift detected at %s\n\n", time_str);
    fprintf(fp, "File: %s\n\n", filepath);
    fprintf(fp, "Diff:\n");
    fprintf(fp, "========================================\n");
    fprintf(fp, "%s\n", diff);
    fprintf(fp, "========================================\n");
    
    fclose(fp);
    
    // Use Python to send email (more reliable than calling external mail command)
    snprintf(command, sizeof(command),
        "python3 -c \""
        "import smtplib; "
        "from email.mime.text import MIMEText; "
        "msg = MIMEText(open('%s').read()); "
        "msg['Subject'] = 'Configuration Drift Detected - %s'; "
        "msg['From'] = '%s'; "
        "msg['To'] = '%s'; "
        "try: "
        "  s = smtplib.SMTP('%s', %d); "
        "  s.starttls(); "
        "  s.login('%s', '%s'); "
        "  s.send_message(msg); "
        "  s.quit(); "
        "  print('Email sent'); "
        "except Exception as e: "
        "  print('Error:', e); "
        "  exit(1); "
        "\" 2>&1",
        temp_file, filepath, config->smtp_user, config->alert_email,
        config->smtp_server, config->smtp_port, config->smtp_user, config->smtp_pass);
    
    int result = system(command);
    
    // Clean up temp file
    unlink(temp_file);
    
    return (result == 0) ? 0 : -1;
}
