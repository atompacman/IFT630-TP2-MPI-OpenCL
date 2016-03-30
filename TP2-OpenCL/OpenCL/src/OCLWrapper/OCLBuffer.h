#ifndef _OCL_BUFFER_H_
#define _OCL_BUFFER_H_

#include <CL/opencl.h>

class OCLContext;
class OCLProgram;

class OCLBuffer
{
public:
    enum BufferAccess { READ_WRITE =(1 << 0), WRITE_ONLY = (1 << 1), READ_ONLY = (1 << 2), USE_HOST_PTR = (1 << 3), ALLOC_HOST_PTR = (1 << 4), COPY_HOST_PTR = (1 << 5) };

    OCLBuffer(OCLProgram* program, BufferAccess flag, int size);
    OCLBuffer(OCLProgram* program, BufferAccess flag, int size, void* buffSource);
    ~OCLBuffer();

    int GetSize() const { return m_size; }
    
    bool ReadBuffer(void* buff);
    bool WriteBuffer(void* buff);

private:
    cl_mem& GetInnerBuffer() { return m_innerBuffer; }

	OCLBuffer(const OCLBuffer&);
	OCLBuffer& operator=(const OCLBuffer&);

    OCLContext* m_context;
    OCLProgram* m_program;
    int m_size;
    BufferAccess m_flags;
    cl_mem m_innerBuffer;

    friend class OCLProgram;
    friend class OCLKernel;
};

#endif //_OCL_BUFFER_H_