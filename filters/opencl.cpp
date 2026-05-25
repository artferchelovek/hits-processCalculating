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

static cl_platform_id ocl_platform;
static cl_device_id ocl_device;
static cl_context ocl_context = nullptr;
static cl_command_queue ocl_queue = nullptr;
static cl_program ocl_program = nullptr;

static void checkError(cl_int err, const char* operation) {
    if (err != CL_SUCCESS) {
        cerr << "Ошибка OpenCL во время " << operation << ": " << err << endl;
        exit(1);
    }
}

static string readKernelSource(const string& filePath) {
    ifstream file(filePath);
    if (!file.is_open()) return "";
    stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

bool initOpenCL() {
    cl_int err;
    err = clGetPlatformIDs(1, &ocl_platform, NULL);
    if (err != CL_SUCCESS) return false;

    err = clGetDeviceIDs(ocl_platform, CL_DEVICE_TYPE_GPU, 1, &ocl_device, NULL);
    if (err != CL_SUCCESS) return false;

    ocl_context = clCreateContext(NULL, 1, &ocl_device, NULL, NULL, &err);
    checkError(err, "clCreateContext");

    ocl_queue = clCreateCommandQueue(ocl_context, ocl_device, 0, &err);
    checkError(err, "clCreateCommandQueue");

    string source = readKernelSource("filters/kernels.cl");
    const char* sourcePtr = source.c_str();
    ocl_program = clCreateProgramWithSource(ocl_context, 1, &sourcePtr, NULL, &err);
    checkError(err, "clCreateProgramWithSource");

    err = clBuildProgram(ocl_program, 1, &ocl_device, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        char log[4096];
        clGetProgramBuildInfo(ocl_program, ocl_device, CL_PROGRAM_BUILD_LOG, sizeof(log), log, NULL);
        cerr << "Ошибка компиляции кернела:\n" << log << endl;
        return false;
    }
    return true;
}

void cleanupOpenCL() {
    if (ocl_program) clReleaseProgram(ocl_program);
    if (ocl_queue) clReleaseCommandQueue(ocl_queue);
    if (ocl_context) clReleaseContext(ocl_context);
}

static unsigned char* runOCLFilter(const string& kernelName, const unsigned char* data, int width, int height, int channels) {
    int size = width * height * channels;
    unsigned char* result = new unsigned char[size];
    cl_int err;

    cl_kernel kernel = clCreateKernel(ocl_program, kernelName.c_str(), &err);
    checkError(err, "clCreateKernel");

    cl_mem inputBuffer = clCreateBuffer(ocl_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, size, (void*)data, &err);
    checkError(err, "clCreateBuffer (input)");

    cl_mem outputBuffer = clCreateBuffer(ocl_context, CL_MEM_WRITE_ONLY, size, NULL, &err);
    checkError(err, "clCreateBuffer (output)");

    if (kernelName == "inversion_kernel") {
        clSetKernelArg(kernel, 0, sizeof(cl_mem), &inputBuffer);
        clSetKernelArg(kernel, 1, sizeof(cl_mem), &outputBuffer);
        clSetKernelArg(kernel, 2, sizeof(int), &channels);
        size_t globalSize = size;
        checkError(clEnqueueNDRangeKernel(ocl_queue, kernel, 1, NULL, &globalSize, NULL, 0, NULL, NULL), "clEnqueueNDRangeKernel");
    } else {
        clSetKernelArg(kernel, 0, sizeof(cl_mem), &inputBuffer);
        clSetKernelArg(kernel, 1, sizeof(cl_mem), &outputBuffer);
        clSetKernelArg(kernel, 2, sizeof(int), &width);
        clSetKernelArg(kernel, 3, sizeof(int), &height);
        clSetKernelArg(kernel, 4, sizeof(int), &channels);
        size_t globalSize[2] = {(size_t)width, (size_t)height};
        checkError(clEnqueueNDRangeKernel(ocl_queue, kernel, 2, NULL, globalSize, NULL, 0, NULL, NULL), "clEnqueueNDRangeKernel");
    }

    checkError(clEnqueueReadBuffer(ocl_queue, outputBuffer, CL_TRUE, 0, size, result, 0, NULL, NULL), "clEnqueueReadBuffer");

    clReleaseMemObject(inputBuffer);
    clReleaseMemObject(outputBuffer);
    clReleaseKernel(kernel);

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
