#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// http://blog.acipo.com/generating-wave-files-in-c/
//
/*
Compiling
To compile the above example, run:

gcc example.c -lm
You need the -lm to (-l) link to the (m) math library. Otherwise the cos function will not work.
*/

// ------------------------------------------------- [ Section: Wave Header ] -
typedef struct WaveHeader {
    // Riff Wave Header
    char chunkId[4];
    int  chunkSize;
    char format[4];

    // Format Subchunk
    char subChunk1Id[4];
    int  subChunk1Size;
    short int audioFormat;
    short int numChannels;
    int sampleRate;
    int byteRate;
    short int blockAlign;
    short int bitsPerSample;
    //short int extraParamSize;

    // Data Subchunk
    char subChunk2Id[4];
    int  subChunk2Size;

} WaveHeader;

typedef struct Wave {
    WaveHeader header;
    char* data;
    long long int index;
    long long int size;
    long long int nSamples;
} Wave;

// -------------------------------------------------- [ Section: Endianness ] -
int isBigEndian() ;
void reverseEndianness(const long long int size, void* value);
void toBigEndian(const long long int size, void* value);
void toLittleEndian(const long long int size, void* value);
Wave makeWave(int const sampleRate, short int const numChannels, short int const bitsPerSample);
void waveDestroy( Wave* wave );
void waveSetDuration( Wave* wave, const float seconds );
void waveAddSample( Wave* wave, const float* samples );
void waveToFile( Wave* wave, const char* filename );
