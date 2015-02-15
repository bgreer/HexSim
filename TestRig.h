#include "btBulletDynamicsCommon.h"
#include "GlutStuff.h"
#include "GL_ShapeDrawer.h"
#include "LinearMath/btIDebugDraw.h"
#include "GLDebugDrawer.h"
#include "organism.h"
#include "HexSim.h"
#include "tuning.h"

#define BODYPART_COUNT 2 * NUM_LEGS + 1
#define JOINT_COUNT BODYPART_COUNT - 1

class TestRig
{
	btDynamicsWorld *m_ownerWorld;
	btCollisionShape *m_shapes[BODYPART_COUNT];
	btRigidBody *m_bodies[BODYPART_COUNT];
	btTypedConstraint *m_joints[JOINT_COUNT];


	btRigidBody* localCreateRigidBody (btScalar mass, 
			const btTransform& startTransform, btCollisionShape* shape)
	{
		bool isDynamic;

		if (mass != 0.0) isDynamic = true;

		btVector3 localInertia(0,0,0);
		if (isDynamic)
			shape->calculateLocalInertia(mass,localInertia);

		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,
				shape,localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		m_ownerWorld->addRigidBody(body);

		return body;
	}


public:

	btVector3 getBodyPosition ()
	{
		return m_bodies[0]->getCenterOfMassPosition();
	}
	btVector3 getBodyVelocity ()
	{
		return m_bodies[0]->getLinearVelocity();
	}
	float getBodyAngle ()
	{
		return 0.0;//m_bodies[0]->getCenterOfMassTransform();
	}

	// CONSTRUCTOR
	
	TestRig (btDynamicsWorld* ownerWorld) : m_ownerWorld (ownerWorld)
	{
		int ii;
		btVector3 vUp(0, 1, 0);
		btVector3 pos(0,0.5,0);

		// Setup geometry
		float fBodySize  = 0.25f;
		float fLegLength = 0.45f;
		float fForeLegLength = 0.75f;
		
		// main body shape
		m_shapes[0] = new btCylinderShape(btVector3(0.2,0.1,0.0));
		// add legs
		for (ii=0; ii<NUM_LEGS; ii++)
		{
			m_shapes[2*ii+1] = new btCapsuleShape(btScalar(0.10), btScalar(fLegLength));
			m_shapes[2*ii+2] = new btCapsuleShape(btScalar(0.08), btScalar(fForeLegLength));
		}

		// Setup rigid bodies
		float fHeight = 0.5;
		btTransform offset;
		offset.setIdentity();
		offset.setOrigin(pos);		

		// root
		btVector3 vRoot = btVector3(btScalar(0.), btScalar(fHeight), btScalar(0.));
		btTransform transform;
		transform.setIdentity();
		transform.setOrigin(vRoot);
		m_bodies[0] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[0]);
		// legs
		for (ii=0; ii<NUM_LEGS; ii++)
		{
			float fAngle = 2.0 * PI * ii / NUM_LEGS;
			float fSin = sin(fAngle);
			float fCos = cos(fAngle);

			transform.setIdentity();
			btVector3 vBoneOrigin = btVector3(btScalar(fCos*(fBodySize+0.5*fLegLength)), btScalar(fHeight), btScalar(fSin*(fBodySize+0.5*fLegLength)));
			transform.setOrigin(vBoneOrigin);

			// thigh
			btVector3 vToBone = (vBoneOrigin - vRoot).normalize();
			btVector3 vAxis = vToBone.cross(vUp);			
			transform.setRotation(btQuaternion(vAxis, M_PI_2));
			m_bodies[1+2*ii] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[1+2*ii]);

			// shin
			transform.setIdentity();
			transform.setOrigin(btVector3(btScalar(fCos*(fBodySize+fLegLength)), btScalar(fHeight-0.5*fForeLegLength), btScalar(fSin*(fBodySize+fLegLength))));
			m_bodies[2+2*ii] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[2+2*ii]);
		}

		// Setup some damping on the m_bodies
		for (ii = 0; ii < BODYPART_COUNT; ++ii)
		{
			m_bodies[ii]->setDamping(0.01, 0.01);
			m_bodies[ii]->setDeactivationTime(0.8);
			m_bodies[ii]->setSleepingThresholds(0.01f, 0.01f);
		}


		// Setup the constraints
		btHingeConstraint* hingeC;
		//btConeTwistConstraint* coneC;

		btTransform localA, localB, localC;

		for (ii=0; ii<NUM_LEGS; ii++)
		{
			float fAngle = 2.0 * PI * ii / NUM_LEGS;
			float fSin = sin(fAngle);
			float fCos = cos(fAngle);

			// hip joints
			localA.setIdentity(); localB.setIdentity();
			localA.getBasis().setEulerZYX(0,-fAngle,0);	localA.setOrigin(btVector3(btScalar(fCos*fBodySize), btScalar(0.), btScalar(fSin*fBodySize)));
			localB = m_bodies[1+2*ii]->getWorldTransform().inverse() * m_bodies[0]->getWorldTransform() * localA;
			hingeC = new btHingeConstraint(*m_bodies[0], *m_bodies[1+2*ii], localA, localB);
			hingeC->setLimit(btScalar(-0.75 * PI_4), btScalar(PI_8));
			//hingeC->setLimit(btScalar(-0.1), btScalar(0.1));
			m_joints[2*ii] = hingeC;
			m_ownerWorld->addConstraint(m_joints[2*ii], true);

			// knee joints
			localA.setIdentity(); localB.setIdentity(); localC.setIdentity();
			localA.getBasis().setEulerZYX(0,-fAngle,0);	localA.setOrigin(btVector3(btScalar(fCos*(fBodySize+fLegLength)), btScalar(0.), btScalar(fSin*(fBodySize+fLegLength))));
			localB = m_bodies[1+2*ii]->getWorldTransform().inverse() * m_bodies[0]->getWorldTransform() * localA;
			localC = m_bodies[2+2*ii]->getWorldTransform().inverse() * m_bodies[0]->getWorldTransform() * localA;
			hingeC = new btHingeConstraint(*m_bodies[1+2*ii], *m_bodies[2+2*ii], localB, localC);
			//hingeC->setLimit(btScalar(-0.01), btScalar(0.01));
			hingeC->setLimit(btScalar(-PI_8), btScalar(0.2));
			m_joints[1+2*ii] = hingeC;
			m_ownerWorld->addConstraint(m_joints[1+2*ii], true);
		}
	}

	virtual	~TestRig ()
	{
		int i;

		// Remove all constraints
		for ( i = 0; i < JOINT_COUNT; ++i)
		{
			m_ownerWorld->removeConstraint(m_joints[i]);
			delete m_joints[i]; m_joints[i] = 0;
		}

		// Remove all bodies and shapes
		for ( i = 0; i < BODYPART_COUNT; ++i)
		{
			m_ownerWorld->removeRigidBody(m_bodies[i]);
			
			delete m_bodies[i]->getMotionState();

			delete m_bodies[i]; m_bodies[i] = 0;
			delete m_shapes[i]; m_shapes[i] = 0;
		}
	}

	btTypedConstraint** GetJoints() {return &m_joints[0];}

};


