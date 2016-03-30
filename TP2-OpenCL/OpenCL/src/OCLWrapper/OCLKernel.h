#ifndef _OCL_KERNEL_H_
#define _OCL_KERNEL_H_

#include <QString>

#include <CL/opencl.h>

class OCLProgram;
class OCLContext;
class OCLBuffer;

class OCLKernel
{
public:
    OCLKernel(OCLProgram* program, QString name);
    ~OCLKernel();

    QString GetKernelName() const { return m_kernelName; }
    OCLProgram* GetOCLProgram() const;
    OCLContext* GetOCLContext() const;

    bool SetArgBuffer(int paramID, OCLBuffer* buffer);
    bool SetArgBuffer(int paramID, QString bufferName);
    bool SetArgULong(int paramID, unsigned long& val);
    bool SetArgInt(int paramID, int& val);
    bool SetArgDouble(int paramID, double& val);
    bool SetArgFloat(int paramID, float& val);

private:
    cl_kernel GetInnerKernel() const { return m_innerKernel; }

	OCLKernel(const OCLKernel&);
	OCLKernel& operator=(const OCLKernel&);

    QString m_kernelName;
    cl_kernel m_innerKernel;
    OCLProgram* m_program;

    friend class OCLProgram;
};

#endif //_OCL_KERNEL_H_
