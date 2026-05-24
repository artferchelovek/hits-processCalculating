#ifndef OPENMP_H
#define OPENMP_H

unsigned char* getInversionOMP(const unsigned char* data, int width, int height, int channels);

unsigned char* getMedianOMP(const unsigned char* data, int width, int height, int channels);

unsigned char* getEdgesOMP(const unsigned char* data, int width, int height, int channels);
#endif
