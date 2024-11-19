#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>

#define INPUT_DIR "input"
#define OUTPUT_DIR "output"
#define MODULES_COUNT 3
const char *MODULES[] = {"text", "image", "video"};

void create_output_folder(char *output_folder) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(output_folder, 100, OUTPUT_DIR "/%Y%m%d_%H%M%S", t);
    mkdir(output_folder, 0755);
}

void process_file(const char *file_name, const char *output_folder) {
    char command[512];

    for (int i = 0; i < MODULES_COUNT; i++) {
        snprintf(command, sizeof(command),
                 "python3 modules/%s.py %s/%s %s",
                 MODULES[i], INPUT_DIR, file_name, output_folder);
        printf("Running: %s\n", command);
        int ret = system(command);
        if (ret != 0) {
            fprintf(stderr, "Error in %s module for file %s\n", MODULES[i], file_name);
        }
    }
}

int main() {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(INPUT_DIR);
    if (dir == NULL) {
        perror("Input directory not found");
        return 1;
    }

    char output_folder[100];
    create_output_folder(output_folder);
    printf("Output folder created: %s\n", output_folder);

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { // Regular file
            printf("Processing file: %s\n", entry->d_name);
            process_file(entry->d_name, output_folder);
        }
    }

    closedir(dir);
    printf("Analysis completed. Results saved in %s\n", output_folder);
    return 0;
}
