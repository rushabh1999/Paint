#include "drift_detector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>

int send_email_alert(const Config *config, const char *filepath, const char *diff) {
    char temp_file[256];
    char time_str[64];
    FILE *fp;
    time_t now;
    int fd;
    
    // Check if SMTP settings are configured
    if (strlen(config->smtp_server) == 0 || strlen(config->alert_email) == 0) {
        fprintf(stderr, "SMTP settings not configured, skipping email alert\n");
        return -1;
    }
    
    time(&now);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    // Create secure temporary file
    snprintf(temp_file, sizeof(temp_file), "/tmp/drift_alert_XXXXXX");
    fd = mkstemp(temp_file);
    if (fd < 0) {
        perror("mkstemp");
        return -1;
    }
    
    fp = fdopen(fd, "w");
    if (!fp) {
        perror("fdopen");
        close(fd);
        unlink(temp_file);
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
    
    // Write Python script to a separate secure temporary file
    char script_file[256];
    snprintf(script_file, sizeof(script_file), "/tmp/send_email_XXXXXX");
    fd = mkstemp(script_file);
    if (fd < 0) {
        perror("mkstemp");
        unlink(temp_file);
        return -1;
    }
    
    fp = fdopen(fd, "w");
    if (!fp) {
        perror("fdopen");
        close(fd);
        unlink(temp_file);
        unlink(script_file);
        return -1;
    }
    
    // Write Python script with properly escaped values
    fprintf(fp, "#!/usr/bin/env python3\n");
    fprintf(fp, "import smtplib\n");
    fprintf(fp, "from email.mime.text import MIMEText\n");
    fprintf(fp, "import sys\n\n");
    fprintf(fp, "try:\n");
    fprintf(fp, "    with open('%s', 'r') as f:\n", temp_file);
    fprintf(fp, "        msg_text = f.read()\n");
    fprintf(fp, "    msg = MIMEText(msg_text)\n");
    fprintf(fp, "    msg['Subject'] = 'Configuration Drift Detected'\n");
    fprintf(fp, "    msg['From'] = '%s'\n", config->smtp_user);
    fprintf(fp, "    msg['To'] = '%s'\n", config->alert_email);
    fprintf(fp, "    s = smtplib.SMTP('%s', %d)\n", config->smtp_server, config->smtp_port);
    fprintf(fp, "    s.starttls()\n");
    fprintf(fp, "    s.login('%s', '%s')\n", config->smtp_user, config->smtp_pass);
    fprintf(fp, "    s.send_message(msg)\n");
    fprintf(fp, "    s.quit()\n");
    fprintf(fp, "    print('Email sent')\n");
    fprintf(fp, "except Exception as e:\n");
    fprintf(fp, "    print('Error:', e, file=sys.stderr)\n");
    fprintf(fp, "    sys.exit(1)\n");
    
    fclose(fp);
    
    // Make script executable and run it
    chmod(script_file, 0700);
    char command[512];
    snprintf(command, sizeof(command), "python3 %s 2>&1", script_file);
    int result = system(command);
    
    // Clean up temp files
    unlink(temp_file);
    unlink(script_file);
    
    return (result == 0) ? 0 : -1;
}
