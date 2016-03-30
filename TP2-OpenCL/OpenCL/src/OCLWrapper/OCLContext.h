#ifndef _OCL_CONTEXT_H_
#define _OCL_CONTEXT_H_

#include <CL/opencl.h>
#include <QList>
#include <QString>

class OCLPlatformInfo;

class OCLDeviceInfo
{
public:	

    OCLPlatformInfo* GetPlatformOwner() const { return m_platformOwner; }
	QString GetDeviceName() const { return m_deviceName; }
	uint GetComputeUnits() const { return m_computeUnits; }
	size_t GetMaxWorkGroupSize() const { return m_maxWorkGroupSize; }
	uint GetMaxWorkItemDimensions() const { return m_maxWorkItemDimensions; }
	ulong GetGlobalMemSize() const { return m_globalMemSize; }
	ulong GetLocalMemSize() const { return m_localMemSize; }

private:
    OCLDeviceInfo(OCLPlatformInfo* owner, cl_device_id deviceID);
    cl_device_id GetDeviceID() const { return m_deviceID; }
	cl_device_type GetDeviceType() const { return m_deviceType; }

	OCLDeviceInfo(const OCLDeviceInfo&);
	OCLDeviceInfo& operator=(const OCLDeviceInfo&);

    OCLPlatformInfo* m_platformOwner;
	cl_device_id m_deviceID;
	QString m_deviceName;
	uint m_computeUnits;
	size_t m_maxWorkGroupSize;
	uint m_maxWorkItemDimensions;
	ulong m_globalMemSize;
	ulong m_localMemSize;
	cl_device_type m_deviceType;

    friend class OCLContext;
    friend class OCLProgram;
    friend class OCLPlatformInfo;
};

class OCLPlatformInfo
{
public:

    QString GetPlatformName() const { return m_name; }
    QString GetPlatformVendor() const { return m_vendor; }
    QString GetPlatformVersion() const { return m_version; }
    QString GetPlatformExtensions() const { return m_extension; }
    int GetDeviceCount() const { return m_devices.size(); }
    const QList<OCLDeviceInfo*>* GetDevices() const { return &m_devices; }

private:
    OCLPlatformInfo(cl_platform_id platformID, cl_device_type type = CL_DEVICE_TYPE_ALL);
    cl_platform_id GetPlatformID() const { return m_platformID; }

	OCLPlatformInfo(const OCLPlatformInfo&);
	OCLPlatformInfo& operator=(const OCLPlatformInfo&);

    cl_platform_id m_platformID;
    QString m_name;
    QString m_vendor;
    QString m_extension;
    QString m_version;
    QList<OCLDeviceInfo*> m_devices;

    friend class OCLContext;
};

class OCLContext
{
public:
    OCLContext();
    OCLContext(cl_device_type deviceType);
    ~OCLContext();

	OCLDeviceInfo* GetDeviceInfo(int index) const { return m_devicesInfo[index]; }
    uint GetNumberOfDevices() const { return m_numberDevices; }

private:
    cl_context GetInnerContext() const { return m_innerContext; }
    cl_device_id* GetDevicesID()const { return m_devicesID; }

	OCLContext(const OCLContext&);
	OCLContext& operator=(const OCLContext&);

    cl_context m_innerContext;
    cl_device_id *m_devicesID;
	QList<OCLDeviceInfo*> m_devicesInfo;
    QList<OCLPlatformInfo*> m_platformInfo;
    uint m_numberDevices;

    friend class OCLProgram;
    friend class OCLBuffer;
};

#endif //_OCL_CONTEXT_H_
