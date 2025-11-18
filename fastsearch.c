#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>

// Global flag indicating thread limit was reached.
volatile char thread_limit_reached = 0;

// Data passed to threads: A current path and the target file name.
struct threadTask {
  char path[1024];
  char target[256];
};

// The function called by each thread to scan a directory.
void *scan_dir(void *arg) {
  struct threadTask *task = (struct threadTask *)arg;
  DIR *directory = opendir(task->path);
  if (!directory) {
    free(task);
    return NULL;
  }
  struct dirent *entry;
  while ((entry = readdir(directory)) != NULL) {
    // Filter out current and parent directories.
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }
    char full_path[1024];
    memset(full_path, 0, sizeof(full_path));
    strncpy(full_path, task->path, sizeof(full_path) - 1);
    size_t len = strlen(full_path);
    if (len < sizeof(full_path) - 2) {
      full_path[len] = '/';
      full_path[len + 1] = '\0';
    }
    strncat(full_path, entry->d_name, sizeof(full_path) - strlen(full_path) - 1);
    // Check filename for match.
    if (strcmp(entry->d_name, task->target) == 0) {
      printf("Found: %s\n", full_path);
    }
    struct stat st;
    if (stat(full_path, &st) == 0 && S_ISDIR(st.st_mode)) {
      pthread_t th;
      struct threadTask *childTask = malloc(sizeof(struct threadTask));
      strncpy(childTask->path, full_path, sizeof(childTask->path));
      strncpy(childTask->target, task->target, sizeof(childTask->target));
      if (pthread_create(&th, NULL, scan_dir, childTask)) {
        thread_limit_reached = 1;
        scan_dir(childTask);
        free(childTask);
      } else {
        pthread_detach(th);
      }
    }
  }
  closedir(directory);
  free(task);
  return NULL;
}

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("Usage: %s <start_dir> <filename>\n", argv[0]);
    return 1;
  }
  // Build the first task structure.
  struct threadTask *root = malloc(sizeof(struct threadTask));
  strncpy(root->path, argv[1], sizeof(root->path));
  strncpy(root->target, argv[2], sizeof(root->target));
  // Begin the first searching thread!
  pthread_t th;
  if (pthread_create(&th, NULL, scan_dir, root) != 0) {
    printf("Thread creation failed at start.\n");
    free(root);
    return 1;
  }
  // Captures the first thread.
  pthread_join(th, NULL);
  return 0;
}
