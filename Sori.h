#pragma once

// Sori - A simple WAV file parser written in C++.
// Written by Donghyun Lee

#include <string>
#include <vector>

//---------------------------------------------------------------
//
// If your system is big endian, then uncomment macro below
// #define WAVE_BIG_ENDIAN_SYSTEM
//
//---------------------------------------------------------------




#ifdef _MSC_VER
#define PACK( struct_ ) __pragma( pack(push, 1) ) struct_ __pragma( pack(pop))
#else
#define PACK( struct_ ) struct_ __attribute__((__packed__))
#endif

namespace Sori
{
    //-------------------------------------------
    // [Channel]
    // - Stereo    : [left][right]
    // - 3 channel : [left][right][center]
    // - Quad      : [front left][front right][rear left][rear right]
    // - 4 channel : [left][center][right][surround]
    // - 6 channel : [left center][left][center][right center][right][surround]
    //-------------------------------------------

    // Number of channel according to FMTHeader::NumChannels
    const int CHANNEL_COUNT[] = {-1, 1, 2, 3, 4, 4, 6};

    enum WaveChannel
    {
        MONO = 1,
        STEREO = 2,
        THREE_CHANNEL = 3,
        QUAD = 4,
        FOUR_CHANNEL = 5,
        SIX_CHANNEL = 6
    };

    PACK(struct RIFFHeader
    {
        char chunk_id[4];
        uint32_t chunk_size;
        char format[4];
    });

    PACK(struct FMTHeader
    {
        char chunk_id[4];
        uint32_t chunk_size;
        uint16_t audio_format;
        uint16_t num_channels;
        uint32_t sample_rate;
        uint32_t avg_byte_rate;
        uint16_t block_align;
        uint16_t bit_per_sample;
    });

    PACK(struct DATAHeader
    {
        char chunk_id[4];
        uint32_t chunk_size;
    });

    PACK(struct WAVEHeader
    {
        RIFFHeader riff;
        FMTHeader fmt;
        DATAHeader data;
    });

    class Sori
    {
    private:
        uint16_t change_endian_u16(uint16_t x);
        int16_t change_endian_16(int16_t x);
        uint32_t change_endian_u32(uint32_t x);
        
    private:
        int num_channels;
        int bytes_per_sample;
        int sample_rate;
        
    public:
        int get_num_channels() { return num_channels; }
        int get_bytes_per_sample() { return bytes_per_sample; }
        int get_sample_rate() { return sample_rate; }
        
        // change second to wave index
        int to_index(double second)
        {
            int idx = second * sample_rate;
            if(idx < channels[0].size())
                return idx;
            else
                return channels[0].size();
        }
        
    public:
        std::vector<int> channels[7];
        
    public:
        Sori(int num_channels = 1, int bytes_per_sample = 2, int sample_rate = 44100);
        
        bool open(std::istream& fin);
        bool open(const std::string& filename);
        
    private:
        bool check_header(WAVEHeader& header);
        bool read_data(std::istream& fin, WAVEHeader& header, int num_channels);
        
    public:
        bool write(const std::string& filename);
        
    public:
        int get(int channel, double idx);
    };
};
