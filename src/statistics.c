#include "statistics.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

bool generate_daily_summary(const char* log_file, DailyStatistics* stats) {
    if (log_file == NULL || stats == NULL) return false;
    
    // Initialize statistics
    memset(stats, 0, sizeof(DailyStatistics));
    stats->shortest_outage_sec = LONG_MAX;
    
    FILE* fp = fopen(log_file, "r");
    if (fp == NULL) return false;
    
    char line[512];
    bool in_outage = false;
    long current_outage_start = 0;
    
    // Skip header
    if (fgets(line, sizeof(line), fp) == NULL) {
        fclose(fp);
        return false;
    }
    
    while (fgets(line, sizeof(line), fp) != NULL) {
        char timestamp[64];
        char status[16];
        long latency;
        long outage_duration;
        
        if (sscanf(line, "%[^,],%[^,],%ld,%ld", 
                   timestamp, status, &latency, &outage_duration) != 4) {
            continue;
        }
        
        stats->total_checks++;
        
        if (strcmp(status, "UP") == 0) {
            stats->successful_checks++;
            
            // If we were in an outage, record it
            if (in_outage && outage_duration > 0) {
                stats->total_outages++;
                stats->total_downtime_sec += outage_duration;
                
                if (outage_duration > stats->longest_outage_sec) {
                    stats->longest_outage_sec = outage_duration;
                }
                if (outage_duration < stats->shortest_outage_sec) {
                    stats->shortest_outage_sec = outage_duration;
                }
                in_outage = false;
            }
        } else if (strcmp(status, "DOWN") == 0) {
            stats->failed_checks++;
            in_outage = true;
        }
    }
    
    fclose(fp);
    
    // Calculate percentages and averages
    if (stats->total_checks > 0) {
        stats->uptime_percentage = (double)stats->successful_checks / stats->total_checks * 100.0;
        stats->downtime_percentage = (double)stats->failed_checks / stats->total_checks * 100.0;
    }
    
    if (stats->total_outages > 0) {
        stats->avg_outage_sec = (double)stats->total_downtime_sec / stats->total_outages;
    }
    
    if (stats->shortest_outage_sec == LONG_MAX) {
        stats->shortest_outage_sec = 0;
    }
    
    return true;
}

bool save_daily_summary(const char* summary_file, const DailyStatistics* stats) {
    if (summary_file == NULL || stats == NULL) return false;
    
    FILE* fp = fopen(summary_file, "w");
    if (fp == NULL) return false;
    
    fprintf(fp, "=== Daily Internet Uptime Summary ===\n\n");
    fprintf(fp, "Total Checks: %ld\n", stats->total_checks);
    fprintf(fp, "Successful Checks: %ld\n", stats->successful_checks);
    fprintf(fp, "Failed Checks: %ld\n", stats->failed_checks);
    fprintf(fp, "Uptime Percentage: %.2f%%\n", stats->uptime_percentage);
    fprintf(fp, "Downtime Percentage: %.2f%%\n", stats->downtime_percentage);
    fprintf(fp, "\n=== Outage Statistics ===\n");
    fprintf(fp, "Number of Outages: %ld\n", stats->total_outages);
    fprintf(fp, "Total Downtime: %ld seconds (%.2f minutes)\n", 
            stats->total_downtime_sec, stats->total_downtime_sec / 60.0);
    
    if (stats->total_outages > 0) {
        fprintf(fp, "Average Outage Duration: %.2f seconds (%.2f minutes)\n", 
                stats->avg_outage_sec, stats->avg_outage_sec / 60.0);
        fprintf(fp, "Longest Outage: %ld seconds (%.2f minutes)\n", 
                stats->longest_outage_sec, stats->longest_outage_sec / 60.0);
        fprintf(fp, "Shortest Outage: %ld seconds (%.2f minutes)\n", 
                stats->shortest_outage_sec, stats->shortest_outage_sec / 60.0);
    }
    
    fclose(fp);
    return true;
}
