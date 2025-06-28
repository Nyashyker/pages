#include "pages.h"

/* # ZAPLANOVANO #
 * Funkciji:
 * - number
 * - order
 * - move
 * - konvertacija ffnpeg-om
 * Osoblyvosti:
 * - dovžyna
 * - prefiks & sufiks
 */

int main(const int argc, const char *const *const argv)
{
	if (argc <= 1) {
		fprintf(stderr, "You have to provide a folder\n");
		return 1;
	}
	openPages(argv[1]);
	//applyPages();
	closePages();

	return 0;
}

