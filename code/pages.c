#include "pages.h"
#include <stddef.h>
#define DEBUG_PAGES

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>


#define FILE_TYPE 8
//#include <linux/limits.h>
// PATH_MAX

struct BufferedPage {
	const char *original_name;
	char *new_name;
};

DIR *dir = NULL;
const char *folder = NULL;
struct BufferedPage *pages = NULL;
size_t pg_count = 0;


// Повертає кількість файлів у теці
void countFiles();
// Витягує назви файлів
// (кількість має бути вже знайдена)
// ! якщо файли пораховані неправильно, продовжувати неможна !
void getFilesNames();
// Сортуємо за назвою (buble sort)
void sortPaths();
// Виводить список `pages`
void showPages();


void openPages(const char *const path)
{
	folder = path;
#ifdef DEBUG_PAGES
	printf("Folder: %s\n", path);
#endif
	dir = opendir(path);
	if (dir == NULL) { fprintf(stderr, "ERROR: The dirrectory can not be open!\n"); closePages(); return; }

	countFiles();
	if (pg_count == 0) { fprintf(stderr, "ERROR: The dirrectory has not files in it.\n"); closePages(); return; }
#ifdef DEBUG_PAGES
	printf("Files found: %zu\n", pg_count);
#endif

	getFilesNames();
	if (dir == NULL) { /* Already notified & closed */ return; }
#ifdef DEBUG_PAGES
	showPages();
#endif

	sortPaths();
#ifdef DEBUG_PAGES
	showPages();
#endif
}

void applyPages()
{
	// TODO: do the applying
	closePages();
}

void closePages()
{
	if (dir != NULL) { closedir(dir); }
	folder = NULL;
	if (pages != NULL) { free(pages); }
	pg_count = 0;
}


void countFiles()
{
	const struct dirent *de;
	while ((de = readdir(dir)) != NULL) {
		if (de->d_type == FILE_TYPE) {
			pg_count++;
		}
	}
	rewinddir(dir);
}

void getFilesNames()
{
	pages = malloc(pg_count * sizeof(struct BufferedPage));
	size_t i = 0;
	const struct dirent *de;
	while ((de = readdir(dir)) != NULL)
	{
		if (de->d_type == FILE_TYPE) {
			if (i >= pg_count) { fprintf(stderr, "ERROR: Files was counted badly (founded more then counted)\n"); closePages(); return; }
			pages[i].original_name = de->d_name;
		#ifdef DEBUG_PAGES
			printf("File: %s\n", pages[i].original_name);
		#endif
			const size_t len = strnlen(de->d_name, MAXNAMLEN);
			pages[i].new_name = malloc(MAXNAMLEN * sizeof(char));
			strlcpy(pages[i].new_name, de->d_name, MAXNAMLEN);
			i++;
		}
	}
	if (i != pg_count) { fprintf(stderr, "ERROR: Files was counted badly (founded less then counted)\n"); closePages(); return; }
	rewinddir(dir);
}


void sortPaths()
{
	if (pages == NULL) { fprintf(stderr, "ERROR: The pages are not set!\n"); return; }
	for (size_t left = pg_count - 1; left > 0; left--) {
		for (size_t i = 0; i < left; i++) {
			for (size_t check_symbol = 0; check_symbol < MAXNAMLEN; check_symbol++)
			{
				const char bubble = pages[i].original_name[check_symbol];
				const char water = pages[i+1].original_name[check_symbol];
				if (bubble == water) { continue; }
				// TODO: Upgrade sorting ordering
				// https://en.m.wikipedia.org/wiki/Natural_sort_order
				// AND: fix cyrilic ordering
				if (bubble != '\0')
				{
					if (water == '\0' || bubble > water) {
						const struct BufferedPage tmp = pages[i + 1];
						pages[i + 1] = pages[i];
						pages[i] = tmp;
					}
				}
				break;
			}
		}
	}
}

void showPages()
{
	printf("Listed pages:\n");
	for (size_t i = 0; i < pg_count; i++) {
		printf("'%s' -> '%s'\n", pages[i].original_name, pages[i].new_name);
	}
	printf("\n");
}


void movePartPages(const size_t start, const size_t end, const size_t amount)
{
	if (pages == NULL) { fprintf(stderr, "ERROR: The pages are not set!\n"); return; }
	if (start > end) { fprintf(stderr, "ERROR: `start` must be before `end`\n"); return; }

	// TODO: the logic
}

