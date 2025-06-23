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
	if (argc <= 1) { return 1; }
	development_tests(argv[1]);

	return 0;
}

