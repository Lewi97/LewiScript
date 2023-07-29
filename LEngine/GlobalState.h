#pragma once

#include "MemoryManager.h"

/* Objects that are better held in a global namespace */
namespace le::global
{
	/* Better to place it here than give every object its own pointer to it */
	extern MemoryManager* mem;
}
