#include "csgostructs.hpp"
#include "../helpers/math.h"
#include "../helpers/utils.h"
#include "../helpers/console.h"

using fnUpdateAnimState = void(__vectorcall*)(void*, void*, float, float, float, void*);

int c_base_entity::GetSequenceActivity(const int& sequence)
{
	auto hdr = interfaces::mdl_info->GetStudiomodel(GetModel());
	if (!hdr)
		return -1;

	static const auto offset = utils::pattern_scan(GET_SEQUENCE_ACTIVITY);
	assert(offset);
	static auto get_sequence_activity = reinterpret_cast<int(__fastcall*)(void*, studiohdr_t*, int)>(offset);

	return get_sequence_activity(this, hdr, sequence);
}

int filterException(int code, PEXCEPTION_POINTERS ex) 
{
	return EXCEPTION_EXECUTE_HANDLER;
}

uint8_t* UpdateVisibilityAllEntitiesOffset = nullptr;

void c_base_entity::UpdateVisibilityAllEntities()
{
	if (!UpdateVisibilityAllEntitiesOffset)
		UpdateVisibilityAllEntitiesOffset = utils::pattern_scan(UPDATE_VISIBILITY_ENTITIES);

	__try
	{
		auto fn_offset = reinterpret_cast<void(__thiscall*)(void*)>(UpdateVisibilityAllEntitiesOffset);

		fn_offset(this);
	}
	__except (filterException(GetExceptionCode(), GetExceptionInformation()))
	{
#ifdef _DEBUG
		console::print("[error] UpdateVisibilityAllEntities");
#endif
	}
}

const matrix3x4_t& c_base_entity::m_rgflCoordinateFrame()
{
	return *(matrix3x4_t*)(uintptr_t(this) + 0x444);
}

Vector& c_base_entity::GetAbsOrigin()
{
	return CallVFunction<Vector&(__thiscall*)(void*)>(this, 10)(this);
}

void c_base_entity::SetAbsOrigin(const Vector& origin)
{
	static const auto offset = utils::pattern_scan(SET_ABS_ORIGIN);
	if (offset)
	{
		static auto set_origin_fn = reinterpret_cast<void(__thiscall*)(void*, const Vector&)>(offset);

		set_origin_fn(this, origin);
	}
}

QAngle& c_base_player::GetAbsAngles()
{
	return CallVFunction<QAngle&(__thiscall*)(void*)>(this, 11)(this);
}

void c_base_player::SetAbsAngles(const QAngle& angles)
{
	static const auto offset = utils::pattern_scan(SET_ABS_ANGLE);
	if (offset)
	{
		static auto set_angle_fn = reinterpret_cast<void(__thiscall*)(void*, const QAngle&)>(offset);

		set_angle_fn(this, angles);
	}
}

bool c_base_entity::is_dormant()
{
	return GetClientNetworkable()->IsDormant();
}

float c_base_combat_weapon::GetInaccuracy()
{
	return CallVFunction<float(__thiscall*)(void*)>(this, 476)(this);
}

float c_base_combat_weapon::GetSpread()
{
	return CallVFunction<float(__thiscall*)(void*)>(this, 446)(this);
}

void c_base_combat_weapon::UpdateAccuracyPenalty()
{
	CallVFunction<void(__thiscall*)(void*)>(this, 478)(this);
}

void c_base_player::PVSFix()
{
	*reinterpret_cast<int*>(uintptr_t(this) + 0xA30) = interfaces::global_vars->framecount;
	*reinterpret_cast<int*>(uintptr_t(this) + 0xA28) = 0;
}

CCSPlayerAnimState* c_base_player::GetPlayerAnimState()
{
	//BE A4 3A ? ? ? 74 48 8B 8E ? ? + 0xA
	return *reinterpret_cast<CCSPlayerAnimState**>(DWORD(this) + 0x3900);
}

