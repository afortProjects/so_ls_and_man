#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#define RESET   "\x1B[0m"
#define RED     "\x1B[31m"
#define GREEN   "\x1B[32m"
#define YELLOW  "\x1B[33m"
#define BLUE    "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN    "\x1B[36m"

void printPermissions(mode_t mode) {
    // If it is directory add d in the beginning
    printf((S_ISDIR(mode)) ? CYAN "d" : "-");

    // Permissions for user (owner)
    printf((mode & S_IRUSR) ? GREEN "r" : "-");
    printf((mode & S_IWUSR) ? GREEN "w" : "-");
    printf((mode & S_IXUSR) ? GREEN "x" : "-");
    
    // Permissions for group
    printf((mode & S_IRGRP) ? YELLOW "r" : "-");
    printf((mode & S_IWGRP) ? YELLOW "w" : "-");
    printf((mode & S_IXGRP) ? YELLOW "x" : "-");

    // Permissions for others
    printf((mode & S_IROTH) ? MAGENTA "r" : "-");
    printf((mode & S_IWOTH) ? MAGENTA "w" : "-");
    printf((mode & S_IXOTH) ? MAGENTA "x" : "-");
    printf(RESET);
}

void printFileInfo(const char *name, struct dirent *entry, struct stat *fileStat) {
    struct passwd *user;
    struct group *group;
    char date[20];

    user = getpwuid(fileStat->st_uid);
    group = getgrgid(fileStat->st_gid);
    strftime(date, 20, "%b %d %H:%M", localtime(&(fileStat->st_mtime)));

    // File name
    printf("%s%s ", CYAN, name);

    // File type & mode
    printPermissions(fileStat->st_mode);

    // Print hard links
    printf(" %ld ", fileStat->st_nlink);

    // User name login
    printf("%s ", user->pw_name);

    // User group name
    printf("%s ", group->gr_name);

    // File size
    printf("%s%lld ", BLUE, (long long)fileStat->st_size);

    // Date
    printf("%s%s ", YELLOW, date);

    printf(RESET);
}

void listDirectory(const char *path, int recursive, int showFileInfo, int level) {
    DIR *dir;
    struct dirent *entry;
    struct stat fileStat;

    // Open the directory
    dir = opendir(path);
    if (dir == NULL) {
        perror("Unable to open directory");
        return;
    }

    // Read directory entries
    while ((entry = readdir(dir)) != NULL) {
        // Skip current and parent directory entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Indent for recursive listing
        if (recursive && level > 0) {
            printf(BLUE);
            for (int i = 0; i < level - 1; i++) {
                printf("│   ");
            }
            printf("├── ");
        }

        // Build the full path
        char fullpath[256];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        // Get file information
        if (lstat(fullpath, &fileStat) < 0) {
            perror("Unable to get file status");
            continue;
        }
        printf(RESET);

        // Print file information if requested

        if (showFileInfo) {
            printFileInfo(entry->d_name, entry, &fileStat);
            printf("\n");
        } else {
            printf("%s%s\n", GREEN, entry->d_name);
        }

        // If recursive flag is set and it's a directory, recursively list its contents
        if (recursive && S_ISDIR(fileStat.st_mode)) {
            listDirectory(fullpath, recursive, showFileInfo, level + 1);
        }

    }

    // Close the directory
    closedir(dir);
}

bool checkForArgument(int argc, char* argv[], char* argument) {
    for(int i=0; i<argc; i++) {
        if(strcmp(argv[i], argument) == 0) return true;
    }
    return false;
}

int main(int argc, char *argv[]) {
    const char *path = ".";  // Default path is current directory
    int recursive = 0;
    int showFileInfo = 0;

    // Check command-line arguments
    if (argc > 1) {
        if (checkForArgument(argc, argv, "--author")) {
            printf("Wojciech Trapkowski - 193176\n");
            return 0;
        }
        if (checkForArgument(argc, argv, "--version")) {
            printf("custom_ls - 0.1\n");
            return 0;
        }
        if (checkForArgument(argc, argv, "--help")) {
            system("man ls");
            return 0;
        }
        if (checkForArgument(argc, argv, "-R")){
            recursive = 1;
        } 
        if (checkForArgument(argc, argv, "-l")) {
            showFileInfo = 1;
        }
        //Scan for folder path
        for(int i=0; i<argc; i++) {
            if(argv[i][0] != '-') {
                path = argv[i];
            }
        }
    }
    printf(RESET);

    // List the directory
    listDirectory(path, recursive, showFileInfo, 0);

    printf(RESET);

    return 0;
}
