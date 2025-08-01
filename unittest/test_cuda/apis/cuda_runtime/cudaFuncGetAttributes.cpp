#include"test_cuda/test_cuda_common.h"


TEST_F(PhOSCudaTest, cudaFuncGetAttributes) {
    cudaError cuda_retval;
    CUmodule module;
    CUmodule *module_ptr = &module;
    CUfunction function;
    CUfunction *function_ptr = &function;
    std::ifstream in;
    std::stringstream buffer;
    std::string function_name;
    const char* function_name_ptr;
    cudaFuncAttributes attr;
    cudaFuncAttributes *attr_ptr = &attr;

    std::filesystem::path current_path = __FILE__;
    std::filesystem::path current_abs_path = std::filesystem::absolute(current_path);
    std::filesystem::path current_dir_abs_path = current_abs_path.parent_path();
    std::filesystem::path current_dir_dir_abs_path = current_dir_abs_path.parent_path();

    #if CUDA_VERSION >= 9000 && CUDA_VERSION < 11040
        std::filesystem::path cubin_asb_path = std::filesystem::canonical(
            current_dir_dir_abs_path / "assets" / "sm70_72_75_80_86.fatbin"
        );
    #else
        POS_WARN("no test file for current cuda architecture: cuda_version(%d)", CUDA_VERSION);
        goto exit;
    #endif

    in.open(cubin_asb_path, std::ios::binary);
    EXPECT_EQ(true, in.is_open());
    buffer << in.rdbuf();

    // load module first
    cuda_retval = (cudaError)this->_ws->pos_process( 
        /* api_id */ PosApiIndex_cuModuleLoadData, 
        /* uuid */ this->_clnt->id,
        /* param_desps */ { 
            { .value = &module_ptr, .size = sizeof(CUmodule*) },
            { .value = buffer.str().data(), .size = buffer.str().size() }
        }
    );
    EXPECT_EQ(cudaSuccess, cuda_retval);

    function_name = "_Z15squareMatrixMulPKiS0_Pii";
    function_name_ptr = function_name.data();

    // get function
    cuda_retval = (cudaError)this->_ws->pos_process( 
        /* api_id */ PosApiIndex_cuModuleGetFunction, 
        /* uuid */ this->_clnt->id,
        /* param_desps */ { 
            { .value = &function_ptr, .size = sizeof(CUfunction*) },
            { .value = &module, .size = sizeof(CUmodule) },
            { .value = &function_name_ptr, .size = sizeof(const char*) }
        }
    );
    EXPECT_EQ(cudaSuccess, cuda_retval);

    cuda_retval = (cudaError)this->_ws->pos_process(
        /* api_id */ PosApiIndex_cudaFuncGetAttributes,
        /* uuid */ this->_clnt->id,
        /* param_desps */ {
            { .value = &attr_ptr, .size = sizeof(cudaFuncAttributes*) },
            { .value = &function, .size = sizeof(const void*) }
        }
    );
    EXPECT_EQ(cudaSuccess, cuda_retval);
    EXPECT_GT(attr.maxThreadsPerBlock, 0);
    EXPECT_GE(attr.numRegs, 0);

exit:
    if(in.is_open()){
        in.close();
    }    
}
