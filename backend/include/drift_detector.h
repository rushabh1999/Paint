#ifndef DRIFT_DETECTOR_H
#define DRIFT_DETECTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <openssl/evp.h>

#define CONFIG_DIR_PATH ".config/drift_detector"
#define HASH_ALGORITHM "SHA256"
#define HASH_SIZE 32  // SHA-256 produces 32 bytes
#define HASH_HEX_SIZE 65  // 32 bytes * 2 + null terminator

// Function declarations
int init_config_dir(void);
char* get_config_path(void);
char* get_baseline_path(const char* filename);
char* get_hash_path(const char* filename);
int calculate_file_hash(const char* filepath, unsigned char* hash_output);
int hash_to_hex_string(const unsigned char* hash, char* hex_string, size_t hex_size);
int store_baseline(const char* filepath);
int store_hash(const char* filepath, const unsigned char* hash);
int verify_hash(const char* filepath, char* result_message, size_t message_size);
int detect_drift(const char* filepath, char* result_message, size_t message_size);

#endif // DRIFT_DETECTOR_H
