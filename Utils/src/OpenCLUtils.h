#pragma once

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include "Cl/cl.h"

#include <string>

class OpenCLUtils 
{
public:
    /// <summary>
	/// Find a GPU or CPU associated with the first available platform
	/// The `platform` structure identifies the first platform identified by the
	/// OpenCL runtime.A platform identifies a vendor's installation, so a system
	/// may have an NVIDIA platform and an AMD platform.
	/// 
	/// The `device` structure corresponds to the first accessible device
	/// associated with the platform.Because the second parameter is
	/// `CL_DEVICE_TYPE_GPU`, this device must be a GPU.
    /// </summary>
    /// <returns></returns>
    static cl_device_id create_device();

	static bool initialize_device_and_context(cl_device_id& device,
                                              cl_context& context);

    /// <summary>
    /// Create program from a file and compile it. 
    /// </summary>
    /// <param name="ctx"></param>
    /// <param name="dev"></param>
    /// <param name="filename"></param>
    /// <returns></returns>
    static cl_program build_program(cl_context ctx, cl_device_id dev, const char* filename);

	static bool initialize_program(const std::string& filepath,
                                   const std::string& kernalName,
                                   cl_context context,
                                   cl_device_id device,
                                   cl_program& program,
                                   cl_kernel& kernel,
                                   cl_command_queue& queue);

    static cl_mem create_input_buffer(cl_context context, void* dataPtr, size_t dataSize);

    static cl_mem create_inout_buffer(cl_context context, void* dataPtr, size_t dataSize);

    static cl_mem create_output_buffer(cl_context context, size_t dataSize);
};