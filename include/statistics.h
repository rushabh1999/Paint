#ifndef STATISTICS_H
#define STATISTICS_H

#include <time.h>

typedef struct {
    double uptime_percentage;
    double downtime_percentage;
    long total_checks;
    long successful_checks;
    long failed_checks;
    long total_outages;
    long total_downtime_sec;
    long longest_outage_sec;
    long shortest_outage_sec;
    double avg_outage_sec;
} DailyStatistics;

bool generate_daily_summary(const char* log_file, DailyStatistics* stats);
bool save_daily_summary(const char* summary_file, const DailyStatistics* stats);

#endif
