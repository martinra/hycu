#ifndef _H_OPENCL_INTERFACE
#define _H_OPENCL_INTERFACE

#include <memory>
#include <CL/cl.hpp>

using namespace std;


class OpenCLInterface
{
  public:
    OpenCLInterface();

    friend class Curve;
    friend class ReductionTable;

  protected:

    shared_ptr<cl::Platform> platform;
    shared_ptr<cl::Device> device;
    shared_ptr<cl::Context> context;
    shared_ptr<cl::CommandQueue> queue;

    shared_ptr<cl::Program> program_evaluation;
    shared_ptr<cl::Program> program_reduction;
};

#endif
