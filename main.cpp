#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <chrono>

#include "filters/sequential.h"
#include "filters/openmp.h"
#include "filters/simd.h"
#include "filters/opencl.h"

using namespace std;

void createResultDir() {
#ifdef _WIN32
    _mkdir("result");
#else
    mkdir("result", 0777);
#endif
}

int main() {
    createResultDir();
    
    if (!initOpenCL()) {
        cout << "[!] Видеокарта не поддерживает OpenCL или ошибка инициализации." << endl;
    }

    vector<string> fileNames = {
        "300x300.png",
        "400x400.png",
        "500x500.png",
        "600x600.png",
        "950x950.png",
        "2400x2400.png"
    };

    cout << "--- ВЫБОР КАРТИНКИ ---" << endl;
    for (int i = 0; i < (int)fileNames.size(); i++) cout << i + 1 << ". " << fileNames[i] << endl;
    int imgChoice; cin >> imgChoice;
    if (imgChoice < 1 || imgChoice > (int)fileNames.size()) return 1;
    string fileName = fileNames[imgChoice - 1];

    cout << "\n--- ВЫБОР ФИЛЬТРА ---" << endl;
    cout << "1. Инверсия\n2. Медианный фильтр\n3. Обнаружение границ" << endl;
    int filterChoice; cin >> filterChoice;

    cout << "\n--- ВЫБОР МЕТОДА ---" << endl;
    cout << "1. Последовательно\n2. OpenMP\n3. SIMD\n4. OpenCL" << endl;
    int methodChoice; cin >> methodChoice;

    int width, height, channels;
    string filePath = "Картинки/Исходные/" + fileName;
    unsigned char* image = stbi_load(filePath.c_str(), &width, &height, &channels, 0);

    if (!image) { cout << "Ошибка загрузки!" << endl; return 1; }

    unsigned char* resultImg = nullptr;
    string filterName;
    string methodName;

    auto start = chrono::high_resolution_clock::now();

    if (methodChoice == 1) { 
        methodName = "Seq";
        if (filterChoice == 1) { resultImg = getInversionSeq(image, width, height, channels); filterName = "inversion"; }
        else if (filterChoice == 2) { resultImg = getMedianSeq(image, width, height, channels); filterName = "median"; }
        else if (filterChoice == 3) { resultImg = getEdgesSeq(image, width, height, channels); filterName = "edges"; }
    } 
    else if (methodChoice == 2) { 
        methodName = "OMP";
        if (filterChoice == 1) { resultImg = getInversionOMP(image, width, height, channels); filterName = "inversion"; }
        else if (filterChoice == 2) { resultImg = getMedianOMP(image, width, height, channels); filterName = "median"; }
        else if (filterChoice == 3) { resultImg = getEdgesOMP(image, width, height, channels); filterName = "edges"; }
    }
    else if (methodChoice == 3) { 
        methodName = "SIMD";
        if (filterChoice == 1) { resultImg = getInversionSIMD(image, width, height, channels); filterName = "inversion"; }
        else if (filterChoice == 2) { resultImg = getMedianSIMD(image, width, height, channels); filterName = "median"; }
        else if (filterChoice == 3) { resultImg = getEdgesSIMD(image, width, height, channels); filterName = "edges"; }
    }
    else if (methodChoice == 4) { 
        methodName = "OCL";
        if (filterChoice == 1) { resultImg = getInversionOCL(image, width, height, channels); filterName = "inversion"; }
        else if (filterChoice == 2) { resultImg = getMedianOCL(image, width, height, channels); filterName = "median"; }
        else if (filterChoice == 3) { resultImg = getEdgesOCL(image, width, height, channels); filterName = "edges"; }
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> duration = end - start;

    if (resultImg) {
        cout << "\nМетод: " << methodName << endl;
        cout << "Время обработки: " << duration.count() << " ms" << endl;
        string outPath = "result/" + filterName + "_" + methodName + "_" + fileName;
        stbi_write_png(outPath.c_str(), width, height, channels, resultImg, width * channels);
        cout << "Результат: " << outPath << endl;
        delete[] resultImg;
    }

    stbi_image_free(image);
    cleanupOpenCL();
    return 0;
}
