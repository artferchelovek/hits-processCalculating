#include "opencl.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

using namespace std;

void checkError(cl_int err, const char* operation) {
    if (err != CL_SUCCESS) {
        cerr << "Ошибка OpenCL во время " << operation << ": " << err << endl;
        exit(1);
    }
}

string readKernelSource(const string& filePath) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Не удалось открыть файл кернела: " << filePath << endl;
        return "";
    }
    stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

unsigned char* runOCLFilter(const string& kernelName, const unsigned char* data, int width, int height, int channels) {
    int size = width * height * channels;
    unsigned char* result = new unsigned char[size];
    cl_int err;

    cl_platform_id platform;
    checkError(clGetPlatformIDs(1, &platform, NULL), "clGetPlatformIDs");

    cl_device_id device;
    checkError(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL), "clGetDeviceIDs");

    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    checkError(err, "clCreateContext");

    cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err);
    checkError(err, "clCreateCommandQueue");

    string source = readKernelSource("filters/kernels.cl");
    const char* sourcePtr = source.c_str();
    cl_program program = clCreateProgramWithSource(context, 1, &sourcePtr, NULL, &err);
    checkError(err, "clCreateProgramWithSource");

    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        char log[4096];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(log), log, NULL);
        cerr << "Ошибка компиляции кернела:\n" << log << endl;
        exit(1);
    }

    cl_kernel kernel = clCreateKernel(program, kernelName.c_str(), &err);
    checkError(err, "clCreateKernel");

    cl_mem inputBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, size, (void*)data, &err);
    checkError(err, "clCreateBuffer (input)");

    cl_mem outputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, size, NULL, &err);
    checkError(err, "clCreateBuffer (output)");

    if (kernelName == "inversion_kernel") {
        clSetKernelArg(kernel, 0, sizeof(cl_mem), &inputBuffer);
        clSetKernelArg(kernel, 1, sizeof(cl_mem), &outputBuffer);
        clSetKernelArg(kernel, 2, sizeof(int), &channels);

        size_t globalSize = size;
        checkError(clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalSize, NULL, 0, NULL, NULL), "clEnqueueNDRangeKernel");
    } else {
        clSetKernelArg(kernel, 0, sizeof(cl_mem), &inputBuffer);
        clSetKernelArg(kernel, 1, sizeof(cl_mem), &outputBuffer);
        clSetKernelArg(kernel, 2, sizeof(int), &width);
        clSetKernelArg(kernel, 3, sizeof(int), &height);
        clSetKernelArg(kernel, 4, sizeof(int), &channels);

        size_t globalSize[2] = {(size_t)width, (size_t)height};
        checkError(clEnqueueNDRangeKernel(queue, kernel, 2, NULL, globalSize, NULL, 0, NULL, NULL), "clEnqueueNDRangeKernel");
    }

    checkError(clEnqueueReadBuffer(queue, outputBuffer, CL_TRUE, 0, size, result, 0, NULL, NULL), "clEnqueueReadBuffer");

    clReleaseMemObject(inputBuffer);
    clReleaseMemObject(outputBuffer);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return result;
}

unsigned char* getInversionOCL(const unsigned char* data, int width, int height, int channels) {
    return runOCLFilter("inversion_kernel", data, width, height, channels);
}

unsigned char* getMedianOCL(const unsigned char* data, int width, int height, int channels) {
    return runOCLFilter("median_kernel", data, width, height, channels);
}

unsigned char* getEdgesOCL(const unsigned char* data, int width, int height, int channels) {
    return runOCLFilter("edges_kernel", data, width, height, channels);
}
