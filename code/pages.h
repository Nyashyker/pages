#include <stdlib.h>
#include <stdio.h>


// Відкриває
void openPages(const char *const path, const char *const type);
// Утілює і закриває за собою
void applyPages();
// Закриває без втілення
void closePages();

// `start` та `end` мають бути дійсними номерами сторінок
void movePartPages(const size_t start, const size_t end, const size_t amount);

