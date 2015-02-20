#include "btBulletDynamicsCommon.h"
#include "GlutStuff.h"
#include "GL_ShapeDrawer.h"
#include "LinearMath/btIDebugDraw.h"
#include "GLDebugDrawer.h"
#include "organism.h"
#include "HexSim.h"
#include "TestRig.h"
#include "tuning.h"

void vertex(btVector3 &v)
{
	glVertex3d(v.getX(), v.getY(), v.getZ());
}

void drawFrame(btTransform &tr)
{
	const float fSize = 1.f;

	glBegin(GL_LINES);

	// x
	glColor3f(255.f,0,0);
	btVector3 vX = tr*btVector3(fSize,0,0);
	vertex(tr.getOrigin());	vertex(vX);

	// y
	glColor3f(0,255.f,0);
	btVector3 vY = tr*btVector3(0,fSize,0);
	vertex(tr.getOrigin());	vertex(vY);

	// z
	glColor3f(0,0,255.f);
	btVector3 vZ = tr*btVector3(0,0,fSize);
	vertex(tr.getOrigin());	vertex(vZ);

	glEnd();
}

// CALLBACK called before the simulation takes a setp
void motorPreTickCallback (btDynamicsWorld *world, btScalar timeStep)
{
	HexSim* sim = (HexSim*)world->getWorldUserInfo();
	sim->setMotorTargets(timeStep);
}
void	postTickCallback(btDynamicsWorld *world, btScalar timeStep)
{
	static HexSim *sim = (HexSim*) world->getWorldUserInfo();
	sim->computeStats(timeStep);
}

double satFunc (double val, double satval)
{
	return atan(val*2./satval)/PI_2;
}

void HexSim::computeStats (btScalar timeStep)
{	
	static long iter = 0;
	static btVector3 lastpos, lastvel;
	btVector3 pos, vel;
	float angle;

	currtime += timeStep;
	pos = rig->getBodyPosition();
	vel = rig->getBodyVelocity();
	angle = rig->getBodyAngle();

	// compute update to fitness
//	fitness += sqrt(pow(vel.getX(),2.0)+pow(vel.getZ(),2.0));
	fitness += 0.01 * satFunc(vel.getZ(),10.0) * satFunc(pos.getY(),0.5) * (PI_2-angle);
	cout << angle << endl;

	// only print stats every now and then
	if (iter % STATS_SKIP == 0)
	{
		
		// check convergence
		// if oranism can't move at all, gets eaten quickly
		if (currtime > 5.0 && pos.getZ()/currtime < 0.2)
			converged = true;
	}

	// store values for next stats check
	lastpos = pos;
	lastvel = vel;
	iter ++;
}

double HexSim::getFitness ()
{
	// thisAlgorithmBecomingSkynetCost = 999999999;
	return fitness;
}

TestRig* HexSim::getRig ()
{
	return rig;
}

void HexSim::attachOrganism(organism *bob)
{
	org = bob; // simple enough
}

void HexSim::stepSimulation(float a, int b)
{
	m_dynamicsWorld->stepSimulation(a, b);
}

void HexSim::initPhysics()
{
	setTexturing(true);
	setShadows(true);

	// Setup the basic world

	fitness = 0.0;
	currtime = 0.0;
	converged = false;

	setCameraDistance(btScalar(15.));

	m_collisionConfiguration = new btDefaultCollisionConfiguration();

	m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);

	btVector3 worldAabbMin(-10000,-10000,-10000);
	btVector3 worldAabbMax(10000,10000,10000);
	m_broadphase = new btAxisSweep3 (worldAabbMin, worldAabbMax);

	m_solver = new btSequentialImpulseConstraintSolver;

	m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher,m_broadphase,m_solver,m_collisionConfiguration);
	m_dynamicsWorld->setGravity(btVector3(0,-98.1,0));
	m_dynamicsWorld->getSolverInfo().m_numIterations = 50;
	m_dynamicsWorld->setInternalTickCallback(motorPreTickCallback,this,true);
	m_dynamicsWorld->setInternalTickCallback(postTickCallback,this,false);

	// Setup a big ground box
	{
		btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(200.),btScalar(10.),btScalar(200.)));
		m_collisionShapes.push_back(groundShape);
		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0,-10,0));
		localCreateRigidBody(btScalar(0.),groundTransform,groundShape);
	}


	// Spawn one ragdoll
	spawnTestRig();

	clientResetScene();		
}


void HexSim::spawnTestRig()
{
	rig = new TestRig(m_dynamicsWorld);
}

void HexSim::reset ()
{
	delete rig;
	converged = false;
	currtime = 0.0;
	fitness = 0.0;
	rig = new TestRig(m_dynamicsWorld);
}


void HexSim::setMotorTargets(btScalar deltaTime)
{
	int ii;
	float motorspeed;
	btHingeConstraint *hinge;

	// set nn inputs;
	for (ii=0; ii<3*NUM_LEGS; ii++)
	{
		btHingeConstraint* hingeC = 
			static_cast<btHingeConstraint*>(rig->GetJoints()[ii]);
		btScalar fCurAngle = hingeC->getHingeAngle();
		org->inputs[ii] = ((float)fCurAngle);
	}
	org->computeOutputs();
	
	// set leg motors
	for (int i=0; i<3*NUM_LEGS; i++)
	{
		hinge = static_cast<btHingeConstraint*>(rig->GetJoints()[i]);
		btScalar fCurAngle      = hinge->getHingeAngle();
		// nn output
		motorspeed = (org->outputs[i]-0.5)*8.0;

		hinge->enableAngularMotor(true, motorspeed, MAX_TORQUE);
	}
	
}

void HexSim::clientMoveAndDisplay()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

	//simple dynamics world doesn't handle fixed-time-stepping
	float deltaTime = getDeltaTimeMicroseconds()/1000000.f;
	

	if (m_dynamicsWorld)
	{
		m_dynamicsWorld->stepSimulation(deltaTime);
		m_dynamicsWorld->debugDrawWorld();
	}

	renderme(); 

	for (int i=2; i>=0 ;i--)
	{
		btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		drawFrame(body->getWorldTransform());
	}

	glFlush();

	glutSwapBuffers();
}

void HexSim::displayCallback()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

	if (m_dynamicsWorld)
		m_dynamicsWorld->debugDrawWorld();

	renderme();

	glFlush();
	glutSwapBuffers();
}


// pass keyboard events to default demo stuff
void HexSim::keyboardCallback(unsigned char key, int x, int y)
{
	DemoApplication::keyboardCallback(key, x, y);
}

void HexSim::exitPhysics()
{
	int i;

	// delete physical body
	delete rig;

	for (i=m_dynamicsWorld->getNumCollisionObjects()-1; i>=0 ;i--)
	{
		btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		m_dynamicsWorld->removeCollisionObject( obj );
		delete obj;
	}

	//delete collision shapes
	for (int j=0;j<m_collisionShapes.size();j++)
	{
		btCollisionShape* shape = m_collisionShapes[j];
		delete shape;
	}

	delete m_dynamicsWorld;
	delete m_solver;
	delete m_broadphase;
	delete m_dispatcher;
	delete m_collisionConfiguration;	
}



