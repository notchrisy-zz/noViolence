#include "hooks.h"
#include "../render/render.h"

namespace hooks 
{
	long __stdcall d3d9::reset::hooked(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params)
	{
		if (render::is_ready())
			render::device_lost();

		const auto hr = hook.get_original<fn>(index)(device, params);
		if (hr >= 0 && render::is_ready())
			render::device_reset();

		return hr;
	}
}