///////////////////////////////////////////////////////////////////////////////
//
// MiniFtLube.h
//
//  MiniFtLube is used in MiniFt systems that manage lube delivery, but do not have
//  a SmartDrill subsystem to manage lube delivery.
//  ConfigData parameters that control coolant/lube modulation ("Burster") control:
//  o   SpritzDurationMs    - Set to zero to deliver lube for the full drill cycle.
//  o   BurstOnDurationMs   - Set to zero to indicate burster valve not installed.
//                              Set > 0 to define how long the burst valve is open on each burst.
//  o   BurstIntervalMs     - Elapsed ms from start of one burst cycle to the start of the next burst.
//                          BurstInterval must be longer than BurstOnDuration.
//
//  Revision history:
//      2013-10-28  TC      Original coding for LFT 2.6, which runs Angstrom Linux on a Beagle Bone platform.
//
///////////////////////////////////////////////////////////////////////////////

//  FIXME: Determine actual output ports.
#define DIGOUT_CHANENTRY_COOLANT    DO_LUBE
#define DIGOUT_CHANENTRY_BURSTER    DO_LUBE_MOD

#define DIGOUT_OFF  0
#define DIGOUT_ON   1


//  If there is no coolant valve, or burster valve, set coolant state to NOT_INSTALLED.
//  If coolant / lube is managed by a SmartDrill subsystem, set the MiniFt ConfigData
//      values for CoolantState to NOT_INSTALLED and SpritzState to DISABLED.

enum{ COOLANT_NOT_INSTALLED = -1, COOLANT_OFF, COOLANT_ON } td_CoolantStates;
enum{ SPRITZ_DISABLED, SPRITZ_OFF, SPRITZ_ON } td_SpritzStates;

//  Prototypes of public functions

void CoolantStateSet( int iState );
int CoolantSpritzStateSet( int iSpritzCmd );
void CoolantUpdate( void );

