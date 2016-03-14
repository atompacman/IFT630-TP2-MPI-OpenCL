#ifndef _FORCES_H_
#define _FORCES_H_

class OCLProgram;
class OCLKernel;

class Forces
{
public:
    static void Initialize();
    static void Uninitialize();
    static void CalculateForces();

private:
    static OCLProgram* m_oclProgram;
    static OCLKernel* m_forcesKernel;
    static float* m_atomsInfo;
    static float* m_forces;
    static size_t m_atomCount;
};

#endif //_FORCES_H_