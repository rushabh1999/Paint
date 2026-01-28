#include "../include/drift_detector.h"
#include <errno.h>
#include <libgen.h>
#include <limits.h>

/**
 * Initialize the configuration directory
 */
int init_config_dir(void) {
    char* config_path = get_config_path();
    if (!config_path) {
        return -1;
    }
    
    struct stat st = {0};
    if (stat(config_path, &st) == -1) {
        if (mkdir(config_path, 0700) != 0) {
            perror("Failed to create config directory");
            free(config_path);
            return -1;
        }
    }
    
    free(config_path);
    return 0;
}

/**
 * Get the configuration directory path
 */
char* get_config_path(void) {
    const char* home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "HOME environment variable not set\n");
        return NULL;
    }
    
    char* config_path = malloc(PATH_MAX);
    if (!config_path) {
        perror("Memory allocation failed");
        return NULL;
    }
    
    snprintf(config_path, PATH_MAX, "%s/%s", home, CONFIG_DIR_PATH);
    return config_path;
}

/**
 * Get the baseline file path for a given filename
 */
char* get_baseline_path(const char* filename) {
    char* config_path = get_config_path();
    if (!config_path) {
        return NULL;
    }
    
    // Extract just the filename from the path
    char* filename_copy = strdup(filename);
    char* base = basename(filename_copy);
    
    char* baseline_path = malloc(PATH_MAX);
    if (!baseline_path) {
        free(config_path);
        free(filename_copy);
        return NULL;
    }
    
    snprintf(baseline_path, PATH_MAX, "%s/%s.baseline", config_path, base);
    
    free(config_path);
    free(filename_copy);
    return baseline_path;
}

/**
 * Get the hash file path for a given filename
 */
char* get_hash_path(const char* filename) {
    char* config_path = get_config_path();
    if (!config_path) {
        return NULL;
    }
    
    // Extract just the filename from the path
    char* filename_copy = strdup(filename);
    char* base = basename(filename_copy);
    
    char* hash_path = malloc(PATH_MAX);
    if (!hash_path) {
        free(config_path);
        free(filename_copy);
        return NULL;
    }
    
    snprintf(hash_path, PATH_MAX, "%s/%s.hash", config_path, base);
    
    free(config_path);
    free(filename_copy);
    return hash_path;
}

/**
 * Calculate SHA-256 hash of a file
 */
int calculate_file_hash(const char* filepath, unsigned char* hash_output) {
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        perror("Failed to open file for hashing");
        return -1;
    }
    
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        fclose(file);
        fprintf(stderr, "Failed to create EVP context\n");
        return -1;
    }
    
    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1) {
        EVP_MD_CTX_free(mdctx);
        fclose(file);
        fprintf(stderr, "Failed to initialize digest\n");
        return -1;
    }
    
    unsigned char buffer[8192];
    size_t bytes_read;
    
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (EVP_DigestUpdate(mdctx, buffer, bytes_read) != 1) {
            EVP_MD_CTX_free(mdctx);
            fclose(file);
            fprintf(stderr, "Failed to update digest\n");
            return -1;
        }
    }
    
    unsigned int hash_len;
    if (EVP_DigestFinal_ex(mdctx, hash_output, &hash_len) != 1) {
        EVP_MD_CTX_free(mdctx);
        fclose(file);
        fprintf(stderr, "Failed to finalize digest\n");
        return -1;
    }
    
    EVP_MD_CTX_free(mdctx);
    fclose(file);
    
    return 0;
}

/**
 * Convert hash bytes to hexadecimal string
 */
int hash_to_hex_string(const unsigned char* hash, char* hex_string, size_t hex_size) {
    if (hex_size < HASH_HEX_SIZE) {
        return -1;
    }
    
    for (int i = 0; i < HASH_SIZE; i++) {
        sprintf(hex_string + (i * 2), "%02x", hash[i]);
    }
    hex_string[HASH_SIZE * 2] = '\0';
    
    return 0;
}

/**
 * Store baseline copy of a file
 */
int store_baseline(const char* filepath) {
    if (init_config_dir() != 0) {
        return -1;
    }
    
    char* baseline_path = get_baseline_path(filepath);
    if (!baseline_path) {
        return -1;
    }
    
    // Copy file to baseline
    FILE* src = fopen(filepath, "rb");
    if (!src) {
        perror("Failed to open source file");
        free(baseline_path);
        return -1;
    }
    
    FILE* dst = fopen(baseline_path, "wb");
    if (!dst) {
        perror("Failed to create baseline file");
        fclose(src);
        free(baseline_path);
        return -1;
    }
    
    char buffer[8192];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        if (fwrite(buffer, 1, bytes, dst) != bytes) {
            perror("Failed to write to baseline file");
            fclose(src);
            fclose(dst);
            free(baseline_path);
            return -1;
        }
    }
    
    fclose(src);
    fclose(dst);
    
    // Calculate and store hash
    unsigned char hash[HASH_SIZE];
    if (calculate_file_hash(filepath, hash) != 0) {
        free(baseline_path);
        return -1;
    }
    
    if (store_hash(filepath, hash) != 0) {
        free(baseline_path);
        return -1;
    }
    
    free(baseline_path);
    return 0;
}

