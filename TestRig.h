#include "btBulletDynamicsCommon.h"
#include "GlutStuff.h"
#include "GL_ShapeDrawer.h"
#include "LinearMath/btIDebugDraw.h"
#include "GLDebugDrawer.h"
#include "organism.h"
#include "HexSim.h"
#include "tuning.h"

#define BODYPART_COUNT 3 * NUM_LEGS + 1
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
		rbInfo.m_friction = 1.0;
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
		btQuaternion q = m_bodies[0]->getOrientation();
		btVector3 v = q.getAxis();
		v.setY(0.0);
		float ang = q.getAngle();
		v = v * ang;
		return v.length();
	}

	// CONSTRUCTOR
	
	TestRig (btDynamicsWorld* ownerWorld) : m_ownerWorld (ownerWorld)
	{
		int ii;
		float tht, stht, ctht, r;
		btVector3 vUp(0, 1, 0);
		btVector3 pos(0,1.0,0); // origin of robot
		btTransform offset;
		btTransform transform;
		btHingeConstraint* hingeC;
		btTransform localA, localB, localC;
		
		// Setup geometry

		float bodyRadius = 1.5;
		float legSegment1 = 0.5;
		float legSegment2 = 0.7;
		float legSegment3 = 1.2;

		
		// main body shape (radius, height, 0)
		m_shapes[0] = new btCylinderShape(btVector3(bodyRadius,0.4,0.0));
		// add legs
		for (ii=0; ii<NUM_LEGS; ii++)
		{
			// radius, length
			m_shapes[3*ii+1] = new btCapsuleShape(0.3, legSegment1);
			m_shapes[3*ii+2] = new btCapsuleShape(0.3, legSegment2);
			m_shapes[3*ii+3] = new btCapsuleShape(0.2, legSegment3);
		}

		// Setup rigid bodies
		float fHeight = pos.getY();
		offset.setIdentity();
		offset.setOrigin(pos);		

		// root
		btVector3 vRoot = btVector3(btScalar(0.), btScalar(fHeight), btScalar(0.));
		transform.setIdentity();
		transform.setOrigin(vRoot);
		

		m_bodies[0] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[0]);
		// legs
		for (ii=0; ii<NUM_LEGS; ii++)
		{
			tht = 2.0 * PI * ii / NUM_LEGS;
			stht = sin(tht);
			ctht = cos(tht);

			// create transform for joint position
			transform.setIdentity();
			r = bodyRadius + 0.5*legSegment2;
			btVector3 vBoneOrigin = btVector3(ctht*r, fHeight, stht*r);
			transform.setOrigin(vBoneOrigin);

			btVector3 vToBone = (vBoneOrigin - vRoot).normalize();
			btVector3 vAxis = vToBone.cross(vUp);
			
			// 1
			transform.setIdentity();
			r = bodyRadius + 0.5*legSegment1;
			transform.setOrigin(btVector3(r*ctht, fHeight, r*stht));
			transform.setRotation(btQuaternion(vAxis, PI_2));
			m_bodies[1+3*ii] = localCreateRigidBody(0.2, offset*transform, m_shapes[1+3*ii]);

			// thigh
			transform.setIdentity();
			r = bodyRadius + legSegment1 + 0.5*legSegment2;
			transform.setOrigin(btVector3(r*ctht, fHeight, r*stht));
			transform.setRotation(btQuaternion(vAxis, PI_2));
			m_bodies[2+3*ii] = localCreateRigidBody(0.5, offset*transform, m_shapes[2+3*ii]);

			// shin
			transform.setIdentity();
			r = bodyRadius + legSegment1 + legSegment2 + 0.5*legSegment3;
			transform.setOrigin(btVector3(ctht*r, fHeight, stht*r));
			transform.setRotation(btQuaternion(vAxis, PI_2));
			m_bodies[3+3*ii] = localCreateRigidBody(0.5, offset*transform, m_shapes[3+3*ii]);
		}

		// Setup some damping on the m_bodies
		for (ii = 0; ii < BODYPART_COUNT; ++ii)
		{
			m_bodies[ii]->setDamping(0.01, 0.01);
			m_bodies[ii]->setDeactivationTime(0.8);
			m_bodies[ii]->setSleepingThresholds(0.01f, 0.01f);
		}


		// Setup the constraints
		for (ii=0; ii<NUM_LEGS; ii++)
		{
			tht = 2.0 * PI * ii / NUM_LEGS;
			stht = sin(tht);
			ctht = cos(tht);

			// 1
			localA.setIdentity();
			localB.setIdentity();
			localA.getBasis().setEulerZYX(0,-tht,0);
			r = bodyRadius;
			localA.setOrigin(btVector3(ctht*r, 0.0, stht*r));
			localB = m_bodies[1+3*ii]->getWorldTransform().inverse()
				* m_bodies[0]->getWorldTransform() * localA;
			hingeC = new btHingeConstraint(*m_bodies[0], *m_bodies[1+3*ii], localA, localB);
			hingeC->setLimit(SEG1_ANG_MIN, SEG1_ANG_MAX);
			btVector3 axis2 = btVector3(0.0,1.0,0.0);
			hingeC->setAxis(axis2);
			m_joints[3*ii] = hingeC;
			m_ownerWorld->addConstraint(m_joints[3*ii], true);


			// hip joints
			localA.setIdentity();
			localB.setIdentity();
			localC.setIdentity();
			localA.getBasis().setEulerZYX(0,-tht,0);
			r = bodyRadius + legSegment1;
			localA.setOrigin(btVector3(ctht*r, 0.0, stht*r));
			localB = m_bodies[1+3*ii]->getWorldTransform().inverse()
				* m_bodies[0]->getWorldTransform() * localA;
			localC = m_bodies[2+3*ii]->getWorldTransform().inverse()
				* m_bodies[0]->getWorldTransform() * localA;
			hingeC = new btHingeConstraint(*m_bodies[1+3*ii], *m_bodies[2+3*ii], localB, localC);
			hingeC->setLimit(SEG2_ANG_MIN, SEG2_ANG_MAX);
			m_joints[1+3*ii] = hingeC;
			m_ownerWorld->addConstraint(m_joints[1+3*ii], true);

			// knee joints
			localA.setIdentity();
			localB.setIdentity();
			localC.setIdentity();
			localA.getBasis().setEulerZYX(0,-tht,0);
			r = bodyRadius + legSegment1 + legSegment2;
			localA.setOrigin(btVector3(ctht*r, 0.0, stht*r));
			localB = m_bodies[2+3*ii]->getWorldTransform().inverse() * 
				m_bodies[0]->getWorldTransform() * localA;
			localC = m_bodies[3+3*ii]->getWorldTransform().inverse() * 
				m_bodies[0]->getWorldTransform() * localA;
			hingeC = new btHingeConstraint(*m_bodies[2+3*ii], *m_bodies[3+3*ii], localB, localC);
			hingeC->setLimit(SEG3_ANG_MIN, SEG3_ANG_MAX);
			m_joints[2+3*ii] = hingeC;
			m_ownerWorld->addConstraint(m_joints[2+3*ii], true);
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


