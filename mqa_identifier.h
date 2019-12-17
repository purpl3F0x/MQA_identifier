/**
 * @file        mqa_identifier.hh
 * @author      Stavros Avramidis (@purpl3F0x)
 * @date        16/12/2019
 * @copyright   2019 Stavros Avramidis under Apache 2.0 License
 * @short       Library to Identify MQA encoding
 */

#pragma once

#include <array>
#include <cinttypes>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include <FLAC++/decoder.h>


class MQA_identifier {
 private:
  class MyDecoder : public FLAC::Decoder::File {
   public:
    uint32_t sample_rate = 0;
    uint32_t channels = 0;
    uint32_t bps = 0;
    FLAC__uint64 decoded_samples = 0;
    std::vector<std::array<const FLAC__int32, 2>> samples;
    std::string mqa_endoder;
    uint32_t original_sample_rate = 0;

    explicit MyDecoder(std::string file) : FLAC::Decoder::File(), file_(std::move(file)) {};

    ::FLAC__StreamDecoderInitStatus decode();

   protected:
    std::string file_;
    using FLAC::Decoder::File::init;
    virtual ::FLAC__StreamDecoderWriteStatus write_callback(const ::FLAC__Frame *frame,
                                                            const FLAC__int32 *const buffer[]) override;
    void metadata_callback(const ::FLAC__StreamMetadata *metadata) override;
    void error_callback(::FLAC__StreamDecoderErrorStatus status) override;

   private:
    MyDecoder(const MyDecoder &);
    MyDecoder &operator=(const MyDecoder &);
  };


  std::string file_;
  MyDecoder decoder;
  bool isMQA_;

 public:
  explicit MQA_identifier(std::string file) : file_(std::move(file)), decoder(file_), isMQA_(false) {}

  bool detect();

  [[nodiscard]] std::string getMQA_encoder() const noexcept;
  [[nodiscard]] uint32_t originalSampleRate() const noexcept;
  [[nodiscard]] bool isMQA() const noexcept;
  [[nodiscard]] std::string filename() const noexcept;
};


::FLAC__StreamDecoderWriteStatus MQA_identifier::MyDecoder::write_callback(const ::FLAC__Frame *frame,
                                                                           const FLAC__int32 *const buffer[]) {

    if (channels != 2 || (bps != 16 && bps != 24)) {
        std::cerr << "ERROR: this tool only supports 16bit/24bit stereo streams\n";
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }

    /* increase number of read samples */
    this->decoded_samples += frame->header.blocksize;

    /* write decoded PCM samples */
    for (size_t i = 0; i < frame->header.blocksize; i++)
        this->samples.push_back((std::array<const FLAC__int32, 2>) {buffer[0][i], buffer[1][i]});

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}


void MQA_identifier::MyDecoder::metadata_callback(const ::FLAC__StreamMetadata *metadata) {

    if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
        this->sample_rate = metadata->data.stream_info.sample_rate;
        this->channels = metadata->data.stream_info.channels;
        this->bps = metadata->data.stream_info.bits_per_sample;

    } else if (metadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT) {
        for (FLAC__uint32 i = 0; i < metadata->data.vorbis_comment.num_comments; i++) {
            const auto comment = reinterpret_cast<char *>(metadata->data.vorbis_comment.comments[i].entry);

            if (std::strncmp("MQAENCODER", comment, 10) == 0)
                this->mqa_endoder =
                    std::string(comment + 10, comment + metadata->data.vorbis_comment.comments[i].length);

            else if (std::strncmp("ORIGINALSAMPLERATE", comment, 18) == 0)
                this->original_sample_rate = std::stoul(comment + 19);

            else if (std::strncmp("MQASAMPLERATE", comment, 13) == 0)
                this->original_sample_rate = std::stoul(comment + 14);
        }
    }

}

void MQA_identifier::MyDecoder::error_callback(::FLAC__StreamDecoderErrorStatus status) {
    std::cerr << "Got error callback: " << FLAC__StreamDecoderErrorStatusString[status] << "\n";
}

::FLAC__StreamDecoderInitStatus MQA_identifier::MyDecoder::decode() {
    bool ok = true;

    (void) this->set_md5_checking(true);
    (void) this->set_metadata_respond(FLAC__METADATA_TYPE_VORBIS_COMMENT); /* instruct decoder to parse vorbis_comments */
    FLAC__StreamDecoderInitStatus init_status = this->init(this->file_);

    if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        std::cerr << "ERROR: initializing decoder: " << FLAC__StreamDecoderInitStatusString[init_status] << "\n";
        ok = false;
    }

    this->process_until_end_of_metadata();

    // pre-allocate samples vector
    this->samples.reserve(this->sample_rate * 3);

    while (this->decoded_samples < this->sample_rate * 3 /* read only 3 first seconds */ )
        ok = this->process_single();

    if (!ok) {
        std::cerr << "decoding FAILED\n";
        std::cerr << this->get_state().resolved_as_cstring(*this);
    }
    return init_status;
}


bool MQA_identifier::detect() {
    this->decoder.decode();

    for (auto p = 0u; p < this->decoder.bps; p++) {
        uint64_t buffer = 0;
        for (const auto &s: this->decoder.samples) {
            buffer |=
                ((static_cast<uint32_t>(s[0]) ^ static_cast<uint32_t>(s[1])) >> p) & 1u;  //static_cast for clang-tidy
            if (buffer == 0xbe0498c88) {        // MQA magic word
                this->isMQA_ = true;
                return true;
            } else
                buffer = (buffer << 1u) & 0xFFFFFFFFFu;
        }
    }
    return false;
}

std::string MQA_identifier::getMQA_encoder() const noexcept {
    return this->decoder.mqa_endoder;
}

uint32_t MQA_identifier::originalSampleRate() const noexcept {
    return this->decoder.original_sample_rate;
}

bool MQA_identifier::isMQA() const noexcept {
    return this->isMQA_;
}

std::string MQA_identifier::filename() const noexcept {
    return this->file_;
}
