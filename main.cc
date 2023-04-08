/**
 * @file        main.cc
 * @author      Stavros Avramidis (@purpl3F0x)
 * @date        16/12/2019
 * @copyright   2019 Stavros Avramidis under Apache 2.0 License
 */

#ifdef __ANDROID__
 #include <boost/filesystem.hpp>
#else
 #include <filesystem>
#endif

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "mqa_identifier.h"
#include <FLAC++/metadata.h>

#ifdef __ANDROID__
 namespace fs = boost::filesystem;
#else
 namespace fs = std::filesystem; // for old VS use namespace fs = std::experimental::filesystem;
#endif

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
	bool add_mqaencoder = false;
	bool rewrite_founded_tags = false;

	if (argc == 1) {
		std::cout << "HINT: To use the tool provide files and/or directories as program arguments\n" \
			"      If yor want add tags use flag --add-mqaencoder and -rw if yor want rewrite existing ones.\n\n";
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
		if (std::string(argv[argn]) == "--add-mqaencoder")
			add_mqaencoder = true;

		if (add_mqaencoder && std::string(argv[argn]) == "-rw")
			rewrite_founded_tags = true;
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
	size_t added_tags = 0;
    std::cout << "  #\tEncoding\t\tName\n";
    for (const auto &file : files) 
    {
        std::cout << std::setw(3) << ++count << "\t";

        //auto id = MQA_identifier(file);
        MQA_identifier id(file);
        if (id.detect())
        {
            std::cout << "MQA " << (id.isMQAStudio() ? "Studio " : "")
                      << getSampleRateString(id.originalSampleRate()) << "  \t"
                      << fs::path(file).filename().string() << "\n";
            mqa_files++;

            // add tags if use flag --add-mqaencoder and -rw if yor want rewrite existing ones
			if (add_mqaencoder) 
			{
				//////////////////////////////////////////
				// read a file using FLAC::Metadata::Chain class
				FLAC::Metadata::Chain chain;
				chain.read(file.c_str());
				// now, find vorbis comment block and make changes in it
				{
					FLAC::Metadata::Iterator iterator;
					iterator.init(chain);
					// find vorbis comment block
					FLAC::Metadata::VorbisComment* vcBlock = 0;
					do {
						FLAC::Metadata::Prototype* block = iterator.get_block();
						if (block->get_type() == FLAC__METADATA_TYPE_VORBIS_COMMENT) {
							vcBlock = (FLAC::Metadata::VorbisComment*) block;
							break;
						}
					} while (iterator.next());
					// if not found, create a new one
					if (vcBlock == 0) {
						// create a new block
						vcBlock = new FLAC::Metadata::VorbisComment();
						// move iterator to the end
						while (iterator.next()) {
						}
						// insert a new block at the end
						if (!iterator.insert_block_after(vcBlock)) {
							delete vcBlock;
						}
					}
					//if the tags (ENCODER,MQAENCODER,ORIGINALSAMPLERATE) is found, we simply delete it
					int ENCODERs = -1;
					int MQAENCODERs = -1;
					int ORIGINALSAMPLERATEs = -1;
					for (int i = 0; i < vcBlock->get_num_comments(); ++i)
					{
						int ENCODER = vcBlock->find_entry_from(i, "ENCODER");
						if (ENCODER > -1)
							ENCODERs = ENCODER;
					}
					if (ENCODERs > -1 && rewrite_founded_tags)
					{
						vcBlock->delete_comment(ENCODERs);
					}
					for (int i = 0; i < vcBlock->get_num_comments(); ++i)
					{
						int MQAENCODER = vcBlock->find_entry_from(i, "MQAENCODER");
						if (MQAENCODER > -1)
							MQAENCODERs = MQAENCODER;
					}
					if (MQAENCODERs > -1 && rewrite_founded_tags)
					{
						vcBlock->delete_comment(MQAENCODERs);
					}
					for (int i = 0; i < vcBlock->get_num_comments(); ++i)
					{
						int ORIGINALSAMPLERATE = vcBlock->find_entry_from(i, "ORIGINALSAMPLERATE");
						if (ORIGINALSAMPLERATE > -1)
							ORIGINALSAMPLERATEs = ORIGINALSAMPLERATE;
					}
					if (ORIGINALSAMPLERATEs > -1 && rewrite_founded_tags)
					{
						vcBlock->delete_comment(ORIGINALSAMPLERATEs);
					}
					//add tags (ENCODER,MQAENCODER,ORIGINALSAMPLERATE) to the flac file if not found or if -rw flag rewrite existing
					std::string OrigSamp = std::to_string(id.originalSampleRate());
					if(ENCODERs == -1 || ENCODERs > -1 && rewrite_founded_tags)
						vcBlock->append_comment(FLAC::Metadata::VorbisComment::Entry("ENCODER", "MQAEncode v1.1, 2.3.3+800 (a505918), F8EC1703-7616-45E5-B81E-D60821434062, Dec 01 2017 22:19:30"));
					if (MQAENCODERs == -1 || MQAENCODERs > -1 && rewrite_founded_tags)
						vcBlock->append_comment(FLAC::Metadata::VorbisComment::Entry("MQAENCODER", "MQAEncode v1.1, 2.3.3+800 (a505918), F8EC1703-7616-45E5-B81E-D60821434062, Dec 01 2017 22:19:30"));
					if (ORIGINALSAMPLERATEs == -1 || ORIGINALSAMPLERATEs > -1 && rewrite_founded_tags)
						vcBlock->append_comment(FLAC::Metadata::VorbisComment::Entry("ORIGINALSAMPLERATE", OrigSamp.c_str()));

					if (ENCODERs == -1 || ENCODERs > -1 && rewrite_founded_tags
						|| MQAENCODERs == -1 || MQAENCODERs > -1 && rewrite_founded_tags
						|| ORIGINALSAMPLERATEs == -1 || ORIGINALSAMPLERATEs > -1 && rewrite_founded_tags)
						added_tags += 1;
				}
				chain.write();//save flac file
			}
            
        } 
        else
            std::cout << "NOT MQA \t" << fs::path(file).filename().string() << "\n";

    }

    std::cout << "\n**************************************************\n";
    std::cout << "Scanned " << files.size() << " files\n"; 
    std::cout << "Found " << mqa_files << " MQA files\n";
	std::cout << "Added " << added_tags << " tags for MQA files\n";
}
