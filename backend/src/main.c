#include "../include/drift_detector.h"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <command> <filepath>\n", argv[0]);
        fprintf(stderr, "Commands:\n");
        fprintf(stderr, "  baseline <file>  - Store baseline and hash\n");
        fprintf(stderr, "  verify <file>    - Verify file hash\n");
        fprintf(stderr, "  check <file>     - Check for drift\n");
        return 1;
    }
    
    const char* command = argv[1];
    const char* filepath = argv[2];
    char message[2048];
    int result;
    
    if (strcmp(command, "baseline") == 0) {
        printf("Storing baseline for %s...\n", filepath);
        result = store_baseline(filepath);
        if (result == 0) {
            printf("SUCCESS: Baseline and hash stored for %s\n", filepath);
            return 0;
        } else {
            printf("ERROR: Failed to store baseline for %s\n", filepath);
            return 1;
        }
    } else if (strcmp(command, "verify") == 0) {
        result = verify_hash(filepath, message, sizeof(message));
        printf("%s\n", message);
        return (result == 0) ? 0 : 1;
    } else if (strcmp(command, "check") == 0) {
        result = detect_drift(filepath, message, sizeof(message));
        printf("%s\n", message);
        return (result == 0) ? 0 : 1;
    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
        return 1;
    }
    
    return 0;
}
