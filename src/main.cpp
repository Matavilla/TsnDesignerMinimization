#include <iostream>
#include <string>
#include <cstring>

#include "Scheduler.h"


int ParseArgs(int argc, char *argv[], std::string& path) {
    for (size_t i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--dataPath") == 0) {
            path = argv[i + 1];
            i += 1;
        } else {
            std::cout << "Usage:" << std::endl;
            std::cout << "--dataPath <dataPath> - path for file with input data for algo" << std::endl;
            return 0;
        }
    }

    return 1;
}

int main(int argc, char *argv[]) {
    std::string dataPath;

    if (!ParseArgs(argc, argv, dataPath)) {
        return -1;
    }

    Scheduler sch(dataPath);
    sch->run();

    return 0;
}
