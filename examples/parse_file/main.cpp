#include <fstream>
#include <iostream>
#include <string>

#include "qlib/json.h"
#include "qlib/string.h"

int32_t main(int32_t argc, char* argv[]) {
    int32_t result{0};

    do {
        if (argc < 2) {
            std::cerr << "Usage: " << argv[0] << " <json_file>" << std::endl;
            result = -1;
            break;
        }

        std::ifstream ifs(argv[1]);
        std::string text{std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};

        using namespace qlib;

        json_view_t json;
        result = json::parse(&json, text.data(), text.data() + text.size());
        if (0 != result) {
            std::cout << "json::parse return " << result << std::endl;
            break;
        }

        std::ofstream ofs("output.json");
        if (!ofs.is_open()) {
            std::cerr << "Failed to open output file." << std::endl;
            result = -2;
            break;
        }
        ofs << json;
    } while (false);

    return result;
}
