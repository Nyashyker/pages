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
	if (argc <= 2) {
		fprintf(stderr, "You have to provide a folder & type\n");
		return 1;
	}
	openPages(argv[1], argv[2]);
	closePages();

	return 0;
}

