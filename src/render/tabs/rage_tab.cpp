#include "../render.h"
#include "../../globals.h"
#include "../../settings.h"
#include "../../features/features.h"
#include "../../helpers/console.h"
#include "../../helpers/imdraw.h"

extern void bind_button(const char* eng, const char* rus, int& key);
extern bool hotkey(const char* label, int* k, const ImVec2& size_arg = ImVec2(0.f, 0.f));

namespace render
{
	namespace menu
	{
		void rage_tab()
		{
			child("Aimbot", []()
			{

			});

			ImGui::NextColumn();

			child("Anti-Aim", []()
			{
			
			});

			ImGui::NextColumn();

			child("Extra", []()
			{
				checkbox("Resolver", u8"Постообработка", &settings::desync::resolver);
				checkbox("Bunny Hop", u8"Распрыжка", &settings::misc::bhop);
				checkbox("Auto Strafe", u8"Стрейфы", &settings::misc::auto_strafe);
				checkbox("Knife Bot", u8"Ножевой бот", &settings::misc::knife_bot);
			});
		}
	}
}
