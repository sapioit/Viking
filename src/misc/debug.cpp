#include <misc/debug.h>

void bug_on(bool condition, const std::string &text) {
    if (condition)
        debug(text);
    exit(EXIT_FAILURE);
}
