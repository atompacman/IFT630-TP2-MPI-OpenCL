#include "OCLKernel.h"
#include "OCLBuffer.h"
#include "OCLProgram.h"
#include "OCLContext.h"

#include <QDebug>

OCLKernel::OCLKernel(OCLProgram* program, QString name)
: m_kernelName(name)
, m_program(program)
{
    m_innerKernel = clCreateKernel(program->GetInnerProgram(), name.toAscii().data(), 0);
}

OCLKernel::~OCLKernel()
{
    if(clReleaseKernel(m_innerKernel) != CL_SUCCESS)
        qDebug()<<"Error in deleting kernel object.";
}

OCLProgram* OCLKernel::GetOCLProgram() const
{
    return m_program;
}

OCLContext* OCLKernel::GetOCLContext() const
{
    return m_program->GetContext();
}

bool OCLKernel::SetArgBuffer(int paramID, OCLBuffer* buffer)
{
    int errNo = clSetKernelArg(m_innerKernel, paramID, sizeof(cl_mem), (void *)&(buffer->GetInnerBuffer()));
    return errNo == CL_SUCCESS;
}

bool OCLKernel::SetArgBuffer(int paramID, QString bufferName)
{
    OCLBuffer* buff = m_program->GetBuffer(bufferName);
    if(buff)
    {
        int errNo = clSetKernelArg(m_innerKernel, paramID, sizeof(cl_mem), (void *)&(buff->GetInnerBuffer()));
        return errNo == CL_SUCCESS;
    }
    return false;
}

bool OCLKernel::SetArgULong(int paramID, unsigned long& val)
{
    int errNo = clSetKernelArg(m_innerKernel, paramID, sizeof(cl_ulong), (void *)&val);
    return errNo == CL_SUCCESS;
}

bool OCLKernel::SetArgInt(int paramID, int& val)
{
    int errNo = clSetKernelArg(m_innerKernel, paramID, sizeof(cl_int), (void *)&val);
    return errNo == CL_SUCCESS;
}

bool OCLKernel::SetArgDouble(int paramID, double& val)
{
    int errNo = clSetKernelArg(m_innerKernel, paramID, sizeof(cl_double), (void *)&val);
    return errNo == CL_SUCCESS;
}

bool OCLKernel::SetArgFloat(int paramID, float& val)
{
    int errNo = clSetKernelArg(m_innerKernel, paramID, sizeof(cl_float), (void *)&val);
    return errNo == CL_SUCCESS;
}
