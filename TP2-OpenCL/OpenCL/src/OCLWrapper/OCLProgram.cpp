#include "OCLProgram.h"
#include "OCLKernel.h"

#include <QDebug>
#include <QFile>
#include <QString>
#include <QTextStream>

// Create a new program in a new context
OCLProgram::OCLProgram()
: m_oclInnerProgram(NULL)
{
    m_context = new OCLContext();
    m_commandQueue = clCreateCommandQueue(m_context->GetInnerContext(), m_context->GetDevicesID()[0], 0, 0);
}

// Create a new program in an existing context
OCLProgram::OCLProgram(OCLContext* context)
: m_context(context)
, m_oclInnerProgram(NULL)
{
    m_commandQueue = clCreateCommandQueue(context->GetInnerContext(), context->GetDevicesID()[0], 0, 0);
}

// Destroy the current program
OCLProgram::~OCLProgram(void)
{
    clFinish(m_commandQueue);

    QHash<QString, OCLKernel*>::Iterator it = m_kernels.begin();
    for(; it != m_kernels.end(); ++it)
    {
        delete it.value();
    }
    m_kernels.clear();

    for(QHash<QString, OCLBuffer*>::Iterator it = m_buffers.begin(); it != m_buffers.end(); ++it)
    {
        delete it.value();
    }
    m_buffers.clear();

    if(m_oclInnerProgram)
    {
        if(clReleaseProgram(m_oclInnerProgram) != CL_SUCCESS)
            qDebug()<<"Error on deleting program.";
    }

    if(clReleaseCommandQueue(m_commandQueue) != CL_SUCCESS)
        qDebug()<<"Error on deleting command queue.";

    delete m_context;
}

// Add a part of the code to the current building program
bool OCLProgram::AddProgramSourceFromSourceFile(QString fileName)
{
    bool success = false;
    QFile openCLFile(fileName);
    if (openCLFile.open(QFile::ReadOnly))
    {
        QTextStream openCLStream(&openCLFile);
        QString ocl = openCLStream.readAll();
        QByteArray text = ocl.toAscii();
        const char* text2 = text.data();
        success = AddProgramSourceFromSourceCode(&text2, 1);
        openCLFile.close();
    }
    return success;
}

// Add a part of the code to the current building program
bool OCLProgram::AddProgramSourceFromSourceCode(const QString& sourceCode)
{
    m_programsCode.append(sourceCode);
    return true;
}

// Add a part of the code to the current building program
bool OCLProgram::AddProgramSourceFromSourceCode(const char** sourceCode, int sourceCount)
{
    for(int i = 0; i < sourceCount; ++i)
        m_programsCode.append(QString(sourceCode[i]));
    return true;
}

// Create the program from all the added parts
bool OCLProgram::CreateProgramWithAddedSources()
{
    cl_int errNo;
    const char** sources = new const char*[m_programsCode.size()];
    for(int i = 0; i < m_programsCode.size() ; ++i)
    {
        QByteArray code = m_programsCode[i].toAscii();
        sources[i] = code.data();
    }
    qDebug()<<"Compiling OpenCL program...";
    m_oclInnerProgram = clCreateProgramWithSource(m_context->GetInnerContext(), m_programsCode.size(), sources, 0, &errNo);
    errNo |= clBuildProgram(m_oclInnerProgram, 0, 0, 0, 0, 0);
    if(errNo != CL_SUCCESS)
        qDebug()<<"Error while compiling OpenCL program :\n"<<GetBuildInfos();
    else
        qDebug()<<"Compile successfully completed. No error found.";
    return errNo == CL_SUCCESS;
}

// Create the program from the specified file
bool OCLProgram::CreateProgramFromSourceFile(QString fileName)
{
    bool success = false;
    QFile openCLFile(fileName);
    if (openCLFile.open(QFile::ReadOnly))
    {
        QTextStream openCLStream(&openCLFile);
        QString ocl = openCLStream.readAll();
        QByteArray code = ocl.toAscii();
        const char* sourceCode = code.data();
        success = CreateProgramFromSourceCode(&sourceCode, 1);
        openCLFile.close();
    }
    return success;
}

// Create the program from the specified code
bool OCLProgram::CreateProgramFromSourceCode(const QString& sourceCode)
{
    cl_int errNo;
    m_programsCode.append(sourceCode);
    QByteArray code = sourceCode.toAscii();
    const char* source = code.data();
    qDebug()<<"Compiling OpenCL program...";
    m_oclInnerProgram = clCreateProgramWithSource(m_context->GetInnerContext(), 1, &source, 0, &errNo);
    errNo |= clBuildProgram(m_oclInnerProgram, 0, 0, 0, 0, 0);
    if(errNo != CL_SUCCESS)
        qDebug() <<"Error while compiling OpenCL program :\n"<<GetBuildInfos();
    else
        qDebug()<<"Compile successfully completed. No error found.";
    return errNo == CL_SUCCESS;
}

// Create the program from the specified code. The code can contains more than one file.
bool OCLProgram::CreateProgramFromSourceCode(const char** sourceCode, int sourceCount)
{
    cl_int errNo;
    for(int i = 0; i < sourceCount; ++i)
        m_programsCode.append(QString(sourceCode[i]));
    qDebug()<<"Compiling OpenCL program...";
    m_oclInnerProgram = clCreateProgramWithSource(m_context->GetInnerContext(), sourceCount, sourceCode, 0, &errNo);
    errNo |= clBuildProgram(m_oclInnerProgram, 0, 0, 0, 0, 0);
    if(errNo != CL_SUCCESS)
        qDebug()<<"Error while compiling OpenCL program :\n"<<GetBuildInfos();
    else
        qDebug()<<"Compile successfully completed. No error found.";
    return errNo == CL_SUCCESS;
}

