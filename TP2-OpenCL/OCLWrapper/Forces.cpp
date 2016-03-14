#include "Forces.h"

#include "Atom.h"
#include "AtomHelper.h"

#include "../OpenCL/OCLProgram.h"
#include "../OpenCL/OCLKernel.h"
#include "../OpenCL/OCLBuffer.h"

#include <QDebug>

#include <omp.h>

const double PI = 3.1415926535897932384626433832795;
const double SpacePermitivity = 0.0000885;//0.00000000000885;
const float k = 4.0 * PI * SpacePermitivity;

const float G = 0.00667f;//0.0000000000667f;

OCLProgram* Forces::m_oclProgram = NULL;
OCLKernel* Forces::m_forcesKernel = NULL;
float* Forces::m_atomsInfo = NULL;
float* Forces::m_forces = NULL;
size_t Forces::m_atomCount = 0;

void Forces::Initialize()
{
    m_oclProgram = new OCLProgram();
    if(m_oclProgram->CreateProgramFromSourceFile("OpenCL/ForcesCompute.cl"))
    {
        m_atomCount = Atom::GetInstances()->size();
        // allocate host vectors
        m_atomsInfo = new float[m_atomCount * 6 * sizeof(float)];
        m_forces = new float[m_atomCount * 6 * sizeof(float)];

        m_forcesKernel = m_oclProgram->CreateKernelFunction("computeForces");
        m_oclProgram->CreateBuffer("AtomsInfoBuffer", OCLBuffer::READ_ONLY, m_atomCount * 6 * sizeof(float));
        m_oclProgram->CreateBuffer("ForcesBuffer", OCLBuffer::WRITE_ONLY, m_atomCount * 6 * sizeof(float));
    }
    else
    {
        delete m_oclProgram;
        m_oclProgram = NULL;
    }    
}

void Forces::Uninitialize()
{
    // Kernel and buffer will be destroy with the program
    if(m_oclProgram)
        delete m_oclProgram;

    if(m_forces)
        delete[] m_forces;

    if(m_atomsInfo)
        delete[] m_atomsInfo;
}

void Forces::CalculateForces()
{
    if(m_oclProgram)
    {
        for(int i = 0; i < Atom::GetInstances()->size(); ++i)
        {
            Atom* current = (*Atom::GetInstances())[i];
            Vector3D position = *current->GetPosition();
            m_atomsInfo[(i * 6) + 0] = (float)position.x();
            m_atomsInfo[(i * 6) + 1] = (float)position.y();
            m_atomsInfo[(i * 6) + 2] = (float)current->GetAtomMass();
            m_atomsInfo[(i * 6) + 3] = (float)current->GetCharge();
            m_atomsInfo[(i * 6) + 4] = (float)current->GetVanDerWaalsRadius();
			m_atomsInfo[(i * 6) + 5] = (float)current->GetElectroNegativity();
        }

        int atomCount = static_cast<int>(m_atomCount);
        bool paramSet = m_forcesKernel->SetArgBuffer(0, "AtomsInfoBuffer");
        paramSet &= m_forcesKernel->SetArgBuffer(1, "ForcesBuffer");        
        paramSet &= m_forcesKernel->SetArgInt(2, atomCount);
        paramSet &= m_forcesKernel->SetArgFloat(3, (float)k);
        paramSet &= m_forcesKernel->SetArgFloat(4, (float)G);

        if(paramSet && m_oclProgram->WriteBuffer("AtomsInfoBuffer", m_atomsInfo))
        {
			size_t globalWorkSize[1];
			size_t localWorkSize[1];

			globalWorkSize[0] = m_atomCount;
			localWorkSize[0] = 0;
			//globalWorkSize[1] = 4;
			//localWorkSize[0] = m_atomCount / 14; // Tesla C2050 have 14 processors of 32 cores each, so, we have a total of 448 cores
			//localWorkSize[0] = 4;
            m_oclProgram->ExecuteKernel(m_forcesKernel, 1, globalWorkSize, 0 /*localWorkSize*/);
            if(m_oclProgram->ReadBuffer("ForcesBuffer", m_forces))
            {
                for(int i = 0; i < Atom::GetInstances()->size(); ++i)
                {
                    Atom* current = (*Atom::GetInstances())[i];
                    current->SetForces(Vector3D(m_forces[(i * 6) + 0], m_forces[(i * 6) + 1], 0.0));
                    current->SetCoulombicEnergy(m_forces[(i * 6) + 2]);
                    current->SetLennardJonesForce(Vector3D(m_forces[(i*6) + 3], m_forces[(i*6)+4], 0.0));
                    current->SetLennardJonesEnergy(m_forces[(i * 6) + 5]);
                }
            }
        }
        else
        {
            qDebug()<<"Error while setting kernel arguments.";
        }
    }
    /*else
    {
        int n = Atom::GetInstances()->size();
        int i;
        AtomList* atomsList = Atom::GetInstances();
        #pragma omp parallel for private(i) firstprivate(n, atomsList)
        for(i = 0; i < n; ++i)
        {
            Atom* a = (*atomsList)[i];
            Vector3D forceVector = Vector3D();

            for(int j = 0; j < n; ++j)
            {
                Atom* b = (*atomsList)[j];

                if(a == b)
                    continue;

                Vector3D r = AtomHelper::GetDistanceBetweenAtom(b, a);
                double rSq = Vector3D::dotProduct(r, r);

                if(rSq == 0.0)
                {
                    forceVector = Vector3D();
                    break;
                }
                double chargeProduct = a->GetCharge() * b->GetCharge();
                double force = chargeProduct / (k * rSq);
                forceVector += force * r.normalized();
            }

            a->SetForces(forceVector);
        }
    }*/
}

