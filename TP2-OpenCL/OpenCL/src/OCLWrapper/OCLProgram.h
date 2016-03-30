#ifndef _OPENCL_PROGRAM_H_
#define _OPENCL_PROGRAM_H_

#include "OCLContext.h"
#include "OCLBuffer.h"

#include <CL/opencl.h>

#include <QList>
#include <QHash>

class OCLKernel;

class OCLProgram
{
public:
    OCLProgram();
    OCLProgram(OCLContext* context);
    ~OCLProgram();

    OCLContext* GetContext() const { return m_context; }
    
    bool AddProgramSourceFromSourceFile(QString fileName);
    bool AddProgramSourceFromSourceCode(const QString& sourceCode);
    bool AddProgramSourceFromSourceCode(const char** sourceCode, int sourceCount);
    
    bool CreateProgramFromSourceCode(const char** sourceCode, int sourceCount);
    bool CreateProgramFromSourceCode(const QString& sourceCode);
    bool CreateProgramFromSourceFile(QString fileName);
    bool CreateProgramWithAddedSources();

    OCLBuffer* CreateBuffer(QString name, OCLBuffer::BufferAccess flags, int size);
    OCLBuffer* CreateBuffer(QString name, OCLBuffer::BufferAccess flags, int size, void* source);
    OCLBuffer* GetBuffer(QString name);
    bool ReadBuffer(OCLBuffer* buffer, void* dest);
    bool ReadBuffer(QString bufferName, void* dest);
    bool WriteBuffer(OCLBuffer* buffer, void* source);    
    bool WriteBuffer(QString bufferName, void* source);

    OCLKernel* CreateKernelFunction(QString kernelFunctionName);
    OCLKernel* GetKernelByName(QString kernelName) const;

    bool ExecuteKernel(OCLKernel* kernel, unsigned int workDimension, const size_t* globalWorkSize, const size_t* localWorkSize);
    bool ExecuteKernel(QString kernelName, unsigned int workDimension, const size_t* globalWorkSize, const size_t* localWorkSize);

    QString GetBuildInfos();
    QString GetBuildInfo(OCLDeviceInfo* device);
    
private:
    cl_program GetInnerProgram() const { return m_oclInnerProgram; }

	OCLProgram(const OCLProgram&);
	OCLProgram& operator=(const OCLProgram&);

    OCLContext* m_context;
    cl_program m_oclInnerProgram;
    cl_command_queue m_commandQueue;
    QHash<QString, OCLKernel*> m_kernels;    
    QHash<QString, OCLBuffer*> m_buffers;
    QList<QString> m_programsCode;

    friend class OCLKernel;
};

#endif //_OPENCL_PROGRAM_H_
