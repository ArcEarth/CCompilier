typedef struct _btVector3{
	float x,y,z;
} btVector3;

typedef struct _btTransform
{
	int x;
} btTransform;

typedef struct _btQuaternion
{
	float x,y,z,w;
} btQuaternion;

typedef float btScalar;


void  applyGravity ();

void  setGravity (const btVector3 &acceleration);

const btVector3 &  getGravity () const;

void  setDamping (btScalar lin_damping, btScalar ang_damping);

btScalar  getLinearDamping () const;

btScalar  getAngularDamping () const;

btScalar  getLinearSleepingThreshold () const;

btScalar  getAngularSleepingThreshold () const;

void  applyDamping (btScalar timeStep);

void  setMassProps (btScalar mass, const btVector3 &inertia);

const btVector3 &  getLinearFactor () const;

void  setLinearFactor (const btVector3 &linearFactor);

btScalar  getInvMass () const;

void  integrateVelocities (btScalar step);

void  setCenterOfMassTransform (const btTransform &xform);

void  applyCentralForce (const btVector3 &force);

const btVector3 &  getTotalForce () const;

const btVector3 &  getTotalTorque () const;

const btVector3 &  getInvInertiaDiagLocal () const;

void  setInvInertiaDiagLocal (const btVector3 &diagInvInertia);

void  setSleepingThresholds (btScalar linear, btScalar angular);

void  applyTorque (const btVector3 &torque);

void  applyForce (const btVector3 &force, const btVector3 &rel_pos);

void  applyCentralImpulse (const btVector3 &impulse);

void  applyTorqueImpulse (const btVector3 &torque);

void  applyImpulse (const btVector3 &impulse, const btVector3 &rel_pos);

void  clearForces ();

void  updateInertiaTensor ();

const btVector3 &  getCenterOfMassPosition () const;

btQuaternion  getOrientation () const;

const btTransform &  getCenterOfMassTransform () const;

const btVector3 &  getLinearVelocity () const;

const btVector3 &  getAngularVelocity () const;

void  setLinearVelocity (const btVector3 &lin_vel);

void  setAngularVelocity (const btVector3 &ang_vel);

btVector3  getVelocityInLocalPoint (const btVector3 &rel_pos) const;

void  translate (const btVector3 &v);

void  getAabb (btVector3 &aabbMin, btVector3 &aabbMax) const;

btScalar  computeImpulseDenominator (const btVector3 &pos, const btVector3 &normal) const;

btScalar  computeAngularImpulseDenominator (const btVector3 &axis) const;

void  updateDeactivation (btScalar timeStep);

bool  wantsSleeping () ;

void  setAngularFactor (const btVector3 &angFac) 
;
void  setAngularFactor (btScalar angFac) 
;
const btVector3 &  getAngularFactor () const;

bool  isInWorld () const;

int main()
{

}