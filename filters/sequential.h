#ifndef SEQUENTIAL_H
#define SEQUENTIAL_H

unsigned char* getInversionSeq(const unsigned char* data, int width, int height, int channels);

unsigned char* getMedianSeq(const unsigned char* data, int width, int height, int channels);

unsigned char* getEdgesSeq(const unsigned char* data, int width, int height, int channels);
#endif