QString OCLProgram::GetBuildInfos()
{
    QString buildLog;
    for(unsigned int i = 0; i < m_context->GetNumberOfDevices(); ++i)
    {
        size_t logSize;
        OCLDeviceInfo* device = m_context->GetDeviceInfo(i);
        clGetProgramBuildInfo(m_oclInnerProgram, device->GetDeviceID(), CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
        char* log = new char[logSize];
        clGetProgramBuildInfo(m_oclInnerProgram, device->GetDeviceID(), CL_PROGRAM_BUILD_LOG, logSize, log, 0);
        QString buildStatus = QString(log);
        delete log;
        buildLog.append("Build log for device ");
        buildLog.append(device->GetDeviceName());
        buildLog.append("\n");
        buildLog.append(buildStatus);
    }

    return buildLog;
}

QString OCLProgram::GetBuildInfo(OCLDeviceInfo* device)
{
    size_t logSize;
    clGetProgramBuildInfo(m_oclInnerProgram, device->GetDeviceID(), CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
    char* log = new char[logSize];
    clGetProgramBuildInfo(m_oclInnerProgram, device->GetDeviceID(), CL_PROGRAM_BUILD_LOG, logSize, log, 0);
    QString returnString = QString(log);
    delete[] log;
    return returnString;
}

// Create a kernel function in this program
OCLKernel* OCLProgram::CreateKernelFunction(QString kernelFunctionName)
{
    OCLKernel* kernel = m_kernels.value(kernelFunctionName, NULL);
    if(!kernel)
    {
        kernel = new OCLKernel(this, kernelFunctionName);
        m_kernels.insert(kernelFunctionName, kernel);
    }
    return kernel;
}

// Get a previously created kernel function by it's name
OCLKernel* OCLProgram::GetKernelByName(QString kernelName) const
{
    return m_kernels.value(kernelName, NULL);
}

// Execute the specified kernel function into the specified dimension with the specified worker
bool OCLProgram::ExecuteKernel(OCLKernel* kernel, unsigned int workDimension, const size_t* globalWorkSize, const size_t* localWorkSize)
{
    int errNo = clEnqueueNDRangeKernel(m_commandQueue, kernel->GetInnerKernel(), workDimension, 0, globalWorkSize, localWorkSize, 0, 0, 0);
    qDebug() << errNo;
    return errNo == CL_SUCCESS;
}

bool OCLProgram::ExecuteKernel(QString kernelName, unsigned int workDimension, const size_t* globalWorkSize, const size_t* localWorkSize)
{
    OCLKernel* kernel = GetKernelByName(kernelName);
    if(kernel)
        return ExecuteKernel(kernel, workDimension, globalWorkSize, localWorkSize);
    return false;
}

OCLBuffer* OCLProgram::CreateBuffer(QString name, OCLBuffer::BufferAccess flags, int size)
{
    OCLBuffer* buff = m_buffers.value(name, NULL);
    if(!buff)
    {
        buff = new OCLBuffer(this, flags, size);
        m_buffers.insert(name, buff);
    }
    return buff;
}

OCLBuffer* OCLProgram::CreateBuffer(QString name, OCLBuffer::BufferAccess flags, int size, void* source)
{
    OCLBuffer* buff = m_buffers.value(name, NULL);
    if(!buff)
    {
        buff = new OCLBuffer(this, (OCLBuffer::BufferAccess)(flags | OCLBuffer::COPY_HOST_PTR), size, source);
        m_buffers.insert(name, buff);
    }
    return buff;
}

OCLBuffer* OCLProgram::GetBuffer(QString name)
{
    return m_buffers.value(name, NULL);
}

bool OCLProgram::ReadBuffer(OCLBuffer* buffer, void* dest)
{
    return clEnqueueReadBuffer(m_commandQueue, buffer->GetInnerBuffer(), CL_TRUE, 0, buffer->GetSize(), dest, 0, 0, 0) == CL_SUCCESS;
}

bool OCLProgram::ReadBuffer(QString bufferName, void* dest)
{
    OCLBuffer* buffer = m_buffers.value(bufferName, NULL);
    if(buffer)
    {
        return clEnqueueReadBuffer(m_commandQueue, buffer->GetInnerBuffer(), CL_TRUE, 0, buffer->GetSize(), dest, 0, 0, 0) == CL_SUCCESS;
    }
    return false;
}

bool OCLProgram::WriteBuffer(OCLBuffer* buffer, void* source)
{
    return clEnqueueWriteBuffer(m_commandQueue, buffer->GetInnerBuffer(), CL_FALSE, 0, buffer->GetSize(), source, 0, 0, 0) == CL_SUCCESS;
}

bool OCLProgram::WriteBuffer(QString bufferName, void* source)
{
    OCLBuffer* buffer = m_buffers.value(bufferName, NULL);
    if(buffer)
    {
        return clEnqueueWriteBuffer(m_commandQueue, buffer->GetInnerBuffer(), CL_FALSE, 0, buffer->GetSize(), source, 0, 0, 0) == CL_SUCCESS;
    }
    return false;
}
