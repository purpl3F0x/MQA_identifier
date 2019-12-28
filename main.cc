
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>


#include "mqa_identifier.h"


int main(int argc, char *argv[]) {
    namespace fs = std::filesystem;

    std::vector<std::string> files;

    if (argc == 1) {
        std::cout
            << "HINT: To use the tool provide files and/or directories as program arguments\n\n";
    }

    for (auto argn = 1; argn < argc; argn++) {

        if (fs::is_directory(argv[argn]))
            for (const auto &entry : fs::directory_iterator(argv[argn])) {
                if (fs::is_regular_file(entry) && (fs::path(entry).extension() == ".flac"))
                    files.push_back(entry.path().string());

            }

        else if (fs::is_regular_file(argv[argn]))
            if (fs::path(argv[argn]).extension() == ".flac")
                files.emplace_back(argv[argn]);
            else
                std::cerr << argv[argn] << " not .flac file\n";

        else if (fs::is_empty(argv[argn]))
            std::cerr << argv[argn] << " not file or directory\n";
    }

    // Flush error buffer (just to make sure our print is pretty and no error line get in between)
    std::cerr << std::flush;

    // Let's do some printing
    std::cout << "**************************************************\n";
    std::cout << "***********  MQA flac identifier tool  ***********\n";
    std::cout << "********  Stavros Avramidis (@purpl3F0x)  ********\n";
    std::cout << "** https://github.com/purpl3F0x/MQA_identifier  **\n";
    std::cout << "**************************************************\n";

    std::cout << "Found " << files.size() << " file for scanning...\n\n";


    // Start parsing the files
    size_t count = 0;
    size_t mqa_files = 0;
    std::cout << "  #\tEncoding\tName\n";
    for (const auto &file : files) {
        std::cout << std::setw(3) << ++count << "\t";
        auto id = MQA_identifier(file);
        if (id.detect()) {
            if (id.originalSampleRate())
                std::cout << "MQA " << id.originalSampleRate() / 1000 << "K \t";
            else
                std::cout << "MQA\t\t";
            std::cout << fs::path(file).filename().string() << "\n";
            mqa_files++;
        } else
            std::cout << "NOT MQA \t" << fs::path(file).filename().string() << "\n";
    }

    std::cout << "\n**************************************************\n";
    std::cout << "Scanned " << files.size() << " files\n";
    std::cout << "Found " << mqa_files << " MQA files\n";
}
