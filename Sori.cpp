#include "Sori.h"

#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>

using namespace std;

using sori = Sori::Sori;
using namespace Sori;

constexpr double PI = 3.14159265358979;

#ifdef WAVE_BIG_ENDIAN_SYSTEM
uint16_t sori::change_endian_u16(uint16_t x)
{
    uint8_t a = x & 0xff;
    uint8_t b = (x >> 8) & 0xff;
    
    return (a<<8) | b;
}

int16_t sori::change_endian_16(int16_t x)
{
    uint8_t a = x & 0xff;
    uint8_t b = (x >> 8) & 0xff;
    
    return (a<<8) | b;
}

uint32_t sori::change_endian_u32(uint32_t x)
{
    uint8_t a = x & 0xff;
    uint8_t b = (x >> 8) & 0xff;
    uint8_t c = (x >> 16) & 0xff;
    uint8_t d = (x >> 24) & 0xff;
    
    return (a<<24) | (b<<16) | (c<<8) | d;
}
#else
uint16_t sori::change_endian_u16(uint16_t x)
{
    return x;
}

int16_t sori::change_endian_16(int16_t x)
{
    return x;
}

uint32_t sori::change_endian_u32(uint32_t x)
{
    return x;
}
#endif



sori::Sori(int num_channels, int bytes_per_sample, int sample_rate) :
    num_channels(num_channels),
    bytes_per_sample(bytes_per_sample),
    sample_rate(sample_rate)
{
}

bool sori::open(istream& fin)
{
    WAVEHeader header;
    
    fin.read((char*)&header, sizeof(header));
    
    if(fin.eof())
        return false;
    
    num_channels = change_endian_u16(header.fmt.num_channels);
    bytes_per_sample = change_endian_u16(header.fmt.bit_per_sample) / 8;
    sample_rate = change_endian_u32(header.fmt.sample_rate);
    
    if(!check_header(header))
        return false;
    
    if(!read_data(fin, header, change_endian_u16(header.fmt.num_channels)))
        return false;
    
    return true;
}

bool sori::open(const string& filename)
{
    ifstream fin(filename, ios::binary);
    
    if(!fin)
        return false;
    
    return open(fin);
}

bool sori::check_header(WAVEHeader& header)
{
    if(!(header.riff.chunk_id[0] == 'R' && header.riff.chunk_id[1] == 'I' &&
         header.riff.chunk_id[2] == 'F' && header.riff.chunk_id[3] == 'F'))
        return false;
    
    if(!(header.fmt.chunk_id[0] == 'f' && header.fmt.chunk_id[1] == 'm' &&
         header.fmt.chunk_id[2] == 't' && header.fmt.chunk_id[3] == ' '))
        return false;
    
    // some wave file doesn't have chunk id 'data'
    /*if(!(header.data.chunk_id[0] == 'd' && header.data.chunk_id[1] == 'a' &&
         header.data.chunk_id[2] == 't' && header.data.chunk_id[3] == 'a'))
        return false;*/
    
    return true;
}

bool sori::read_data(istream& fin, WAVEHeader& header, int num_channels)
{
    int channel_cnt = CHANNEL_COUNT[num_channels];
    
    int len = change_endian_u32(header.riff.chunk_size) - sizeof(WAVEHeader);
    int bytes_per_sample = change_endian_u16(header.fmt.bit_per_sample) / 8;
    
    // This code only works in 16-bit wave file.
    int16_t data[6];
    
    for(int i=0; i<len; i += channel_cnt * bytes_per_sample)
    {
        fin.read((char*)data, bytes_per_sample * channel_cnt);
        if(fin.eof())
            return false;
        
        for(int j=0; j<channel_cnt; j++)
        {
            data[j] = change_endian_16(data[j]);
            channels[j].push_back(data[j]);
        }
    }
    
    return true;
}

bool sori::write(const string& filename)
{
    WAVEHeader header;
    
    ofstream fout(filename);
    
    if(!fout)
        return false;
    
    int channel_cnt = CHANNEL_COUNT[num_channels];
    
    int len = channels[0].size() * channel_cnt * bytes_per_sample;
    
    memcpy(header.riff.chunk_id, "RIFF", 4);
    header.riff.chunk_size = change_endian_u32(len + sizeof(WAVEHeader) - 8);
    memcpy(header.riff.format, "WAVE", 4);
    
    memcpy(header.fmt.chunk_id, "fmt ", 4);
    header.fmt.chunk_size = change_endian_u32(16);
    header.fmt.audio_format = change_endian_u16(1); // PCM: 1
    header.fmt.num_channels = change_endian_u16(num_channels);
    header.fmt.sample_rate = change_endian_u32(sample_rate);
    header.fmt.avg_byte_rate = change_endian_u32(sample_rate * channel_cnt * bytes_per_sample);
    header.fmt.block_align = change_endian_u16(channel_cnt * bytes_per_sample);
    header.fmt.bit_per_sample = change_endian_u16(bytes_per_sample * 8);
    
    memcpy(header.data.chunk_id, "data", 4);
    header.data.chunk_size = change_endian_u32(len);
    
    fout.write((char*)&header, sizeof(header));
    
    int16_t data[6];
    
    int idx = 0;
    for(int i=0; i<len; i += channel_cnt * bytes_per_sample)
    {
        for(int j=0; j<channel_cnt; j++)
        {
            data[j] = change_endian_16(channels[j][idx]);
        }
        
        fout.write((char*)data, bytes_per_sample * channel_cnt);
        idx++;
    }
    
    return true;
}

int sori::get(int channel, double idx)
{
    if(idx >= channels[channel].size())
        return 0;
    else if(idx < 0)
        return 0;
    
    if(channels[channel].size() - 1 <= idx)
        return channels[channel].back();
    
    int idx_ = idx;
    
    int y1 = channels[channel][idx_];
    int y2 = channels[channel][idx_+1];
    int x1 = idx_;
    double a = (double)(y2 - y1) / 2;
    
    // y = -a cos pi (x - x1) + (y1+a)
    
    return -a * cos(PI * (idx - x1)) + (y1 + a);
    //return y1;
}
