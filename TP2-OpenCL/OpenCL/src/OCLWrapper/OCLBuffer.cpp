#include "OCLBuffer.h"
#include "OCLContext.h"
#include "OCLProgram.h"

#include <QDebug>

OCLBuffer::OCLBuffer(OCLProgram* program, BufferAccess flag, int size)
: m_program(program)
, m_size(size)
, m_flags((OCLBuffer::BufferAccess)(flag & ~OCLBuffer::COPY_HOST_PTR))
{
    m_context = program->GetContext();
    m_innerBuffer = clCreateBuffer(m_context->GetInnerContext(), m_flags, size, 0, 0);
}

OCLBuffer::OCLBuffer(OCLProgram* program, BufferAccess flag, int size, void* buffSource)
: m_program(program)
, m_size(size)
, m_flags((OCLBuffer::BufferAccess)(flag | OCLBuffer::COPY_HOST_PTR))
{
    m_context = program->GetContext();
    m_innerBuffer = clCreateBuffer(m_context->GetInnerContext(), m_flags, size, buffSource, 0);
}

OCLBuffer::~OCLBuffer()
{
    if(clReleaseMemObject(m_innerBuffer) != CL_SUCCESS)
        qDebug()<<"Error in deleting buffer object.";
}

bool OCLBuffer::WriteBuffer(void* buff)
{
    return m_program->WriteBuffer(this, buff);
}

bool OCLBuffer::ReadBuffer(void* buff)
{
    return m_program->ReadBuffer(this, buff);
}