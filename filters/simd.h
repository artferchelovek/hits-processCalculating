#ifndef SIMD_H
#define SIMD_H

unsigned char* getInversionSIMD(const unsigned char* data, int width, int height, int channels);

unsigned char* getMedianSIMD(const unsigned char* data, int width, int height, int channels);

unsigned char* getEdgesSIMD(const unsigned char* data, int width, int height, int channels);
#endif
