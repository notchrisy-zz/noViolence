#include "hooks.h"
#include "../globals.h"
#include "../options.hpp"
#include "../render/render.h"
#include "../helpers/console.h"
#include "../features/features.h"

namespace hooks
{
	blackbone::Detour<scene_end::fn> scene_end::hook;

	void __stdcall scene_end::hooked(IVRenderView*& thisptr)
	{
		if (!interfaces::engine_client->IsInGame())
			return;
	}
}