#include "sequential.h"
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;

unsigned char* getInversionSeq(const unsigned char* data, int width, int height, int channels) {
    int size = width * height * channels;
    unsigned char* result = new unsigned char[size];
    for (int i = 0; i < size; i++) {
        if (channels == 4 && (i % 4 == 3)) {
            result[i] = data[i];
        } else {
            result[i] = 255 - data[i];
        }
    }
    return result;
}

unsigned char* getMedianSeq(const unsigned char* data, int width, int height, int channels) {
    unsigned char* result = new unsigned char[width * height * channels];

    for (int i = 0; i < width * height * channels; i++) result[i] = 0;

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            for (int color = 0; color < 3; color++) {
                vector<unsigned char> window;
                window.reserve(9);

                for (int ky = -1; ky <= 1; ky++) {
                    for (int kx = -1; kx <= 1; kx++) {
                        int neighbourX = x + kx;
                        int neighbourY = y + ky;
                        int index = (neighbourY * width + neighbourX) * channels + color;
                        window.push_back(data[index]);
                    }
                }

                sort(window.begin(), window.end());

                int resultIndex = (y * width + x) * channels + color;
                result[resultIndex] = window[4];
            }

            if (channels == 4) {
                int alphaIndex = (y * width + x) * channels + 3;
                result[alphaIndex] = data[alphaIndex];
            }
        }
    }

    return result;
}

unsigned char* getEdgesSeq(const unsigned char* data, int width, int height, int channels) {
    unsigned char* result = new unsigned char[width * height * channels];
    for (int i = 0; i < width * height * channels; i++) result[i] = 0;

    int Gx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1},
    };

    int Gy[3][3] = {
       {1, 2, 1},
        {0,0,0},
       {-1, -2, -1},
    };
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            for (int color = 0; color < 3; color++) {
                float sumX = 0, sumY = 0;

                for (int ky = -1; ky <= 1; ky++) {
                    for (int kx = -1; kx <= 1; kx++) {
                        int neighbourX = x + kx;
                        int neighbourY = y + ky;
                        int index = (neighbourY * width + neighbourX) * channels + color;
                        unsigned char val = data[index];
                        sumX += val * Gx[ky+1][kx+1];
                        sumY += val * Gy[ky+1][kx+1];

                    }
                }

                int total = sqrt(sumX * sumX + sumY * sumY);
                if (total > 255) total = 255;

                result[(y * width + x) * channels + color] = total;
            }
            if (channels == 4) {
                int alphaIndex = (y * width + x) * channels + 3;
                result[alphaIndex] = data[alphaIndex];
                }
        }
    }

    return result;
}
