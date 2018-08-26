# wav2mp3

## Usage
```
wav2mp3 [path]  
```
This converts all valid and supported WAV files in [path] to MP3 files, which are stored next to the source WAV files.

## Build
Run **make** to build wav2mp3 on Linux, run **mingw32-make** on Windows.

## Implementation

#### Reading WAV files
This is taken care of in *wav.cpp*. RIFF subchunks are read until the "fmt" and "data" chunks have been found. Essential information as well as the data chunk are stored in a wav_t struct.

#### Converting to mp3
The "convert" routine in *convert.cpp* deals with that part, by first decoding / padding the data chunk to render it digestible for the "lame_encode_buffer_..." routines which are then called. u-law and A-law decoders are implemented in *convert.cpp*.

#### Endianness and padding
The above 2 files utilize the routines implemented in *memory_layout.cpp* to pad data and correct for a possible endian mismatch between the host and the little endian byte order in WAV files. This generally does nothing, since the common Intel and AMD CPUs are all little endian.
*uint_helper.h* helps out by providing a simple way of getting an equally sized uint type for any given type.

#### Multithreading
*pthread_raii.h* implements RAII wrappers for pthreads and pthread mutexes and a lock_guard analogue for the latter.

#### Directory traversal
This is dealt with in *util.cpp*, which provides platform dependent code for Linux and Windows.

#### Exception handling
Unsupported WAV files lead to runtime_error exceptions, which are caught in the worker threads in *main.cpp*.
