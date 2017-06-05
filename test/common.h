#pragma once

// Enables detection of memory leaks (Debug only).
#define _CRTDBG_MAP_ALLOC 
#include <stdlib.h>
#include <crtdbg.h>

// Need this or else linking directX will fail (for some reason). 
#pragma comment(lib, "dxgi.lib") 

// Common includes
#include "types.h"