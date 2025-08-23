#include "pages.h"
#include <stddef.h>
#define DEBUG_PAGES

#include <stdint.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>


typedef struct BufferedPage {
	const char *original_name;
	int order;
	bool changed /* = false */ ;
} Page;

char *folder = NULL;
size_t path_len = 0;
struct BufferedPage *pages = NULL;
size_t pg_count = 0;


// Повертає `true`, якщо dirent повернув файл
// ! повертає `false`, якщо виникає помилка
// ! `folder == NULL` - означає помилку
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
	if (path_len == 0) { fprintf(stderr, "ERROR: Path to the dirrectory has to be not empty!\n"); path_len = 0; return; }
	if (path_len > PATH_MAX) { fprintf(stderr, "ERROR: Path to the dirrectory is to long (>%u)!\n", PATH_MAX); path_len = 0; return; }
	if (strlen(type) > NAME_MAX) { fprintf(stderr, "ERROR: Type of files is to long (>%u)!\n", NAME_MAX); path_len = 0; return; }
	if (type[0] != '.') { fprintf(stderr, "ERROR: Type has to start from `.`!\n"); path_len = 0; return; }

	DIR *dir = opendir(path);
	if (dir == NULL) { fprintf(stderr, "ERROR: The dirrectory can not be open!\n"); path_len = 0; return; }

	const bool added_path_separator = path[path_len-1] != '/' && path[path_len-1] != '\\';
	path_len += added_path_separator;
	folder = malloc((path_len + NAME_MAX) * sizeof(char));
	if (folder == NULL) { fprintf(stderr, "ERROR: Out of memory for path\n"); path_len = 0; closedir(dir); return; }
	strncpy(folder, path, path_len - added_path_separator);
	if (added_path_separator) { folder[path_len-1] = '/'; }
#ifdef DEBUG_PAGES
	printf("Folder: '%s'\n", folder);
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
	if (folder == NULL) { /* Already notified & closed */ return; }
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
		if (de->d_name[0] == '.') { continue; }
		if (isFile(de->d_type, de->d_name)) {
			count++;
		} else
		if (folder == NULL) { /* Already notified & closed */ return 0; }
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
		if (de->d_name[0] == '.') { continue; }
		if (!isFile(de->d_type, de->d_name)) {
			if (folder == NULL) { /* Already notified & closed */ return; }
			else { continue; }
		}
		if (i >= pg_count) { fprintf(stderr, "ERROR: Files was counted badly (founded more then counted)\n"); closePages(); return; }

		pages[i].original_name = de->d_name;
		pages[i].changed = false;
	#ifdef DEBUG_PAGES
		printf("File: %s\n", pages[i].original_name);
	#endif
		const size_t len = strnlen(de->d_name, NAME_MAX);
		size_t suffix = len - 1;
		for (; suffix > 0; suffix--) { if (de->d_name[suffix] == '.') { break; } }
		const size_t the_name_len = len + 1 - (de->d_name[suffix] == '.' ? suffix : 0);
		char *the_name = malloc(the_name_len * sizeof(char));
		if (the_name == NULL) { fprintf(stderr, "ERROR: Not enough memory for name analysis\n"); closePages(); return; }
		strlcpy(the_name, de->d_name, the_name_len);
		const int order = atoi(the_name);
		if (order == 0 && the_name[0] != '0') {
			pages[i].order = -i - 1;
		} else {
			pages[i].order = order;
		}
		free(the_name);

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
					const Page tmp = pages[i + 1];
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
						const Page tmp = pages[i + 1];
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
		printf("%zu.\t'%s' -> %i%s\n", i, pages[i].original_name, pages[i].order, pages[i].changed?" +":"");
	}
	printf("\n");
}


// Raw version for inner use
void _movePartPages(const size_t start, const size_t end, const int amount);

void movePartPages(const size_t start, const size_t end, const int amount)
{
	if (pg_count == 0) { fprintf(stderr, "ERROR: The pages are not set!\n"); return; }
	if (start > end) { fprintf(stderr, "ERROR: `start` must be before `end`\n"); return; }

	#ifdef DEBUG_PAGES
		printf("\tMoving pages from %zu to %zu on %i\n", start, end, amount);
	#endif
	_movePartPages(start, end, amount);
	
	#ifdef DEBUG_PAGES
		showPages();
	#endif
}

void orderlinePages(const size_t start)
{
	if (pg_count == 0) { fprintf(stderr, "ERROR: The pages are not set!\n"); return; }
	#ifdef DEBUG_PAGES
		printf("\tOrderlining pages to start from %zu\n", start);
	#endif

	size_t the_index = 0;
	int the_last_smallest = INT_MIN;
	for (size_t order = start; order < start + pg_count; order++) {
		int the_smallest = INT_MAX;
		for (size_t i = 0; i < pg_count; i++) {
			if (pages[i].order > the_last_smallest && pages[i].order < the_smallest) {
				the_index = i;
				the_smallest = pages[i].order;
			}
		}
		the_last_smallest = the_smallest;
		pages[the_index].order = order;
		pages[the_index].changed = true;
	}
	#ifdef DEBUG_PAGES
		showPages();
	#endif
}


void _movePartPages(const size_t start, const size_t end, const int amount)
{
	for (size_t i = 0; i < pg_count; i++)
	{
		if (pages[i].order >= start && pages[i].order <= end) {
			pages[i].order += amount;
			pages[i].changed = true;
		}
	}
}

