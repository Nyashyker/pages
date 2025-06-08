#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <dirent.h>

#define FILE_TYPE 8


int countFiles(const char *const path);
void sortPaths(const struct dirent ** list_of_files, const unsigned int len);

// `start` та `end` мають бути дійсними номерами сторінок
void movePartPages(const char *const path, const unsigned int start, const unsigned int end, const unsigned int amount);


int main(const int argc, const char *const *const argv) {
    const char *const test_dir = "./test/";

    movePartPages(test_dir, 0, 5, +1);
    if (argc <= 1) {
        return 0;
    }
    return 0;

    for (int i = 1; i < argc; i++) {
        DIR *const dir = opendir(argv[i]);

        const struct dirent *de;
        while ((de = readdir(dir)) != NULL) {
            printf("%s -> %i\n", de->d_name, de->d_type);
        }
        printf("\n");

        closedir(dir);
    }

    return 0;
}

int countFiles(const char *const path)
{
    // Повертає кількість файлів у теці
    DIR *const dir = opendir(path);

    int count = 0;

    const struct dirent *de;
    while ((de = readdir(dir)) != NULL) {
        if (de->d_type == FILE_TYPE) {
            count++;
        }
    }

    return count;
}

void sortPaths(const struct dirent ** list_of_files, const unsigned int len)
{
    // Сортуємо за назвою

    for (unsigned int left = len; left > 0; left--) {
        printf("%02u:", left);
        for (unsigned int i = 0; i < left; i++) {
            printf(" %2u", i);
        }
        printf("\n");
    }
}

void movePartPages(const char *const path, const unsigned int start, const unsigned int end, const unsigned int amount)
{
    if (start > end) {
        fprintf(stderr, "`start` must be before `end`");
        return;
    }

    const unsigned int len = countFiles(path);
    if (len == 0) {
        fprintf(stderr, "The dirrectory has not files in it.");
        return;
    }
    const struct dirent **list_of_dirs = malloc(len * sizeof(struct dirent *));
    {
        // Заповнюємо список
        unsigned int i = 0;

        DIR *const dir = opendir(path);
        const struct dirent *de;
        while ((de = readdir(dir)) != NULL) {
            if (de->d_type == FILE_TYPE) {
                list_of_dirs[i++] = de;
            }
        }
    }

    sortPaths(list_of_dirs, len);
}

