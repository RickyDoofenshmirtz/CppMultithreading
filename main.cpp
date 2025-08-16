#include "headers/jobs.hpp"

#include <cstdint>
#include <xstring>

auto main(int argc, char** argv) -> std::int32_t
{
    using namespace std::string_literals;
    bool stacked{};
    if (argc > 1 and argv[1] == "--stacked"s) { stacked = true; }
    return do_experiment(stacked);
}