#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include "drift_detector.h"

// Simple test framework
int tests_run = 0;
int tests_passed = 0;

#define TEST(name) \
    printf("Running test: %s\n", name); \
    tests_run++;

#define ASSERT(condition, message) \
    if (!(condition)) { \
        printf("  FAIL: %s\n", message); \
        return 0; \
    } \
    tests_passed++; \
    printf("  PASS: %s\n", message); \
    return 1;

// Test configuration save and load
int test_config_save_load() {
    TEST("Config Save and Load");
    
    Config config1, config2;
    memset(&config1, 0, sizeof(Config));
    
    // Set up test data
    strncpy(config1.smtp_server, "smtp.test.com", 255);
    config1.smtp_port = 587;
    strncpy(config1.smtp_user, "test@test.com", 255);
    strncpy(config1.smtp_pass, "testpass", 255);
    strncpy(config1.alert_email, "alert@test.com", 255);
    
    config1.count = 2;
    strncpy(config1.files[0].path, "/tmp/test1.conf", MAX_PATH_LEN - 1);
    strncpy(config1.files[1].path, "/tmp/test2.conf", MAX_PATH_LEN - 1);
    
    // Save configuration
    if (save_config(&config1) != 0) {
        printf("  FAIL: Could not save config\n");
        return 0;
    }
    
    // Load configuration
    memset(&config2, 0, sizeof(Config));
    if (load_config(&config2) != 0) {
        printf("  FAIL: Could not load config\n");
        return 0;
    }
    
    // Verify
    ASSERT(strcmp(config2.smtp_server, "smtp.test.com") == 0, "SMTP server matches");
    ASSERT(config2.smtp_port == 587, "SMTP port matches");
    ASSERT(config2.count == 2, "File count matches");
    ASSERT(strcmp(config2.files[0].path, "/tmp/test1.conf") == 0, "First file path matches");
    
    return 1;
}

// Test baseline creation
int test_baseline_creation() {
    TEST("Baseline Creation");
    
    const char *test_file = "/tmp/test_config_baseline.txt";
    FILE *fp;
    
    // Create a test file
    fp = fopen(test_file, "w");
    if (!fp) {
        printf("  FAIL: Could not create test file\n");
        return 0;
    }
    fprintf(fp, "test configuration\n");
    fclose(fp);
    
    // Create baseline
    if (create_baseline(test_file) != 0) {
        printf("  FAIL: Could not create baseline\n");
        unlink(test_file);
        return 0;
    }
    
    // Verify baseline exists
    char baseline_path[MAX_PATH_LEN];
    const char *home = getenv("HOME");
    if (!home) home = ".";
    snprintf(baseline_path, sizeof(baseline_path), 
             "%s/%s/test_config_baseline.txt", home, BASELINE_DIR);
    
    if (access(baseline_path, F_OK) != 0) {
        printf("  FAIL: Baseline file does not exist\n");
        unlink(test_file);
        return 0;
    }
    
    // Cleanup
    unlink(test_file);
    unlink(baseline_path);
    
    tests_passed++;
    printf("  PASS: Baseline created successfully\n");
    return 1;
}

// Test drift detection
int test_drift_detection() {
    TEST("Drift Detection");
    
    const char *test_file = "/tmp/test_drift_detect.txt";
    char diff_output[8192];
    FILE *fp;
    
    // Create a test file
    fp = fopen(test_file, "w");
    if (!fp) {
        printf("  FAIL: Could not create test file\n");
        return 0;
    }
    fprintf(fp, "original content\n");
    fclose(fp);
    
    // Create baseline
    if (create_baseline(test_file) != 0) {
        printf("  FAIL: Could not create baseline\n");
        unlink(test_file);
        return 0;
    }
    
    // Modify the file
    fp = fopen(test_file, "w");
    if (!fp) {
        printf("  FAIL: Could not modify test file\n");
        unlink(test_file);
        return 0;
    }
    fprintf(fp, "modified content\n");
    fclose(fp);
    
    // Detect drift
    int result = detect_drift(test_file, diff_output, sizeof(diff_output));
    
    if (result != 0) {
        printf("  FAIL: Drift should be detected\n");
        return 0;
    }
    
    // Verify diff contains expected content
    if (strstr(diff_output, "-original") == NULL) {
        printf("  FAIL: Diff output doesn't contain expected content\n");
        return 0;
    }
    
    // Cleanup
    char baseline_path[MAX_PATH_LEN];
    const char *home = getenv("HOME");
    if (!home) home = ".";
    snprintf(baseline_path, sizeof(baseline_path), 
             "%s/%s/test_drift_detect.txt", home, BASELINE_DIR);
    unlink(test_file);
    unlink(baseline_path);
    
    tests_passed++;
    printf("  PASS: Drift detected successfully\n");
    return 1;
}

// Test no drift when file unchanged
int test_no_drift() {
    TEST("No Drift Detection");
    
    const char *test_file = "/tmp/test_no_drift.txt";
    char diff_output[8192];
    FILE *fp;
    
    // Create a test file
    fp = fopen(test_file, "w");
    if (!fp) {
        printf("  FAIL: Could not create test file\n");
        return 0;
    }
    fprintf(fp, "unchanged content\n");
    fclose(fp);
    
    // Create baseline
    if (create_baseline(test_file) != 0) {
        printf("  FAIL: Could not create baseline\n");
        unlink(test_file);
        return 0;
    }
    
    // Detect drift (should be none)
    int result = detect_drift(test_file, diff_output, sizeof(diff_output));
    
    if (result == 0) {
        printf("  FAIL: No drift should be detected\n");
        return 0;
    }
    
    // Cleanup
    char baseline_path[MAX_PATH_LEN];
    const char *home = getenv("HOME");
    if (!home) home = ".";
    snprintf(baseline_path, sizeof(baseline_path), 
             "%s/%s/test_no_drift.txt", home, BASELINE_DIR);
    unlink(test_file);
    unlink(baseline_path);
    
    tests_passed++;
    printf("  PASS: No drift correctly detected\n");
    return 1;
}

int main() {
    printf("=== Running Backend Tests ===\n\n");
    
    // Run tests
    test_config_save_load();
    test_baseline_creation();
    test_drift_detection();
    test_no_drift();
    
    // Print summary
    printf("\n=== Test Summary ===\n");
    printf("Tests Run: %d\n", tests_run);
    printf("Tests Passed: %d\n", tests_passed);
    printf("Tests Failed: %d\n", tests_run - tests_passed);
    
    if (tests_passed == tests_run) {
        printf("\nAll tests PASSED!\n");
        return 0;
    } else {
        printf("\nSome tests FAILED!\n");
        return 1;
    }
}
