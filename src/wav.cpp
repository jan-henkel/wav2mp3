#include "wav.h"
#include "memory_layout.h"
#include <fstream>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <utility>
#include <string>

using std::ifstream;
using std::runtime_error;
using std::unique_ptr;
using std::string;
using std::move;

using namespace wav;

struct riff_header {
  uint32_t id;
  uint32_t chunk_size;
  uint32_t format_id;
};

struct subchunk_header {
  uint32_t id;
  uint32_t chunk_size;
};

struct fmt_chunk {
  subchunk_header header;
  uint16_t format_code;
  uint16_t num_channels;
  uint32_t sample_rate;
  uint32_t byte_rate;
  uint16_t block_align;
  uint16_t bits_per_sample;
  uint16_t extension_size;
  uint16_t valid_bits_per_sample;
  uint16_t channel_mask;
  uint8_t guid[16];
};

struct data_chunk {
  subchunk_header header;
  std::unique_ptr<uint8_t[]> data;
};

void read_wave_riff_header(ifstream& file,riff_header& header) {
  file.read((char*)&header.id,4);
  file.read((char*)&header.chunk_size,4);
  file.read((char*)&header.format_id,4);
  memory_layout::les_to_host(header.id,header.chunk_size,header.format_id);
  if(header.id!=RIFF_ID || header.format_id!=WAV_FORMAT_ID) {
    throw runtime_error("Not a wave file");
  }
}

int read_subchunk_header(ifstream& file, subchunk_header& header) {
  file.read((char*)&header.id,4);
  file.read((char*)&header.chunk_size,4);
  memory_layout::les_to_host(header.id,header.chunk_size);
  return 8;
}

int read_fmt_chunk(ifstream& file, const subchunk_header& header, fmt_chunk& chunk) {
  chunk.header=header;
  
  file.read((char*)&chunk.format_code,2);
  file.read((char*)&chunk.num_channels,2);
  file.read((char*)&chunk.sample_rate,4);
  file.read((char*)&chunk.byte_rate,4);
  file.read((char*)&chunk.block_align,2);
  file.read((char*)&chunk.bits_per_sample,2);
  int result=16;
  memory_layout::les_to_host(chunk.format_code,chunk.num_channels,chunk.sample_rate,chunk.byte_rate,chunk.block_align,chunk.bits_per_sample);
  
  if(header.chunk_size==16)
    return result;
  
  file.read((char*)&chunk.extension_size, 2);
  memory_layout::le_to_host(chunk.extension_size);
  result+=2;
  
  if(chunk.extension_size!=22)
    return result;
  
  file.read((char*)&chunk.valid_bits_per_sample, 2);
  file.read((char*)&chunk.channel_mask, 4);
  file.read((char*)&chunk.guid, 16);
  memory_layout::les_to_host(chunk.valid_bits_per_sample,chunk.channel_mask);
  result+=22;

  return result;
}

int read_data_chunk(ifstream& file, const subchunk_header& header, data_chunk& chunk) {
  chunk.header=header;
  chunk.data.reset(new uint8_t[header.chunk_size]);
  file.read((char*)chunk.data.get(),header.chunk_size);
  return header.chunk_size;
}

int ignore_chunk(ifstream& file, const subchunk_header& header) {
  file.ignore(header.chunk_size);
  return header.chunk_size;
}

namespace wav {
  void read_wav(string filename, wav_t& wav) {
    ifstream file(filename,std::ios::binary);
    riff_header riff_hdr;
    read_wave_riff_header(file, riff_hdr);

    //remaining bytes
    int remaining=riff_hdr.chunk_size-4;

    subchunk_header sub_hdr;
    fmt_chunk fmt;
    data_chunk data;
    bool fmt_found=false;
    bool data_found=false;

    while(remaining>0 && (!fmt_found || !data_found)) {
      remaining-=read_subchunk_header(file, sub_hdr);
      if(sub_hdr.id==FMT_ID && !fmt_found) {
	fmt_found=true;
	remaining-=read_fmt_chunk(file, sub_hdr, fmt);
      }
      else if(sub_hdr.id==DATA_ID && !data_found) {
	data_found=true;
	remaining-=read_data_chunk(file,sub_hdr,data);
      }
      else {
	remaining-=ignore_chunk(file,sub_hdr);
      }
    }
  
    if(remaining<0 || !fmt_found || !data_found)
      throw runtime_error("Malformed file");

    wav.block_sz=fmt.bits_per_sample/8;
    wav.sample_rate=fmt.sample_rate;
    wav.num_channels=fmt.num_channels;
    if(fmt.format_code!=WAVE_FORMAT_EXTENSIBLE)
      wav.format_code=fmt.format_code;
    else
      wav.format_code=fmt.guid[0];
  
    wav.num_samples=data.header.chunk_size/(wav.block_sz*wav.num_channels);
    wav.data=move(data.data);
  }
}
