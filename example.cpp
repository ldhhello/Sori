#include <iostream>
#include <fstream>

#include "Sori.h"
using namespace std;

// Sori sample code #1
// This example swaps left channel and right channel of original wav file.

int main()
{
    Sori::Sori sori;
    if(!sori.open("/Users/donghyun/Documents/hololol.wav"))
    {
        cout << "Failed to open wav file.";
        return 0;
    }
    
    Sori::Sori sori2(2, 2, sori.get_sample_rate());
    
    for(int i=0; i<sori.channels[0].size(); i++)
    {
        int left = sori.get(0, i);
        int right = sori.get(1, i);

        sori2.channels[0].push_back(right);
        sori2.channels[1].push_back(left);
    }
    
    sori2.write("/Users/donghyun/Documents/hololol2.wav");
}