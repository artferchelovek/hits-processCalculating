#include "simd.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <arm_neon.h>
#include <omp.h>

using namespace std;

unsigned char* getInversionSIMD(const unsigned char* data, int width, int height, int channels) {
    int size = width * height * channels;
    unsigned char* result = new unsigned char[size];

    int i = 0;
    uint8x16_t v_255 = vdupq_n_u8(255);

    for (; i <= size - 16; i += 16) {
        uint8x16_t v_data = vld1q_u8(&data[i]);
        uint8x16_t v_res = vqsubq_u8(v_255, v_data);
        vst1q_u8(&result[i], v_res);
    }

    for (; i < size; i++) {
        if (channels == 4 && (i % 4 == 3)) {
            result[i] = data[i];
        } else {
            result[i] = 255 - data[i];
        }
    }

    if (channels == 4) {
        for (int j = 3; j < size; j += 4) {
            result[j] = data[j];
        }
    }

    return result;
}

unsigned char* getMedianSIMD(const unsigned char* data, int width, int height, int channels) {
    unsigned char* result = new unsigned char[width * height * channels];
    for (int i = 0; i < width * height * channels; i++) result[i] = 0;

    for (int y = 1; y < height - 1; y++) {
        #pragma omp simd
        for (int x = 1; x < width - 1; x++) {
            for (int color = 0; color < 3; color++) {
                unsigned char window[9];
                int k = 0;
                for (int ky = -1; ky <= 1; ky++) {
                    for (int kx = -1; kx <= 1; kx++) {
                        int index = ((y + ky) * width + (x + kx)) * channels + color;
                        window[k++] = data[index];
                    }
                }

                for (int i = 1; i < 9; i++) {
                    unsigned char key = window[i];
                    int j = i - 1;
                    while (j >= 0 && window[j] > key) {
                        window[j + 1] = window[j];
                        j = j - 1;
                    }
                    window[j + 1] = key;
                }

                result[(y * width + x) * channels + color] = window[4];
            }
            if (channels == 4) {
                result[(y * width + x) * channels + 3] = data[(y * width + x) * channels + 3];
            }
        }
    }
    return result;
}

unsigned char* getEdgesSIMD(const unsigned char* data, int width, int height, int channels) {
    unsigned char* result = new unsigned char[width * height * channels];
    for (int i = 0; i < width * height * channels; i++) result[i] = 0;

    int Gx[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    int Gy[3][3] = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};

    for (int y = 1; y < height - 1; y++) {
        #pragma omp simd
        for (int x = 1; x < width - 1; x++) {
            for (int color = 0; color < 3; color++) {
                float sumX = 0, sumY = 0;
                for (int ky = -1; ky <= 1; ky++) {
                    for (int kx = -1; kx <= 1; kx++) {
                        int neighborIndex = ((y + ky) * width + (x + kx)) * channels + color;
                        unsigned char val = data[neighborIndex];
                        sumX += val * Gx[ky+1][kx+1];
                        sumY += val * Gy[ky+1][kx+1];
                    }
                }
                int total = sqrt(sumX * sumX + sumY * sumY);
                if (total > 255) total = 255;
                result[(y * width + x) * channels + color] = (unsigned char)total;
            }
            if (channels == 4) {
                result[(y * width + x) * channels + 3] = data[(y * width + x) * channels + 3];
            }
        }
    }
    return result;
}
