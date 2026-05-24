__kernel void inversion_kernel(__global const uchar* input, __global uchar* output, int channels) {
    int i = get_global_id(0);
    int size = get_global_size(0);

    if (channels == 4 && (i % 4 == 3)) {
        output[i] = input[i];
    } else {
        output[i] = 255 - input[i];
    }
}

__kernel void median_kernel(__global const uchar* input, __global uchar* output, int width, int height, int channels) {
    int x = get_global_id(0);
    int y = get_global_id(1);

    if (x < 1 || x >= width - 1 || y < 1 || y >= height - 1) return;

    for (int c = 0; c < 3; c++) {
        uchar window[9];
        int k = 0;
        for (int ky = -1; ky <= 1; ky++) {
            for (int kx = -1; kx <= 1; kx++) {
                window[k++] = input[((y + ky) * width + (x + kx)) * channels + c];
            }
        }

        for (int i = 0; i < 9; i++) {
            for (int j = i + 1; j < 9; j++) {
                if (window[i] > window[j]) {
                    uchar tmp = window[i];
                    window[i] = window[j];
                    window[j] = tmp;
                }
            }
        }
        output[(y * width + x) * channels + c] = window[4];
    }

    if (channels == 4) {
        output[(y * width + x) * channels + 3] = input[(y * width + x) * channels + 3];
    }
}

__kernel void edges_kernel(__global const uchar* input, __global uchar* output, int width, int height, int channels) {
    int x = get_global_id(0);
    int y = get_global_id(1);

    if (x < 1 || x >= width - 1 || y < 1 || y >= height - 1) return;

    float Gx[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
    float Gy[9] = {1, 2, 1, 0, 0, 0, -1, -2, -1};

    for (int c = 0; c < 3; c++) {
        float sumX = 0, sumY = 0;
        int k = 0;
        for (int ky = -1; ky <= 1; ky++) {
            for (int kx = -1; kx <= 1; kx++) {
                float val = (float)input[((y + ky) * width + (x + kx)) * channels + c];
                sumX += val * Gx[k];
                sumY += val * Gy[k];
                k++;
            }
        }
        float total = sqrt(sumX * sumX + sumY * sumY);
        output[(y * width + x) * channels + c] = (uchar)(total > 255.0f ? 255.0f : total);
    }

    if (channels == 4) {
        output[(y * width + x) * channels + 3] = input[(y * width + x) * channels + 3];
    }
}
