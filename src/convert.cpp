#include "lame.h"
#include "memory_layout.h"
#include "wav.h"
#include <cstdio>
#include <fstream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>

using std::fclose;
using std::FILE;
using std::fopen;
using std::numeric_limits;
using std::ofstream;
using std::runtime_error;
using std::string;
using std::unique_ptr;

using namespace wav;

void center_unsigned_pcm(wav_t &wav) {
  int samples_total = wav.num_samples * wav.num_channels;
  int8_t *buf = (int8_t *)wav.data.get();
  uint8_t *ubuf = wav.data.get();
  for (int i = 0; i < samples_total; ++i)
    buf[i] = (ubuf[i] - (1 << 7));
}

void pad_pcm(wav_t &wav) {
  int block_sz_in = wav.block_sz;
  int block_sz_out = wav.block_sz < sizeof(short) ? sizeof(short) : sizeof(int);
  int num_blocks = wav.num_samples * wav.num_channels;
  unique_ptr<uint8_t[]> input(std::move(wav.data));
  wav.data.reset(new uint8_t[num_blocks * block_sz_out]);
  memory_layout::pad_le(wav.data.get(), input.get(), block_sz_in, block_sz_out,
                        num_blocks);
  wav.block_sz = block_sz_out;
}

void host_align_data(wav_t &wav) {
  int num_blocks = wav.num_samples * wav.num_channels;
  if (wav.block_sz == 1)
    return;
  else if (wav.block_sz == 2)
    memory_layout::le_to_host_arr<uint16_t>(wav.data.get(), num_blocks);
  else if (wav.block_sz == 4)
    memory_layout::le_to_host_arr<uint32_t>(wav.data.get(), num_blocks);
  else if (wav.block_sz == 8)
    memory_layout::le_to_host_arr<uint64_t>(wav.data.get(), num_blocks);
}

// https://en.wikipedia.org/wiki/G.711#A-Law
void decode_alaw(wav_t &wav) {
  int block_out = sizeof(short);
  int samples_total = wav.num_samples * wav.num_channels;
  unique_ptr<uint8_t[]> input(std::move(wav.data));
  wav.data.reset(new uint8_t[samples_total * block_out]);
  for (int i = 0; i < samples_total; ++i) {
    uint8_t *out = wav.data.get() + i * block_out;
    uint8_t *in = input.get() + i;
    short ix = (short)*in ^ (0x0055); // invert even bits
    short mantissa = ix & 0x000F;
    short exponent = (ix >> 4) & ~(1 << 3);
    mantissa += (exponent > 0) ? (1 << 4) : 0;
    mantissa = (mantissa << 4) + (0x0008);
    mantissa = (exponent > 0) ? mantissa << (exponent - 1) : mantissa;
    short sgn = 1 - 2 * (!!(ix & (1 << 7)));
    *(short *)out = sgn * mantissa;
  }
  wav.block_sz = block_out;
  wav.format_code = WAVE_FORMAT_PCM;
}

// https://en.wikipedia.org/wiki/G.711#%CE%BC-Law
void decode_ulaw(wav_t &wav) {
  int block_out = sizeof(short);
  int samples_total = wav.num_samples * wav.num_channels;
  unique_ptr<uint8_t[]> input(std::move(wav.data));
  wav.data.reset(new uint8_t[samples_total * block_out]);
  for (int i = 0; i < samples_total; ++i) {
    uint8_t *out = wav.data.get() + i * block_out;
    uint8_t *in = input.get() + i;
    short ix = (short)(*in ^ 0xFF); // invert all bits
    short mantissa = ix & 0x000F;
    short exponent = (ix >> 4) & ~(1 << 3);
    short sgn = 1 - 2 * ((ix & (1 << 7)) != 0);
    mantissa <<= (exponent + 1);
    *(short *)out =
        sgn * ((mantissa + (33 << exponent) - 33) << (block_out * 8 - 14));
  }
  wav.block_sz = block_out;
  wav.format_code = WAVE_FORMAT_PCM;
}

// check if format is supported, otherwise throw an exception
void check_support(const wav_t &wav) {
  if (wav.format_code == WAVE_FORMAT_PCM) {
    if (wav.block_sz > sizeof(int))
      throw runtime_error("Sample containers too big");
  } else if (wav.format_code == WAVE_FORMAT_IEEE_FLOAT) {
    if (wav.block_sz == sizeof(float)) {
      if (!numeric_limits<float>::is_iec559)
        throw runtime_error("Native float is not IEEE 754 compliant");
    } else if (wav.block_sz == sizeof(double)) {
      if (!numeric_limits<double>::is_iec559)
        throw runtime_error("Native double is not IEEE 754 compliant");
    } else
      throw runtime_error("Wrong sample width for float PCM");
  } else if (wav.format_code == WAVE_FORMAT_ALAW ||
             wav.format_code == WAVE_FORMAT_MULAW) {
    if (wav.block_sz > 1)
      throw runtime_error("Only 1 byte samples supported for A-law and u-law");
  } else
    throw runtime_error("Unsupported format");
}

