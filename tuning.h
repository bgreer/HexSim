
#define PI       3.14159265358979323846
#define PI_2     1.57079632679489661923
#define PI_4     0.785398163397448309616
#define PI_8     0.39269908169

#define degToRad(X) (X*0.01745329251)

// physical body
#define NUM_LEGS 6
#define MAX_TORQUE 150.0
#define SEG1_ANG_MIN -PI_4
#define SEG1_ANG_MAX PI_4
#define SEG2_ANG_MIN -PI_4
#define SEG2_ANG_MAX PI_4
#define SEG3_ANG_MIN -PI_2
#define SEG3_ANG_MAX PI_2

// servos
#define SERVO_ANGLE_MAX degToRad(300.0)
#define SERVO_ANGLE_MIN degToRad(0.0)
//  in radians per second
#define SERVO_MAX_MOTORSPEED degToRad(354.0)
#define SERVO_MARGIN degToRad(2.0)

// brain
#define MIN_HISTORY 3
#define MAX_HISTORY 3

#define MIN_LAYERS 2
#define MAX_LAYERS 2

#define MIN_NODES 20
#define MAX_NODES 20
#define HISTORY_SKIP 15

// analysis
#define NUM_GENS 10
#define GEN_SIZE 10
#define STATS_SKIP 20
#define SELECTION_STRENGTH 1.0

// generation fractions
#define GEN_RANDOM 0.1
#define GEN_CLONES 0.1
#define GEN_BABIES 0.8
