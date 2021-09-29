/**
 * @file        mqa_identifier.h
 * @author      Stavros Avramidis (@purpl3F0x)
 * @date        16/12/2019
 * @copyright   2019 2.0 License
 * @short       Library to Identify MQA encoding
 */

#pragma once

#include <array>
#include <cinttypes>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include <FLAC++/decoder.h>


/**
 * Returns original Sample rate (in Hz) from waveform bytecode.
 * @param c 4bit bytecode
 * @return
 */
uint32_t OriginalSampleRateDecoder(unsigned c) {
    /*
     * If LSB is 0 then base is 44100 else 48000
     * 3 MSB need to be rotated and raised to the power of 2 (so 1, 2, 4, 8, ...)
     * output is base * multiplier
     */
    const uint32_t base = (c & 1u) ? 48000 : 44100;

    uint32_t multiplier = 1u << (((c >> 3u) & 1u) | (((c >> 2u) & 1u) << 1u) | (((c >> 1u) & 1u) << 2u));
    // Double for DSD
    if (multiplier > 16) multiplier *= 2;

    return base * multiplier;
}


class MQA_identifier {
 private:
  class MyDecoder : public FLAC::Decoder::File {
   public:
    uint32_t sample_rate = 0;
    uint32_t channels = 0;
    uint32_t bps = 0;
    FLAC__uint64 decoded_samples = 0;
    std::vector<std::array<const FLAC__int32, 2>> samples;
    std::string mqa_encoder;
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
  bool isMQAStudio_ = false;

 public:
  explicit MQA_identifier(std::string file) : file_(std::move(file)), decoder(file_), isMQA_(false) {}


  bool detect();

  [[nodiscard]] std::string getMQA_encoder() const noexcept;
  [[nodiscard]] uint32_t originalSampleRate() const noexcept;
  [[nodiscard]] bool isMQA() const noexcept;
  [[nodiscard]] bool isMQAStudio() const noexcept;
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
        this->samples.push_back(std::array < const FLAC__int32, 2 > {buffer[0][i], buffer[1][i]});

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
                this->mqa_encoder =
                    std::string(comment + 10, comment + metadata->data.vorbis_comment.comments[i].length);
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
    //check the bufer three times from the P,P+1,P+2 value
    uint64_t buffer = 0;
    uint64_t buffer1 = 0;
    uint64_t buffer2 = 0;
    const auto pos = (this->decoder.bps - 16u); // aim for 16th bit

    for (const auto &s: this->decoder.samples) {
        buffer  |= ((static_cast<uint32_t>(s[0]) ^ static_cast<uint32_t>(s[1])) >> pos    ) & 1u;
        buffer1 |= ((static_cast<uint32_t>(s[0]) ^ static_cast<uint32_t>(s[1])) >> pos + 1) & 1u;
        buffer2 |= ((static_cast<uint32_t>(s[0]) ^ static_cast<uint32_t>(s[1])) >> pos + 2) & 1u;

        if (buffer == 0xbe0498c88) {        // MQA magic word
            this->isMQA_ = true;
            // Get Original Sample Rate
            uint8_t orsf = 0;
            for (auto m = 3u; m < 7; m++) { // TODO: this need fix (orsf is 5bits)
                auto cur = *(&s + m);
                auto j = ((static_cast<uint32_t>(cur[0]) ^ static_cast<uint32_t>(cur[1])) >> pos) & 1u;
                orsf |= j << (6u - m);
            }
            this->decoder.original_sample_rate = OriginalSampleRateDecoder(orsf);

            // Get MQA Studio
            uint8_t provenance = 0u;
            for (auto m = 29u; m < 34; m++) {
                auto cur = *(&s + m);
                auto j = ((static_cast<uint32_t>(cur[0]) ^ static_cast<uint32_t>(cur[1])) >> pos) & 1u;
                provenance |= j << (33u - m);
            }
            this->isMQAStudio_ = provenance > 8;

            // We are done return true
            return true;
        } else
        if (buffer1 == 0xbe0498c88) {        // MQA magic word
            this->isMQA_ = true;
            // Get Original Sample Rate
            uint8_t orsf = 0;
            for (auto m = 3u; m < 7; m++) { // TODO: this need fix (orsf is 5bits)
                auto cur = *(&s + m);
                auto j = ((static_cast<uint32_t>(cur[0]) ^ static_cast<uint32_t>(cur[1])) >> pos + 1) & 1u;
                orsf |= j << (6u - m);
            }
            this->decoder.original_sample_rate = OriginalSampleRateDecoder(orsf);

            // Get MQA Studio
            uint8_t provenance = 0u;
            for (auto m = 29u; m < 34; m++) {
                auto cur = *(&s + m);
                auto j = ((static_cast<uint32_t>(cur[0]) ^ static_cast<uint32_t>(cur[1])) >> pos + 1) & 1u;
                provenance |= j << (33u - m);
            }
            this->isMQAStudio_ = provenance > 8;

            // We are done return true
            return true;
        } else
        if (buffer2 == 0xbe0498c88) {        // MQA magic word
            this->isMQA_ = true;
            // Get Original Sample Rate
            uint8_t orsf = 0;
            for (auto m = 3u; m < 7; m++) { // TODO: this need fix (orsf is 5bits)
                auto cur = *(&s + m);
                auto j = ((static_cast<uint32_t>(cur[0]) ^ static_cast<uint32_t>(cur[1])) >> pos + 2) & 1u;
                orsf |= j << (6u - m);
            }
            this->decoder.original_sample_rate = OriginalSampleRateDecoder(orsf);

            // Get MQA Studio
            uint8_t provenance = 0u;
            for (auto m = 29u; m < 34; m++) {
                auto cur = *(&s + m);
                auto j = ((static_cast<uint32_t>(cur[0]) ^ static_cast<uint32_t>(cur[1])) >> pos + 2) & 1u;
                provenance |= j << (33u - m);
            }
            this->isMQAStudio_ = provenance > 8;

            // We are done return true
            return true;

        } else
        buffer = (buffer << 1u) & 0xFFFFFFFFFu;
        buffer1 = (buffer1 << 1u) & 0xFFFFFFFFFu;
        buffer2 = (buffer2 << 1u) & 0xFFFFFFFFFu;
    }
    return false;
}


std::string MQA_identifier::getMQA_encoder() const noexcept {
    return this->decoder.mqa_encoder;
}


uint32_t MQA_identifier::originalSampleRate() const noexcept {
    return this->decoder.original_sample_rate;
}


bool MQA_identifier::isMQA() const noexcept {
    return this->isMQA_;
}


bool MQA_identifier::isMQAStudio() const noexcept {
    return this->isMQAStudio_;
}


std::string MQA_identifier::filename() const noexcept {
    return this->file_;
}