void decode(wav_t &wav) {
  check_support(wav);
  if (wav.format_code == WAVE_FORMAT_PCM && wav.block_sz != sizeof(short) &&
      wav.block_sz != sizeof(int)) {
    // 8 bit means unsigned
    if (wav.block_sz == 1) {
      center_unsigned_pcm(wav);
    }
    pad_pcm(wav);
  } else if (wav.format_code == WAVE_FORMAT_ALAW) {
    decode_alaw(wav);
  } else if (wav.format_code == WAVE_FORMAT_MULAW) {
    decode_ulaw(wav);
  }
  if (wav.format_code == WAVE_FORMAT_PCM ||
      wav.format_code == WAVE_FORMAT_IEEE_FLOAT)
    host_align_data(wav);
}

// set lame flags in accordance with fmt
void set_lgf(lame_global_flags *lgf, const wav_t &wav) {
  lame_set_num_channels(lgf, wav.num_channels);
  lame_set_in_samplerate(lgf, wav.sample_rate);
  lame_set_mode(lgf, wav.num_channels == 1 ? MPEG_mode::MONO
                                           : MPEG_mode::JOINT_STEREO);

  // sane defaults for the rest
  lame_set_brate(lgf, 128);
  lame_set_quality(lgf, 2);
  // lame_set_bWriteVbrTag(lgf,0);
  if (lame_init_params(lgf) < 0)
    throw runtime_error("Initialization of lame flags failed");
}

namespace convert {

void convert(string filename_in, string filename_out) {
  wav_t wav;
  read_wav(filename_in, wav);
  decode(wav);

  if (wav.num_channels < 1 || wav.num_channels > 2)
    throw runtime_error("Unsupported number of channels");

  // raii wrapper for lame flags
  unique_ptr<lame_global_flags, decltype(&lame_close)> lgf(lame_init(),
                                                           &lame_close);
  set_lgf(lgf.get(), wav);

  int mp3buffer_size = (wav.num_samples * 5) / 4 + 7200;
  unique_ptr<uint8_t[]> mp3buffer(new uint8_t[mp3buffer_size]);

  int bytes_written = -1;

  // case 1: PCM integer data
  if (wav.format_code == WAVE_FORMAT_PCM ||
      wav.format_code == WAVE_FORMAT_ALAW) {
    if (wav.num_channels == 1) {
      if (wav.block_sz == sizeof(short))
        bytes_written = lame_encode_buffer(
            lgf.get(), (const short *)wav.data.get(), nullptr, wav.num_samples,
            mp3buffer.get(), mp3buffer_size);
      else if (wav.block_sz == sizeof(int))
        bytes_written = lame_encode_buffer_int(
            lgf.get(), (const int *)wav.data.get(), nullptr, wav.num_samples,
            mp3buffer.get(), mp3buffer_size);
    } else {
      if (wav.block_sz == sizeof(short))
        bytes_written = lame_encode_buffer_interleaved(
            lgf.get(), (short *)wav.data.get(), wav.num_samples,
            mp3buffer.get(), mp3buffer_size);
      else if (wav.block_sz == sizeof(int))
        bytes_written = lame_encode_buffer_interleaved_int(
            lgf.get(), (const int *)wav.data.get(), wav.num_samples,
            mp3buffer.get(), mp3buffer_size);
    }
  }
  // case 2: PCM IEEE float data
  else if (wav.format_code == WAVE_FORMAT_IEEE_FLOAT) {
    if (wav.num_channels == 1) {
      if (wav.block_sz == sizeof(float))
        bytes_written = lame_encode_buffer_ieee_float(
            lgf.get(), (const float *)wav.data.get(), nullptr, wav.num_samples,
            mp3buffer.get(), mp3buffer_size);
      else if (wav.block_sz == sizeof(double))
        bytes_written = lame_encode_buffer_ieee_double(
            lgf.get(), (const double *)wav.data.get(), nullptr, wav.num_samples,
            mp3buffer.get(), mp3buffer_size);
    } else {
      if (wav.block_sz == sizeof(float))
        bytes_written = lame_encode_buffer_interleaved_ieee_float(
            lgf.get(), (const float *)wav.data.get(), wav.num_samples,
            mp3buffer.get(), mp3buffer_size);
      else if (wav.block_sz == sizeof(double))
        bytes_written = lame_encode_buffer_interleaved_ieee_double(
            lgf.get(), (const double *)wav.data.get(), wav.num_samples,
            mp3buffer.get(), mp3buffer_size);
    }
  }

  if (bytes_written < 0)
    throw runtime_error("Conversion didn't work");

  ofstream file_out(filename_out, std::ios::binary);
  file_out.write((const char *)mp3buffer.get(), bytes_written);

  bytes_written = lame_encode_flush(lgf.get(), mp3buffer.get(), mp3buffer_size);
  file_out.write((const char *)mp3buffer.get(), bytes_written);

  file_out.close();

  auto closer = [](FILE *f) {
    if (f)
      fclose(f);
  };
  unique_ptr<FILE, decltype(closer)> file_out_2(
      fopen(filename_out.c_str(), "rb+"), closer);
  lame_mp3_tags_fid(lgf.get(), file_out_2.get());
}
} // namespace convert
