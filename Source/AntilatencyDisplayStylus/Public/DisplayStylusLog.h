#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(DisplayStylusLog, Log, All);

//Uncomment to view additional log messages related to ADN module
#define DISPLAYSTYLUSLOG

#ifdef DISPLAYSTYLUSLOG
#define DISPLAYSTYLUS_LOG(verbosity, message, ...) UE_LOG(DisplayStylusLog, verbosity, message, ##__VA_ARGS__)
#else
#define DISPLAYSTYLUS_LOG(verbosity, message) 
#endif