void c_base_player::InvalidateBoneCache()
{
	static auto invalidate_bone_cache = utils::pattern_scan(INVALIDATE_BONE_CACHE);
	if (invalidate_bone_cache)
	{
		const auto model_bone_counter = **reinterpret_cast<unsigned long**>(invalidate_bone_cache + 10);
		*reinterpret_cast<unsigned int*>(DWORD(this) + 0x2924) = 0xFF7FFFFF; // m_flLastBoneSetupTime = -FLT_MAX;
		*reinterpret_cast<unsigned int*>(DWORD(this) + 0x2690) = model_bone_counter - 1; // m_iMostRecentModelBoneCounter = g_iModelBoneCounter - 1;
	}
}

void c_base_player::UpdateClientSideAnimation()
{
	CallVFunction<void(__thiscall*)(void*)>(this, 219)(this);
}

bool c_base_entity::IsPlayer()
{
	//return GetClientClass()->m_ClassID == EClassId::CCSPlayer;
	return CallVFunction<bool(__thiscall*)(void*)>(this, 155)(this);
}

bool c_base_entity::IsWeapon()
{
	return CallVFunction<bool(__thiscall*)(c_base_entity*)>(this, 163)(this);
}

bool c_base_entity::IsPlantedC4()
{
	return GetClientClass()->m_ClassID == CPlantedC4;
}

bool c_base_entity::IsDefuseKit()
{
	return GetClientClass()->m_ClassID == CBaseAnimating;
}

CCSWeaponInfo* c_base_combat_weapon::get_weapon_data()
{
	//return interfaces::weapon_system->GetWpnData(m_iItemDefinitionIndex());
	return CallVFunction<CCSWeaponInfo*(__thiscall*)(void*)>(this, 454)(this);
}

bool c_base_combat_weapon::HasBullets()
{
	return !IsReloading() && m_iClip1() > 0;
}

bool c_base_combat_weapon::CanFire()
{
	if (!interfaces::local_player)
		return false;

	static auto stored_tick = 0;
	static decltype(this) stored_weapon = nullptr;
	if (stored_weapon != this || stored_tick >= interfaces::local_player->m_nTickBase()) 
	{
		stored_weapon = this;
		stored_tick = interfaces::local_player->m_nTickBase();

		return false;
	}

	if (IsReloading() || m_iClip1() <= 0 || !interfaces::local_player)
		return false;

	auto flServerTime = interfaces::local_player->m_nTickBase() * interfaces::global_vars->interval_per_tick;

	return m_flNextPrimaryAttack() <= flServerTime;
}

bool c_base_combat_weapon::IsGrenade()
{
	return get_weapon_data()->WeaponType == WEAPONTYPE_GRENADE;
}

bool c_base_combat_weapon::IsGun()
{
	switch (get_weapon_data()->WeaponType)
	{
	case WEAPONTYPE_C4:
	case WEAPONTYPE_GRENADE:
	case WEAPONTYPE_KNIFE:
	case WEAPONTYPE_UNKNOWN:
		return false;
	default:
		return true;
	}
}

bool c_base_combat_weapon::IsKnife()
{
	return m_iItemDefinitionIndex() != WEAPON_TASER && get_weapon_data()->WeaponType == WEAPONTYPE_KNIFE;
}

bool c_base_combat_weapon::IsRifle()
{
	switch (get_weapon_data()->WeaponType)
	{
	case WEAPONTYPE_RIFLE:
	case WEAPONTYPE_SUBMACHINEGUN:
	case WEAPONTYPE_SHOTGUN:
	case WEAPONTYPE_MACHINEGUN:
		return true;
	default:
		return false;
	}
}

bool c_base_combat_weapon::IsPistol()
{
	return get_weapon_data()->WeaponType == WEAPONTYPE_PISTOL;
}

bool c_base_combat_weapon::IsSniper()
{
	auto index = m_iItemDefinitionIndex();

	return index == WEAPON_AWP || index == WEAPON_SSG08 || index == WEAPON_SCAR20 || index == WEAPON_G3SG1;
	
	return get_weapon_data()->WeaponType == WEAPONTYPE_SNIPER_RIFLE;
}

