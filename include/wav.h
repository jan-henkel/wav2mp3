#ifndef WAV_H
#define WAV_H

#include <memory>
#include <string>
#include <cstdint>

namespace wav {

  const uint32_t RIFF_ID=*(uint32_t*)"RIFF";
  const uint32_t WAV_FORMAT_ID=*(uint32_t*)"WAVE";
  const uint32_t FMT_ID=*(uint32_t*)"fmt ";
  const uint32_t DATA_ID=*(uint32_t*)"data";
  const uint16_t WAVE_FORMAT_PCM=0x0001;
  const uint16_t WAVE_FORMAT_IEEE_FLOAT=0x0003;
  const uint16_t WAVE_FORMAT_ALAW=0x0006;
  const uint16_t WAVE_FORMAT_MULAW=0x0007;
  const uint16_t WAVE_FORMAT_EXTENSIBLE=0xFFFE;

  struct wav_t {
    unsigned block_sz;
    unsigned sample_rate;
    unsigned num_channels;
    uint32_t format_code;
    std::unique_ptr<uint8_t[]> data;
    unsigned num_samples;
  };

  void read_wav(std::string filename, wav_t& wav);
  
}
#endif
