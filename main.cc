/**
 * @file        main.cc
 * @author      Stavros Avramidis (@purpl3F0x)
 * @date        16/12/2019
 * @copyright   2019 Stavros Avramidis under Apache 2.0 License
 */

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "mqa_identifier.h"

namespace fs = std::filesystem;


auto getSampleRateString(const uint32_t fs) {
    std::stringstream ss;
    if (fs <= 768000)
        ss << fs / 1000. << "K";
    else if (fs % 44100 == 0)
        ss << "DSD" << fs / 44100;
    else
        ss << "DSD" << fs / 48000 << "x48";

    return ss.str();
}


/**
 * @short Recursively scan a directory for .flac files
 * @param curDir directory to scan
 * @param files vector to add the file paths
 */
void recursiveScan(const fs::directory_entry &curDir, std::vector<std::string> &files) {
    for (const auto &entry : fs::directory_iterator(curDir)) {
        if (fs::is_regular_file(entry) && (fs::path(entry).extension() == ".flac"))
            files.push_back(entry.path().string());

        else if (fs::is_directory(entry))
            recursiveScan(entry, files);
    }
}


int main(int argc, char *argv[]) {

    std::vector<std::string> files;

    if (argc == 1) {
        std::cout << "HINT: To use the tool provide files and/or directories as program arguments\n\n";
    }

    for (auto argn = 1; argn < argc; argn++) {

        if (fs::is_directory(argv[argn]))
            recursiveScan(fs::directory_entry(argv[argn]), files);

        else if (fs::is_regular_file(argv[argn])) {
            if (fs::path(argv[argn]).extension() == ".flac")
                files.emplace_back(argv[argn]);
            else
                std::cerr << argv[argn] << " not .flac file\n";
        }

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
                std::cout << "MQA " << getSampleRateString(id.originalSampleRate()) << "\t";
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
