
#define NOMINMAX

#include "execute_module.hpp"
//#include "D3D12APIWrapper.h"

#ifndef _GAMING_XBOX_SCARLETT
#define OutputDebugStringA printf
#endif

int main(int argc, char** argv)
{
    //printf("start evaluation\n");
    /*if (argc > 1) {
        std::string code, line;
        while (std::getline(std::cin, line)) {
            if (line == "EOF")
                break;
            code += line + '\n';
        }
        FILE* fp = fopen("./my_kernel.cc", argv[1]);
        fwrite(code.c_str(), code.size(), 1, fp);
        fclose(fp);
        return 0;
    }*/
    float expected_timeout = getenv("EXPECTED_TIMEOUT") ? std::atof(getenv("EXPECTED_TIMEOUT")) : -1;
#if !defined(_WIN64) || defined(__MINGW64__)
    pthread_t p_timeout_monitor;
    pthread_create(&p_timeout_monitor, NULL, [](void* arg) -> void* {
        float& expected_timeout = *(float*)arg;
        if (expected_timeout <= 0)
            return nullptr;
        int timeout_sec = 12 + expected_timeout;
        sleep(timeout_sec);
        fprintf(stderr, "[FATAL] Time limit exceeded (>= %d sec) for this evaluation.\n", timeout_sec);
        exit(1);
        return nullptr;
    }, &expected_timeout);
    pthread_detach(p_timeout_monitor);
#endif
    int dev = getenv("DEV_ID") ? std::atoi(getenv("DEV_ID")) : 0;
    ab::init(dev);
    const char* module_path = argc > 1 ? argv[1] : "./my_kernel.cc";
    ExecutionModule gm(std::string("file://") + module_path);

    //printf("prepare inputs\n");
    std::vector<void*> global_args;
    for (int i = 0; i < gm.global_inputs.size(); ++i) {
        auto& it = gm.global_inputs[i];
        void* dptr = allocate_tensor(it);
        global_args.push_back(dptr);

        std::vector<char> hptr(it.mem_size());
        size_t size = it.element_size();
        if (it.dtype == "int32") {
            for (size_t x = 0; x < size; ++x)
                ((int*)hptr.data())[x] = (x + i + 1) % 71;
        }
        else if (it.dtype == "float16") {
            for (size_t x = 0; x < size; ++x)
                ((half*)hptr.data())[x] = (x + i + 1) % 71;
        }
        else if (it.dtype == "float32") {
            for (size_t x = 0; x < size; ++x)
                ((float*)hptr.data())[x] = (x + i + 1) % 71;
        }
        else if (it.dtype == "float64") {
            for (size_t x = 0; x < size; ++x)
                ((double*)hptr.data())[x] = (x + i + 1) % 71;
        }
        else {
            size_t byte_size = size * it.type_size();
            for (size_t x = 0; x < byte_size / sizeof(int); ++x)
                ((int*)hptr.data())[x] = (x + i + 1) % 71;
            for (size_t x = byte_size - byte_size % sizeof(int); x < byte_size; x++)
                ((char*)hptr.data())[x] = 1;
        }
        ab::memcpyHtoD(dptr, hptr.data(), hptr.size(), nullptr);
        ab::synchronize(nullptr);
    }
    for (auto& it : gm.global_outputs) {
        void* dptr = allocate_tensor(it);
        global_args.push_back(dptr);
    }

    //printf("first run\n");
    gm.compute(global_args.data());
    //printf("get outputs\n");
    for (int i = 0; i < gm.global_outputs.size(); ++i) {
        auto& it = gm.global_outputs[i];
        void* dptr = global_args[gm.global_inputs.size() + i];

        std::vector<char> hptr(it.mem_size());
        ab::memcpyDtoH(hptr.data(), dptr, hptr.size(), nullptr);
        ab::synchronize(nullptr);

        size_t byte_size = it.mem_size();
        double digest = 0.0;
        if (it.dtype == "int32") {
            for (size_t x = 0; x < byte_size / sizeof(int); ++x)
                digest += (x + 1) % 83 * ((int*)hptr.data())[x];
        }
        else if (it.dtype == "float16") {
            for (size_t i = 0; i < 10; i++)
            {
                ::std::cout << "output[" << i << "] : " << ((half *)hptr.data())[i] << ::std::endl;
            }
            for (size_t x = 0; x < byte_size / sizeof(half); ++x)
                digest += (x + 1) % 83 * ((half*)hptr.data())[x];
        }
        else if (it.dtype == "float32") {
            for (size_t i = 0; i < 10; i++)
            {
                ::std::cout << "output[" << i << "] : " << ((float*)hptr.data())[i] << ::std::endl;
            }
            for (size_t x = 0; x < byte_size / sizeof(float); ++x)
            {
                
                digest += (x + 1) % 83 * ((float*)hptr.data())[x];
            }
        }
        else if (it.dtype == "float64") {
            for (size_t x = 0; x < byte_size / sizeof(double); ++x)
                digest += (x + 1) % 83 * ((double*)hptr.data())[x];
        }
        else {
            for (size_t x = 0; x < byte_size / sizeof(float); ++x)
                digest += (x + 1) % 83 * ((float*)hptr.data())[x];
            for (size_t x = byte_size - byte_size % sizeof(int); x < byte_size; x++)
                digest += ((char*)hptr.data())[x];
        }
        printf("\n- K/%d: %.10e\n", i, digest);
    }

    //printf("performance run\n");
    do {
        auto x = ab::recordTime(nullptr);
        gm.compute(global_args.data());
        auto y = ab::recordTime(nullptr);
        ab::synchronize(nullptr);

        double tpr = ab::convertToElapsedTime(x, y);
        if ((expected_timeout > 0 && tpr > expected_timeout) || tpr > 2) {
            printf("\n- TPR: %g\n", tpr);
            break;
        }

        int num_runs = (int)::std::max(1LU, std::min(10000LU, (unsigned long)(1.0 / tpr)));
        tpr = 0.0f;
        x = ab::recordTime(nullptr);
        for (int i = 0; i < num_runs; ++i)
            gm.compute(global_args.data());
        y = ab::recordTime(nullptr);
        tpr = ab::convertToElapsedTime(x, y) / num_runs;
        printf("\n- TPR: %g\n", tpr);
    } while (0);

    ab::finalize();
    return 0;
}


