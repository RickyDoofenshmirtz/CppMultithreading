#include "headers/tasks.hpp"

#include <cstring>

auto main(int argc, char** argv) -> int
{
    if (argc > 1 && strcmp(argv[1], "--smol") == 0) { return do_smallies(); }
    return do_biggie();
}