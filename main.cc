/**
 * @filename main.cc
 * @author Stavros Avramidis (@purpl3F0x)
 * @date 9/12/2019
 * @copyright 2019 Stavros Avramidis under Apache 2.0 License
 * @short Handy tool for identifying MQA files (This tool isn't related with MQA Ltd. and is made for purely educational purposes)
 */

#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>

#define DR_FLAC_IMPLEMENTATION

#include <dr_flac.h>


struct myUserData {
    int MQADetection = -1;      // -1 no mqa, 0 unsure, >0 bit where mqa is detected
    unsigned originalSampleRAte = 0;
};


/**
 * @short   Parses file for MQA data
 * @param   file filename to be parsed
 * @return  Returns the bit where MQA data are found
 */
auto identifier(const std::string &file) {
    myUserData result;
    // Get metadata of the file, to try to see if there is an MQAENCODER vorbis comment
    drflac *pFlac = drflac_open_file_with_metadata(
        file.c_str(),
        [](void *pUserData, drflac_metadata *pMetadata) {
          auto data = (myUserData *) pUserData;
          if (pMetadata->type == DRFLAC_METADATA_BLOCK_TYPE_VORBIS_COMMENT) {
              auto *it = new drflac_vorbis_comment_iterator;
              drflac_uint32 s = 128u;
              drflac_init_vorbis_comment_iterator(it,
                                                  pMetadata->data.vorbis_comment.commentCount,
                                                  pMetadata->data.vorbis_comment.pComments);
              while (it->countRemaining > 0) {
                  char comment[128];
                  std::strcpy(comment, drflac_next_vorbis_comment(it, &s));
                  comment[s] = '\0';
                  if (std::strncmp("MQAENCODER", comment, 10) == 0)
                      data->MQADetection = 0;
                  else if (std::strncmp("ORIGINALSAMPLERATE", comment, 18) == 0)
                      data->originalSampleRAte = std::stoul(comment + 19);

                  s = 128u;
              }
          }
        },
        (void *) &result
    );

    // Just in case file isn't found
    if (pFlac == nullptr)
        return result;

    auto pSampleData = (int32_t *) malloc((size_t) pFlac->totalPCMFrameCount * pFlac->channels * sizeof(int32_t));
    drflac_read_pcm_frames_s32(pFlac, pFlac->totalPCMFrameCount, pSampleData);

    // At this point pSampleData contains every decoded sample as signed 32-bit PCM.
    for (int p = 10; p < 32; p++) {
        uint64_t buffer = 0;

        for (auto i = 0u; i < pFlac->sampleRate; i += 2) {
            buffer |= ((pSampleData[i] ^ pSampleData[1 + i]) >> p) & 1u;
            if (buffer == 0xbe0498c88) { // <== MQA magic word
                result.MQADetection = p;
                return result;
            } else
                buffer = (buffer << 1u) & 0xFFFFFFFFFu;
        }
    }

    drflac_close(pFlac);
    return result;
}


int main(int argc, char *argv[]) {
    namespace fs = std::filesystem;

    std::vector<std::string> files;


    if (argc == 1) {
        std::cout << "HINT: To use the tool provide files and directories as program arguments";
    }

    for (auto argn = 1; argn < argc; argn++) {
        std::cout << argv[argn] << "\n";
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
    size_t mqa_files = 0, not_sure = 0;
    for (const auto &file : files) {
        std::cout << "(" << count++ << "/" << files.size() << ")  " << file << "\n";
        std::cout << "\t";
        auto res = identifier(file);
        switch (res.MQADetection) {
            case -1:
                std::cout << "file isn't MQA\n";
                break;

            case 0:
                not_sure++;
                std::cout << "MQAENCODER reported in metadata but file doesn't appear to be MQA\n";
                break;

            default:
                mqa_files++;
                std::cout << "file is MQA (" << "on bit " << res.MQADetection << ")";
                if (res.originalSampleRAte)
                    std::cout << " Metadata Original SampleRate " << res.originalSampleRAte;
                std::cout << "\n";
        }
    }

    std::cout << "\n**************************************************\n";
    std::cout << "Scanned " << files.size() << " files\n";
    std::cout << "Found " << mqa_files << " MQA files\n";
    std::cout << "Maybe " << not_sure << " more are MQA\n";

}