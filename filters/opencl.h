#ifndef OPENCL_H
#define OPENCL_H

bool initOpenCL();
void cleanupOpenCL();

unsigned char* getInversionOCL(const unsigned char* data, int width, int height, int channels);
unsigned char* getMedianOCL(const unsigned char* data, int width, int height, int channels);
unsigned char* getEdgesOCL(const unsigned char* data, int width, int height, int channels);

#endif
