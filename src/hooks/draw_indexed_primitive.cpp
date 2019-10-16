#include "hooks.h"
#include "../options.hpp"
#include "../helpers/console.h"

namespace chams
{
	IDirect3DPixelShader9* pixel_shader;
	IDirect3DPixelShader9* old_pixel_shader;

	void* traverse_stack(void** ebp)
	{
		static void* draw_points_ret_addr = nullptr;
		if (!draw_points_ret_addr)
			draw_points_ret_addr = static_cast<void*>(utils::pattern_scan(DRAW_POINTS));

		if (ebp == nullptr)
			return nullptr;

		auto** next = *reinterpret_cast<void***>(ebp);
		if (ebp[1] == draw_points_ret_addr)
			return next[4];

		return traverse_stack(next);
	}

	bool is_enabled()
	{
		if (!settings::chams::enabled || !interfaces::local_player)
			return false;

		auto data = 0;
		__asm
		{
			mov data, ebp
		}

		const auto* ent = traverse_stack(reinterpret_cast<void**>(data));
		if (!ent)
			return false;

		const auto team_num = *reinterpret_cast<int*>(uintptr_t(ent) + 0xF4);
		const auto health = *reinterpret_cast<int*>(uintptr_t(ent) + 0x100);
		if (!team_num || (team_num != team::team_ct && team_num != team::team_t) || !(health > 0))
			return false;

		return interfaces::local_player->m_iTeamNum() != team_num || settings::misc::deathmatch;
	}

	void player(LPDIRECT3DDEVICE9 device, const bool& ignore_z, std::function<void(void)> original)
	{
		DWORD old_zenable;
		DWORD old_fillmode;

		const auto color = ignore_z ? settings::chams::occluded_color : settings::chams::visible_color;

		if (settings::chams::flat && !pixel_shader)
		{
			ID3DXBuffer* pShaderBuf = NULL;
			char szShader[] = "ps_1_3 \nmov r0, c0 \n";

			if (SUCCEEDED(D3DXAssembleShader(szShader, sizeof(szShader), NULL, NULL, 0, &pShaderBuf, NULL)))
			{
				device->CreatePixelShader((const DWORD*)pShaderBuf->GetBufferPointer(), &pixel_shader);
				pShaderBuf->Release();
			}
		}

		if (settings::chams::wireframe)
		{
			device->GetRenderState(D3DRS_FILLMODE, &old_fillmode);
			device->SetRenderState(D3DRS_FILLMODE, _D3DFILLMODE::D3DFILL_WIREFRAME);
		}

		if (ignore_z)
		{
			device->GetRenderState(D3DRS_ZENABLE, &old_zenable);
			device->SetRenderState(D3DRS_ZENABLE, false);
		}

		if (settings::chams::flat)
		{
			device->GetPixelShader(&old_pixel_shader);

			float color_shader[4] =
			{
				std::clamp<float>(color[0] / 255.f, 0.f, 1.f),
				std::clamp<float>(color[1] / 255.f, 0.f, 1.f),
				std::clamp<float>(color[2] / 255.f, 0.f, 1.f),
				1.f
			};

			device->SetPixelShader(pixel_shader);
			device->SetPixelShaderConstantF(0, color_shader, 1);
		}
		else
		{
			float color_shader[4] =
			{
				std::clamp<float>(color[0] / 12.f, 0.f, 255.f),
				std::clamp<float>(color[1] / 12.f, 0.f, 255.f),
				std::clamp<float>(color[2] / 12.f, 0.f, 255.f),
				255.f
			};

			device->SetPixelShaderConstantF(1, color_shader, 1);
		}

		original();

		if (settings::chams::flat && old_pixel_shader)
			device->SetPixelShader(old_pixel_shader);

		if (ignore_z)
			device->SetRenderState(D3DRS_ZENABLE, old_zenable);

		if (settings::chams::wireframe)
			device->SetRenderState(D3DRS_FILLMODE, old_fillmode);
	}
}

namespace hooks
{
	long __stdcall d3d9::draw_indexed_primitive::hooked(LPDIRECT3DDEVICE9 device, D3DPRIMITIVETYPE type, INT base_vertex_index, UINT min_index, UINT num_vertices, UINT start_index, UINT prim_count)
	{
		static auto original = hook.get_original<fn>(index);

		auto original_fn = [device, type, base_vertex_index, min_index, num_vertices, start_index, prim_count]()
		{
			return original(interfaces::d3_device, device, type, base_vertex_index, min_index, num_vertices, start_index, prim_count);
		};

		if (!chams::is_enabled())
		{
			original_fn();
			return true;
		}

		IDirect3DPixelShader9* old_shader;
		device->GetPixelShader(&old_shader);
	
		float shader0[4];
		device->GetPixelShaderConstantF(0, shader0, 1);

		float shader1[4];
		device->GetPixelShaderConstantF(1, shader1, 1);

		if (!settings::chams::visible_only)
			chams::player(device, true, original_fn);

		chams::player(device, false, original_fn);

		device->SetPixelShaderConstantF(0, shader0, 1);
		device->SetPixelShaderConstantF(1, shader1, 1);

		device->SetPixelShader(old_shader);

		return true;
	}
}