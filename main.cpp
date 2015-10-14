#include "slist.h"
#include <cstdlib>
#include <fstream>

namespace
{
    std::vector<std::string> parse_arguments(int argc, char **argv);
}

int main(int argc, char **argv)
{
    using namespace slist;

    auto trailing = parse_arguments(argc, argv);

    if (trailing.empty()) 
    {
        // TODO: REPL
    }
    else if (trailing.size() == 1)
    {
        std::ifstream in(trailing[0]);
        if (in)
        {
            context ctx;
            exec(ctx, in);
        }
        else 
        {
            log_errorln("Invalid input file: " + trailing[0]);
            return -1;
        }
    }
    else 
    {
        log_errorln("Too many arguments");
        return -1;
    }

    // node_ptr n = parse("");

    // if (get_log_level() >= log_level::trace)
    // {
    //     log_traceln("");
    //     debug_print_node(n);
    //     log_traceln("");
    // }

    // context ctx;
    // while (n != nullptr)
    // {
    //     auto r = eval(ctx, n->car);
    //     if (r != nullptr)
    //     {
    //         outputln("", r);
    //     }
    //     n = n->cdr;
    // }

    return 0;
}

namespace
{
    std::vector<std::string> parse_arguments(int argc, char **argv)
    {
        using namespace std;
        using namespace slist;

        --argc;
        ++argv;

        std::vector<std::string> trailing;

        for (int i = 0; i < argc; ++i)
        {
            char *arg = argv[i];
            if (strcmp(arg, "-l") == 0 || strcmp(arg, "--log-level") == 0)
            {
                ++i;
                if (i < argc && argv[i] != nullptr)
                {
                    arg = argv[i];
                    log_level level = (log_level)atoi(arg);
                    set_log_level(level);
                }
                else 
                {
                    log_error("Invalid argument to '-v'/'--log-level'\n");
                }
            }
            else 
            {
                trailing.push_back(arg);
            }
        }

        return trailing;
    }
}