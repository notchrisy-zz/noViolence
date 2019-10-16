#include "hooks.h"
#include "../globals.h"
#include "../options.hpp"
#include "../imgui/imgui.h"
#include "../render/render.h"
#include "../helpers/console.h"
#include "../helpers/notifies.h"
#include "../features/features.h"

#include <intrin.h>

namespace hooks 
{
	void handle(IDirect3DDevice9* device)
	{
		if (!render::is_ready())
			return;

		ImGui_ImplDX9_NewFrame();

		ImGui::GetIO().MouseDrawCursor = render::menu::is_visible();

		globals::draw_list = ImGui::GetOverlayDrawList();

		render::menu::show();
		render::timers::show();
		notifies::handle(globals::draw_list);
		render::spectators::show();
		grenade_prediction::render(globals::draw_list);
		offscreen_entities::render(globals::draw_list);
		esp::render(globals::draw_list);
		visuals::render(globals::draw_list);

		ImGui::Render();

		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	long __stdcall d3d9::end_scene::hooked(IDirect3DDevice9* device)
	{
		IDirect3DStateBlock9* pixel_state = NULL; 
		IDirect3DVertexDeclaration9* vertDec; 
		IDirect3DVertexShader9* vertShader;

		device->CreateStateBlock(D3DSBT_PIXELSTATE, &pixel_state);
		device->GetVertexDeclaration(&vertDec);
		device->GetVertexShader(&vertShader);

		device->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);

		handle(device);

		pixel_state->Apply();
		pixel_state->Release();
		device->SetVertexDeclaration(vertDec);
		device->SetVertexShader(vertShader);

		return hook.get_original<fn>(index)(device);
	}
}