/**
 * Store hash of a file
 */
int store_hash(const char* filepath, const unsigned char* hash) {
    char* hash_path = get_hash_path(filepath);
    if (!hash_path) {
        return -1;
    }
    
    FILE* hash_file = fopen(hash_path, "w");
    if (!hash_file) {
        perror("Failed to create hash file");
        free(hash_path);
        return -1;
    }
    
    char hex_hash[HASH_HEX_SIZE];
    if (hash_to_hex_string(hash, hex_hash, HASH_HEX_SIZE) != 0) {
        fclose(hash_file);
        free(hash_path);
        return -1;
    }
    
    fprintf(hash_file, "%s\n", hex_hash);
    fclose(hash_file);
    free(hash_path);
    
    return 0;
}

/**
 * Verify hash of a file against stored hash
 */
int verify_hash(const char* filepath, char* result_message, size_t message_size) {
    // Calculate current hash
    unsigned char current_hash[HASH_SIZE];
    if (calculate_file_hash(filepath, current_hash) != 0) {
        snprintf(result_message, message_size, "ERROR: Failed to calculate hash for %s", filepath);
        return -1;
    }
    
    char current_hex[HASH_HEX_SIZE];
    hash_to_hex_string(current_hash, current_hex, HASH_HEX_SIZE);
    
    // Read stored hash
    char* hash_path = get_hash_path(filepath);
    if (!hash_path) {
        snprintf(result_message, message_size, "ERROR: Failed to get hash path for %s", filepath);
        return -1;
    }
    
    FILE* hash_file = fopen(hash_path, "r");
    if (!hash_file) {
        snprintf(result_message, message_size, "ERROR: No stored hash found for %s. Run baseline command first.", filepath);
        free(hash_path);
        return -1;
    }
    
    char stored_hex[HASH_HEX_SIZE];
    if (fgets(stored_hex, HASH_HEX_SIZE, hash_file) == NULL) {
        snprintf(result_message, message_size, "ERROR: Failed to read stored hash for %s", filepath);
        fclose(hash_file);
        free(hash_path);
        return -1;
    }
    
    fclose(hash_file);
    free(hash_path);
    
    // Remove newline if present
    stored_hex[strcspn(stored_hex, "\n")] = 0;
    
    // Compare hashes
    if (strcmp(current_hex, stored_hex) == 0) {
        snprintf(result_message, message_size, "OK: Hash verification passed for %s", filepath);
        return 0;
    } else {
        snprintf(result_message, message_size, 
                "ALERT: Hash mismatch detected for %s - Potential integrity breach!\nStored: %s\nCurrent: %s", 
                filepath, stored_hex, current_hex);
        return 1;
    }
}

/**
 * Detect drift by comparing file with baseline and verifying hash
 */
int detect_drift(const char* filepath, char* result_message, size_t message_size) {
    char* baseline_path = get_baseline_path(filepath);
    if (!baseline_path) {
        snprintf(result_message, message_size, "ERROR: Failed to get baseline path for %s", filepath);
        return -1;
    }
    
    // Check if baseline exists
    struct stat st;
    if (stat(baseline_path, &st) != 0) {
        snprintf(result_message, message_size, "ERROR: No baseline found for %s. Run baseline command first.", filepath);
        free(baseline_path);
        return -1;
    }
    
    // First verify hash
    char hash_message[1024];
    int hash_result = verify_hash(filepath, hash_message, sizeof(hash_message));
    
    if (hash_result == 1) {
        // Hash mismatch detected
        snprintf(result_message, message_size, "%s", hash_message);
        free(baseline_path);
        return 1;
    } else if (hash_result == -1) {
        // Error during hash verification
        snprintf(result_message, message_size, "%s", hash_message);
        free(baseline_path);
        return -1;
    }
    
    // Compare with baseline
    FILE* current_file = fopen(filepath, "rb");
    if (!current_file) {
        snprintf(result_message, message_size, "ERROR: Failed to open %s", filepath);
        free(baseline_path);
        return -1;
    }
    
    FILE* baseline_file = fopen(baseline_path, "rb");
    if (!baseline_file) {
        snprintf(result_message, message_size, "ERROR: Failed to open baseline for %s", filepath);
        fclose(current_file);
        free(baseline_path);
        return -1;
    }
    
    char current_buffer[8192];
    char baseline_buffer[8192];
    size_t current_bytes, baseline_bytes;
    int files_match = 1;
    
    while (1) {
        current_bytes = fread(current_buffer, 1, sizeof(current_buffer), current_file);
        baseline_bytes = fread(baseline_buffer, 1, sizeof(baseline_buffer), baseline_file);
        
        if (current_bytes != baseline_bytes || memcmp(current_buffer, baseline_buffer, current_bytes) != 0) {
            files_match = 0;
            break;
        }
        
        if (current_bytes == 0) {
            break;
        }
    }
    
    fclose(current_file);
    fclose(baseline_file);
    free(baseline_path);
    
    if (files_match) {
        snprintf(result_message, message_size, "OK: No drift detected for %s (hash verified)", filepath);
        return 0;
    } else {
        snprintf(result_message, message_size, "DRIFT: File %s has drifted from baseline", filepath);
        return 1;
    }
}