bool c_base_combat_weapon::IsReloading()
{
	static auto inReload = *reinterpret_cast<uint32_t*>(utils::pattern_scan(INRELOAD) + 2);
	return *reinterpret_cast<bool*>(uintptr_t(this) + inReload);
}

CUserCmd*& c_base_player::m_pCurrentCommand()
{
	static auto current_command = *reinterpret_cast<uint32_t*>(utils::pattern_scan(CURRENT_COMMAND) + 2);
	return *reinterpret_cast<CUserCmd**>(uintptr_t(this) + current_command);
}

Vector c_base_player::GetEyePos()
{
	//return CallVFunction<Vector(__thiscall*)(void*)>(this, 279)(this);
	return m_vecOrigin() + m_vecViewOffset();
}

player_info_t c_base_player::GetPlayerInfo()
{
	return interfaces::engine_client->GetPlayerInfo(EntIndex());
}

bool c_base_player::IsAlive()
{
	return m_lifeState() == LIFE_ALIVE;
}

bool c_base_player::IsUnknown()
{
	return !IsPlayer() || is_dormant() || this == interfaces::local_player || m_iHealth() <= 0 || !IsAlive() || m_bGunGameImmunity() || (m_fFlags() & FL_FROZEN);
}

bool c_base_player::IsFlashed()
{
	return m_flFlashMaxAlphaOffset() > 200.0;
}

bool c_base_player::HasC4()
{
	static auto fnHasC4 = reinterpret_cast<bool(__thiscall*)(void*)>(utils::pattern_scan(HAS_C4));

	return fnHasC4(this);
}

bool c_base_player::CanSeePlayer(c_base_player* player, const Vector& pos)
{
	CGameTrace tr;
	Ray_t ray;
	CTraceFilter filter;
	filter.pSkip = this;

	ray.Init(GetEyePos(), pos);
	interfaces::engine_trace->trace_ray(ray, MASK_VISIBLE, &filter, &tr);

	return tr.hit_entity == player || tr.fraction > 0.97f;
}

int& c_base_player::m_nMoveType()
{
	return *reinterpret_cast<int*>(uintptr_t(this) + 0x25C);
}

void c_base_attributable_item::SetModelIndex(int modelIndex)
{
	return CallVFunction<void(__thiscall*)(void*, int)>(this, 75)(this, modelIndex);
}

void c_base_view_model::SendViewModelMatchingSequence(int sequence)
{
	return CallVFunction<void(__thiscall*)(void*, int)>(this, 214)(this, sequence);
}

bool c_base_combat_weapon::is_grenade()
{
	auto m_ClassID = GetClientClass()->m_ClassID;

	return m_ClassID == EClassId::CDecoyGrenade ||
		m_ClassID == EClassId::CHEGrenade ||
		m_ClassID == EClassId::CIncendiaryGrenade ||
		m_ClassID == EClassId::CMolotovGrenade ||
		m_ClassID == EClassId::CSensorGrenade ||
		m_ClassID == EClassId::CSmokeGrenade ||
		m_ClassID == EClassId::CFlashbang;
}

bool c_base_combat_weapon::check_detonate(const Vector& vecThrow, const trace_t& tr, int tick, float interval)
{
	switch (m_iItemDefinitionIndex())
	{
	case WEAPON_SMOKEGRENADE:
	case WEAPON_DECOY:
		if (vecThrow.Length2D() < 0.1f)
		{
			int det_tick_mod = (int)(0.2f / interval);
			return !(tick % det_tick_mod);
		}
		return false;
	case WEAPON_MOLOTOV:
	case WEAPON_INCGRENADE:
		if (tr.fraction != 1.0f && tr.plane.normal.z > 0.7f)
			return true;
	case WEAPON_FLASHBANG:
	case WEAPON_HEGRENADE:
		return (float)tick * interval > 1.5f && !(tick % (int)(0.2f / interval));
	default:
		//assert(false);
		return false;
	}
}