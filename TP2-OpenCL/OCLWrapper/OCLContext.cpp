#include "OCLContext.h"

#include <QDebug>

OCLDeviceInfo::OCLDeviceInfo(OCLPlatformInfo* owner, cl_device_id deviceID)
: m_platformOwner(owner)
, m_deviceID(deviceID)
{
	size_t nameSize;
	clGetDeviceInfo(m_deviceID, CL_DEVICE_NAME, 0, 0, &nameSize);
	char* name = new char[nameSize];
	clGetDeviceInfo(m_deviceID, CL_DEVICE_NAME, nameSize, name, 0);
	m_deviceName = QString(name);
	delete[] name;
	clGetDeviceInfo(m_deviceID, CL_DEVICE_TYPE, sizeof(cl_device_type), &m_deviceType, 0);
	clGetDeviceInfo(m_deviceID, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(uint), &m_computeUnits, 0);
	clGetDeviceInfo(m_deviceID, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &m_maxWorkGroupSize, 0);
	clGetDeviceInfo(m_deviceID, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(uint), &m_maxWorkItemDimensions, 0);
	clGetDeviceInfo(m_deviceID, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(ulong), &m_globalMemSize, 0);
	clGetDeviceInfo(m_deviceID, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(ulong), &m_localMemSize, 0);
}


OCLPlatformInfo::OCLPlatformInfo(cl_platform_id platformID, cl_device_type deviceType)
: m_platformID(platformID)
{
    size_t nameSize;
    cl_uint ciErrNum = clGetPlatformInfo(m_platformID, CL_PLATFORM_NAME, 0, NULL, &nameSize);
    char* name = new char[nameSize];
    ciErrNum = clGetPlatformInfo(m_platformID, CL_PLATFORM_NAME, nameSize, name, 0);
    m_name = QString(name);
    delete[] name;
    ciErrNum = clGetPlatformInfo(m_platformID, CL_PLATFORM_VERSION, 0, NULL, &nameSize);
    name = new char[nameSize];
    ciErrNum = clGetPlatformInfo(m_platformID, CL_PLATFORM_VERSION, nameSize, name, 0);
    m_version = QString(name);
    delete name;
    ciErrNum = clGetPlatformInfo(m_platformID, CL_PLATFORM_VENDOR, 0, NULL, &nameSize);
    name = new char[nameSize];
    ciErrNum = clGetPlatformInfo(m_platformID, CL_PLATFORM_VENDOR, nameSize, name, 0);
    m_vendor = QString(name);
    delete name;
    ciErrNum = clGetPlatformInfo(m_platformID, CL_PLATFORM_EXTENSIONS, 0, NULL, &nameSize);
    name = new char[nameSize];
    ciErrNum = clGetPlatformInfo(m_platformID, CL_PLATFORM_EXTENSIONS, nameSize, name, 0);
    m_extension = QString(name);
    delete name;

    uint totalDevice;
    ciErrNum = clGetDeviceIDs(m_platformID, deviceType, 0, NULL, &totalDevice);
    cl_device_id* devicesID =  new cl_device_id[totalDevice];
    ciErrNum = clGetDeviceIDs(m_platformID, deviceType, totalDevice, devicesID, NULL);
    for(int i = 0; i < totalDevice; ++i)
    {
        m_devices.append(new OCLDeviceInfo(this, devicesID[i]));
    }
    delete devicesID;
}

// Create a new context for the specified deviceType
OCLContext::OCLContext(cl_device_type deviceType)
: m_devicesID(NULL)
, m_numberDevices(0)
{
    int errNo;    
    cl_uint num_platform;
    cl_int ciErrNum = clGetPlatformIDs(0, NULL, &num_platform);
    cl_platform_id* cpPlatforms = new cl_platform_id[num_platform];
    ciErrNum = clGetPlatformIDs(num_platform, cpPlatforms, 0);
    
    for(int i = 0; i < num_platform; ++i)
    {
        uint totalDevice = 0;
        OCLPlatformInfo* pInfo = new OCLPlatformInfo(cpPlatforms[i], deviceType);
        m_platformInfo.append(pInfo);
        m_numberDevices += pInfo->GetDeviceCount();
        m_devicesInfo.append(*pInfo->GetDevices());
    }

    m_devicesID =  new cl_device_id[m_numberDevices];
    for(int i = 0; i < m_numberDevices; ++i)
    {
        m_devicesID[i] = m_devicesInfo[i]->GetDeviceID();
    }

    m_innerContext = clCreateContext(0, m_numberDevices, m_devicesID, NULL, NULL, &ciErrNum);
    if(ciErrNum != CL_SUCCESS)
        qDebug()<<"Error on creating context.";
}

// Create a new context for all the device
OCLContext::OCLContext()
: m_devicesID(NULL)
, m_numberDevices(0)
{
    int errNo;    
    cl_uint num_platform;
    cl_int ciErrNum = clGetPlatformIDs(0, NULL, &num_platform);
    cl_platform_id* cpPlatforms = new cl_platform_id[num_platform];
    ciErrNum = clGetPlatformIDs(num_platform, cpPlatforms, 0);
    
    for(int i = 0; i < num_platform - 1; ++i)
    {
        uint totalDevice = 0;
        OCLPlatformInfo* pInfo = new OCLPlatformInfo(cpPlatforms[i]);
        m_platformInfo.append(pInfo);
        m_numberDevices += pInfo->GetDeviceCount();
        m_devicesInfo.append(*pInfo->GetDevices());
    }

    m_devicesID =  new cl_device_id[m_numberDevices];
    for(int i = 0; i < m_numberDevices; ++i)
    {
        m_devicesID[i] = m_devicesInfo[i]->GetDeviceID();
    }

    m_innerContext = clCreateContext(0, m_numberDevices, m_devicesID, NULL, NULL, &ciErrNum);
    if(ciErrNum != CL_SUCCESS)
        qDebug()<<"Error on creating context.";
}

// Destroy the context
OCLContext::~OCLContext()
{
    for(int i = 0; i < m_devicesInfo.size(); ++i)
    {
        delete m_devicesInfo[i];
    }

    for(int i = 0; i < m_platformInfo.size(); ++i)
    {
        delete m_platformInfo[i];
    }

    if(m_devicesID != NULL)
    {
        delete[] m_devicesID;
    }

    if(clReleaseContext(m_innerContext) != CL_SUCCESS)
        qDebug()<<"Error on releasing OpenCL Context";
}