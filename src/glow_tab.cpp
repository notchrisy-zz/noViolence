#include "render/render.h"
#include "globals.h"
#include "settings.h"
#include "helpers/imdraw.h"
#include "helpers/console.h"

extern void bind_button(const char* eng, const char* rus, int& key);
extern bool hotkey(const char* label, int* k, const ImVec2& size_arg = ImVec2(0.f, 0.f));

namespace render
{
	namespace menu
	{
		void glow_tab()
		{
			
			child(___("Players", u8"Цветные Модели"), []()
			{
				checkbox("Enemy", &settings::glow::GlowEnemyEnabled);
				checkbox("Team", &settings::glow::GlowTeamEnabled);
			
				separator("Misc", u8"Руки");

				checkbox("Planted C4", &settings::glow::GlowC4PlantedEnabled);
				checkbox("Grenades", &settings::glow::GlowNadesEnabled);
				checkbox("Dropped Weapons", &settings::glow::GlowDroppedWeaponsEnabled);

				separator("Players - Colors", u8"Руки");

				ImGui::ColorEdit4("Enemy - Color", settings::glow::GlowEnemy);
				ImGui::ColorEdit4("Teammate - Color", settings::glow::GlowTeam);
				
				separator("Misc - Colors", u8"Руки");

				ImGui::ColorEdit4("Planted C4", settings::glow::GlowC4Planted);
				ImGui::ColorEdit4("Grenades", settings::glow::GlowNades);
				ImGui::ColorEdit4("Dropped Weapons", settings::glow::GlowDroppedWeapons);
			});
		}
	}
}