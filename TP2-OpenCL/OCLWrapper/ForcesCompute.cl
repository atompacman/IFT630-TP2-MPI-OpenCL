// atomsInfo + 0 = atomPosition.x
// atomsInfo + 1 = atomPosition.y
// atomsInfo + 2 = atom mass
// atomsInfo + 3 = atom charge
// atomsInfo + 4 = atom van der Waal radius
// atomsInfo + 5 = atom electro-negativity

// forces + 0 = resultForce.x
// forces + 1 = resultForce.y
// forces + 2 = coulombic energy
// forces + 3 = Lennard-Jones force : X
// forces + 4 = Lennard-Jones force : Y
// forces + 5 = Lennard-Jones energy

// k = Electromagnetic constant
// G = Gravitanional constant
__kernel void computeForces(__constant const float * atomsInfo, __global float * forces, int atomCount, float k, float G)
{
    unsigned int bIndex = get_global_id(0);
    unsigned int nIndex = bIndex * 6;
    unsigned int forceIndex = bIndex * 6;
    
    // Retrieve current atom informations
    float2 currentPos   = (float2)(atomsInfo[nIndex + 0], atomsInfo[nIndex + 1]);
    float currentMass   = atomsInfo[nIndex + 2];
    float currentCharge = atomsInfo[nIndex + 3];
    float currentVdWRadius = atomsInfo[nIndex + 4];
    float currentElectroNeg = atomsInfo[nIndex + 5];
    
    // Init forces vectors and energy counters
    float2 totalForcesVec = (float2)(0.0f);
    float2 coulombForceVector = (float2)(0.0f);
    float2 lennardJonesForceVector = (float2)(0.0f);
    float2 gravitationalForceVector = (float2)(0.0f);
    float coulombicEnergy = 0.0f;
    float lennardJonesEnergy = 0.0f;
    float gravitationalEnergy = 0.0f;
    
    for(int i = 0; i < atomCount * 6; i += 6)
    {
        if(i == nIndex)
        {
            continue;
        }
        
        float2 withPos = (float2)(atomsInfo[i + 0], atomsInfo[i + 1]);
        float withMass   = atomsInfo[i + 2];
        float withVdWRadius = atomsInfo[i + 4];
        
        float2 rV = currentPos - withPos;
        float rSq = dot(rV, rV);
        if(rSq == 0.0)
        {
            continue;
        }
        
        float epsilon = 0.2f * ((0.5f * (currentElectroNeg + atomsInfo[i + 5])) / 3.44f);

        float r = length(rV);
        float r2 = rSq;
        float r4 = r2 * r2;
        float r8 = r4 * r4;
        float r6 = r2 * r2 * r2;
        float r12 = r6 * r6;
        float r14 = r12 * r2;
        float sigma1 = currentVdWRadius + withVdWRadius;
        float sigma2  = sigma1 * sigma1;
        float sigma4  = sigma2 * sigma2;
        float sigma6  = sigma2 * sigma2 * sigma2;
        float sigma8  = sigma4 * sigma4;
        float sigma12 = sigma6 * sigma6;
        float sigma14 = sigma12 * sigma2;
        
        float2 rNorm = normalize(rV);
        
        // Coulombic force and energy compute
        float chargeProduct = currentCharge * atomsInfo[i + 3];
        coulombForceVector += (chargeProduct / (k * r2)) * rNorm;
        coulombicEnergy += 0.5f * (chargeProduct / (k * r));
        
        // Lennard-Jones force and energy compute
        //lennardJonesForceVector += ((24.0f * epsilon / sigma1) * (2.0f*(sigma14 / r14) - (sigma8 / r8))) * rNorm;
        //lennardJonesEnergy += (2.0f * epsilon * ((sigma12 / r12) - (sigma6 / r6)));
        //lennJonesForce = lennJonesForce + ljForce;
        
        // Gravitationnal force and energy compute
        //gravitationalForceVector += (-G * currentMass * withMass / r2) * rNorm;
        
        // Total force compute
        //totalForcesVec = totalForcesVec + electricForce + ljForce;// + gravitationalForce;
    }
    
    float2 totalForces = coulombForceVector + lennardJonesForceVector + gravitationalForceVector;
    forces[forceIndex + 0] = totalForces.x;
    forces[forceIndex + 1] = totalForces.y;
    forces[forceIndex + 2] = coulombicEnergy;
    forces[forceIndex + 3] = lennardJonesForceVector.x;
    forces[forceIndex + 4] = lennardJonesForceVector.y;
    forces[forceIndex + 5] = lennardJonesEnergy;
}