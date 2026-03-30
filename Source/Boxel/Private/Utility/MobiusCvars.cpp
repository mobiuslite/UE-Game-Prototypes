#include "Utility/MobiusCvars.h"

TAutoConsoleVariable<int32> CVDebugDrawGunfire(
	TEXT("cv.DebugDrawGunfire"),
	0,
	TEXT("Shows line trace for local gunfire"),
	ECVF_Cheat);