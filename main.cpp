#include "headers/jobs.hpp"

#include <bits/basic_string.h>
#include <cstring>

auto main(int argc, char** argv) -> int
{
    using namespace std::string_literals;
    bool stacked{};
    if (argc > 1 and argv[1] == "--stacked"s) { stacked = true; }
    return do_experiment(stacked);
}