#ifndef HEXSIM_H
#define HEXSIM_H

#include "GlutDemoApplication.h"
#include "LinearMath/btAlignedObjectArray.h"
class btBroadphaseInterface;
class btCollisionShape;
class btOverlappingPairCache;
class btCollisionDispatcher;
class btConstraintSolver;
struct btCollisionAlgorithmCreateFunc;
class btDefaultCollisionConfiguration;

class HexSim : public GlutDemoApplication
{
	float currtime;
	organism *org;

	btAlignedObjectArray<class TestRig*> m_rigs;
	TestRig *rig;

	//keep the collision shapes, for deletion/cleanup
	btAlignedObjectArray<btCollisionShape*>	m_collisionShapes;

	btBroadphaseInterface*	m_broadphase;

	btCollisionDispatcher*	m_dispatcher;

	btConstraintSolver*	m_solver;

	btDefaultCollisionConfiguration* m_collisionConfiguration;

public:
	void stepSimulation(float a, int b);
	void initPhysics();
	void attachOrganism(organism *bob);
	void exitPhysics();
	TestRig* getRig();
	void computeStats (btScalar timeStep);
	double getFitness ();
	void reset();

	double fitness;
	bool converged;

	virtual ~HexSim()
	{
		exitPhysics();
	}

	void spawnTestRig();

	virtual void clientMoveAndDisplay();

	virtual void displayCallback();

	virtual void keyboardCallback(unsigned char key, int x, int y);

	static DemoApplication* Create()
	{
		HexSim* sim = new HexSim();
		sim->myinit();
		sim->initPhysics();
		return sim;
	}
	
	void setMotorTargets(btScalar deltaTime);

};


#endif
