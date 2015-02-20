#include "btBulletDynamicsCommon.h"

class servo
{
	// reset positions (new sim)
	// attach to hinge class for direct control
	
public:
	float target;
	btHingeConstraint *hinge;

	// CONSTRUCTOR
	servo (btHingeConstraint *h)
	{
		hinge = h;
		target = hinge->getHingeAngle();
	}

	// DESTRUCTOR
	~servo ()
	{

	}

	void reset (btHingeConstraint *h)
	{
		hinge = h;
		target = hinge->getHingeAngle();
	}

	void setMotorSpeed ()
	{
		hinge->enableAngularMotor(true, getMotorSpeed(), MAX_TORQUE);
	}

	float getMotorSpeed ()
	{
		float ret, pos;

		// get current position
		pos = getPosition();

		if (fabs(target-pos) > SERVO_MARGIN) // dont move if we are close
		{
			ret = (target-pos)*SERVO_MAX_MOTORSPEED/degToRad(10.0);
		} else {
			ret = 0.0;
		}

		if (ret > SERVO_MAX_MOTORSPEED) ret = SERVO_MAX_MOTORSPEED;
		if (ret < -SERVO_MAX_MOTORSPEED) ret = -SERVO_MAX_MOTORSPEED;
		return ret;
	}

	void setTarget (float angle)
	{
		if (angle < SERVO_ANGLE_MIN) target = SERVO_ANGLE_MIN;
		else if (angle > SERVO_ANGLE_MAX) target = SERVO_ANGLE_MAX;
		else target = angle;
	}

	float getPosition ()
	{
		// for a real servo, read from device itself?
		return hinge->getHingeAngle()+PI;
	}
};
