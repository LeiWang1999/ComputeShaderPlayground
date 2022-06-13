// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

//; eval_flags(c-hlsl_win64): [x86_64-w64-mingw32-g++] -O2 -static -lpthread

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "D3D12APIWrapper.h"

#define CHECK(stat, reason, ...)  ((stat) ? 1 : (fprintf(stderr, "[CheckFail] "), fprintf(stderr, reason, ##__VA_ARGS__), fprintf(stderr, "\n\n"), fflush(stderr), exit(1), 0))

namespace ab {

  static HMODULE hLibDll;

  void init(int dev) {
    CHECK(0 == dxInit(0), "Failed initialize DirectX12 device.");
  }

  void finalize() {
    CHECK(0 == dxFinalize(), "Failed to finalize DirectX12 device.");
  }

  void* alloc(size_t byteSize, const std::vector<size_t> &shape, const std::string &dtype, const std::string &name) {
    void *dptr = dxMemAlloc(byteSize);
    return dptr;
  }

  void release(void *dptr, size_t byteSize) {
    CHECK(0 == dxMemFree(dptr), "Failed to free device pointer.");
  }

  void* moduleLoad(const std::string &source) {
    void *hModule = dxModuleLoad(source.c_str());
    CHECK(hModule != nullptr, "Failed to load device module.");
    return hModule;
  }

  std::vector<void*> moduleGetFunction(const void *hModule, const std::string &fname, const std::unordered_map<std::string, int> &threads) {
    void *hFunction = dxModuleGetShader(const_cast<void*>(hModule), fname.c_str());
    CHECK(hFunction != nullptr, "Failed to get function `%s` from module.", fname.c_str());
    return { hFunction };
  }

  void launchKernel(const std::vector<void*> &hFunction, const std::vector<void*> &krnl_args, void *stream) {
    CHECK(0 == dxShaderLaunchAsync(const_cast<void*>(hFunction[0]), const_cast<void**>(krnl_args.data()), stream), "Failed to launch a shader.");
  }

  void memcpyHtoD(void *dptr, void *hptr, size_t byteSize, void *stream) {
    CHECK(0 == dxMemcpyHtoDAsync(dptr, hptr, byteSize, stream), "Failed to copy memory to device.");
  }

  void memcpyDtoH(void *hptr, void *dptr, size_t byteSize, void *stream) {
    CHECK(0 == dxMemcpyDtoHAsync(hptr, dptr, byteSize, stream), "Failed to copy memory from device.");
  }

  void synchronize(void *stream) {
    CHECK(0 == dxStreamSynchronize(stream), "Failed to sychronize default device stream.");
  }

  void* recordTime(void *stream) {
    void *hEvent = dxEventCreate();
    CHECK(0 == dxEventRecord(hEvent, stream), "Failed to record event to default stream.");
    return hEvent;
  }

  double convertToElapsedTime(void *hStart, void *hStop) {
    ab::synchronize(nullptr);
    float sec = dxEventElapsedSecond(hStart, hStop);
    CHECK(0 == dxEventDestroy(hStart), "Failed to destroy released event.");
    CHECK(0 == dxEventDestroy(hStop), "Failed to destroy released event.");
    return (double)sec;
  }
}

