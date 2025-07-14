#include "pages.h"
#define DEBUG_PAGES

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>


struct BufferedPage {
	const char *original_name;
	char *new_name;
};

char *folder = NULL;
size_t path_len = 0;
struct BufferedPage *pages = NULL;
size_t pg_count = 0;


// Повертає `true`, якщо dirent повернув файл
// ! повертає `false`, якщо виникає помилка
// ! `path_len == 0` - означає помилку
bool isFile(const unsigned char type, const char *const name);
// Повертає кількість файлів у теці
size_t countFiles(DIR *const dir);
// Витягує назви файлів
// (кількість має бути вже знайдена)
// ! якщо файли пораховані неправильно, продовжувати неможна !
void getFilesNames(DIR *const dir);
// Сортуємо за ориґінальною назвою (buble sort)
void sortPaths(const size_t type_len);
// Виводить список `pages`
void showPages();


void openPages(const char *const path, const char *const type)
{
	closePages(); // In case of double opening
#ifdef DEBUG_PAGES
	printf("\tOpening Pages\n");
#endif
	if (path == NULL) { fprintf(stderr, "ERROR: A path has to be provided!\n"); return; }
	if (type == NULL) { fprintf(stderr, "ERROR: A type has to be provided (but may be empty)\n"); return; }

	path_len = strlen(path);
	if (path_len == 0) { fprintf(stderr, "ERROR: Path to the dirrectory has to be not empty!\n"); return; }
	if (path_len > PATH_MAX) { fprintf(stderr, "ERROR: Path to the dirrectory is to long (>%u)!\n", PATH_MAX); return; }
	if (strlen(type) > NAME_MAX) { fprintf(stderr, "ERROR: Type of files is to long (>%u)!\n", NAME_MAX); return; }
	if (type[0] != '.') { fprintf(stderr, "ERROR: Type has to start from `.`!\n"); return; }

	DIR *dir = opendir(path);
	if (dir == NULL) { fprintf(stderr, "ERROR: The dirrectory can not be open!\n"); return; }

	//  TODO: path has to ALWAYS end on /
	folder = malloc((path_len + NAME_MAX + 1) * sizeof(char));
	if (folder == NULL) { fprintf(stderr, "ERROR: Out of memory for path\n"); closedir(dir); return; }
	strlcpy(folder, path, path_len + 1);
#ifdef DEBUG_PAGES
	printf("Folder: '%s'\n", path);
	printf("Type: '%s'\n", type);
#endif

	pg_count = countFiles(dir);
	if (path_len == 0) { closedir(dir); return; }
	if (pg_count == 0) { fprintf(stderr, "ERROR: The dirrectory has no files in it.\n"); closedir(dir); closePages(); return; }
	rewinddir(dir);
#ifdef DEBUG_PAGES
	printf("Files found: %zu\n", pg_count);
#endif

	getFilesNames(dir);
	closedir(dir);
	if (path_len == 0) { /* Already notified & closed */ return; }
#ifdef DEBUG_PAGES
	showPages();
#endif

	sortPaths(strnlen(type, NAME_MAX));
#ifdef DEBUG_PAGES
	showPages();
#endif
}

void applyPages()
{
	//  TODO: do the applying
	closePages();
}

void closePages()
{
#ifdef DEBUG_PAGES
	printf("\tClosing Pages\n");
#endif
	if (folder != NULL) {
		free(folder);
		folder = NULL;
	}
	path_len = 0;
	if (pages != NULL) {
		for (size_t i = 0; i < pg_count; i++) { free(pages[i].new_name); }
		free(pages);
		pages = NULL;
	}
	pg_count = 0;
}


bool isFile(const unsigned char type, const char *const name)
{
	if (type == DT_REG) {
		return true;
	} else if (type == DT_UNKNOWN) {
		struct stat sb;
		strlcpy(folder + path_len, name, NAME_MAX);
		if (stat(folder, &sb) == -1) { fprintf(stderr, "ERROR: Can not indentify dir-entity\n"); closePages(); return false; }
		if ((sb.st_mode & S_IFMT) == S_IFREG) {
			return true;
		}
	}
	return false;
}

size_t countFiles(DIR *const dir)
{
	size_t count = 0;
	const struct dirent *de;
	while ((de = readdir(dir)) != NULL) {
		if (isFile(de->d_type, de->d_name)) {
			count++;
		}
		if (path_len == 0) { return 0; }
	}
	return count;
}

void getFilesNames(DIR *const dir)
{
	pages = malloc(pg_count * sizeof(struct BufferedPage));
	size_t i = 0;
	const struct dirent *de;
	while ((de = readdir(dir)) != NULL)
	{
		if (!isFile(de->d_type, de->d_name)) {
			if (path_len == 0) { return; }
			else { continue; }
		}
		if (i >= pg_count) { fprintf(stderr, "ERROR: Files was counted badly (founded more then counted)\n"); closePages(); return; }
		pages[i].original_name = de->d_name;
	#ifdef DEBUG_PAGES
		printf("File: %s\n", pages[i].original_name);
	#endif
		const size_t len = strnlen(de->d_name, NAME_MAX);
		pages[i].new_name = malloc(NAME_MAX * sizeof(char));
		strlcpy(pages[i].new_name, de->d_name, len < NAME_MAX ? len + 1 : NAME_MAX);
		i++;
	}
	if (i < pg_count) { fprintf(stderr, "ERROR: Files was counted badly (founded less then counted)\n"); pg_count = i; closePages(); return; }
}


void sortPaths(size_t type_len)
{
	if (pg_count == 0) { fprintf(stderr, "ERROR: The pages are not set!\n"); return; }
	for (size_t left = pg_count - 1; left > 0; left--) {
		for (size_t i = 0; i < left; i++) {
			// TODO: Support preffix & suffix
			const size_t bubble_len = strnlen(pages[i].original_name, NAME_MAX);
			const size_t water_len = strnlen(pages[i+1].original_name, NAME_MAX);
			if (bubble_len != water_len) {
				// Shorter from start
				if (bubble_len > water_len) {
					const struct BufferedPage tmp = pages[i + 1];
					pages[i + 1] = pages[i];
					pages[i] = tmp;
				}
				continue;
			}
			for (size_t check_symbol = 0; check_symbol < bubble_len - type_len; check_symbol++)
			{
				const char bubble = pages[i].original_name[check_symbol];
				const char water = pages[i+1].original_name[check_symbol];
				// TODO: Upgrade sorting ordering
				// https://en.m.wikipedia.org/wiki/Natural_sort_order
				// AND: fix cyrilic ordering
				if (bubble != water) {
					if (bubble > water) {
						const struct BufferedPage tmp = pages[i + 1];
						pages[i + 1] = pages[i];
						pages[i] = tmp;
					}
					break;
				}
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
	if (pg_count == 0) { fprintf(stderr, "ERROR: The pages are not set!\n"); return; }
	if (start > end) { fprintf(stderr, "ERROR: `start` must be before `end`\n"); return; }

	// TODO: the logic
}

