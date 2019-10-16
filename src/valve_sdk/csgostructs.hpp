#pragma once

#include "sdk.hpp"
#include "../helpers/utils.h"

#include <array>

#define MAX_PLAYERS 65

#define PNETVAR_OFFSET(type, funcname, class_name, var_name, offset) \
auto funcname() -> std::add_pointer_t<type> \
{ \
	constexpr auto hash = fnv::hash_constexpr(class_name "->" var_name); \
	const auto addr = std::uintptr_t(this) + offset + netvar_manager::get_offset_by_hash_cached<hash>(); \
	return reinterpret_cast<std::add_pointer_t<type>>(addr); \
}

#define PNETVAR(type, funcname, class_name, var_name) \
	PNETVAR_OFFSET(type, funcname, class_name, var_name, 0)

#define NETVAR_OFFSET(type, funcname, class_name, var_name, offset) \
auto funcname() -> std::add_lvalue_reference_t<type> \
{ \
	constexpr auto hash = fnv::hash_constexpr(class_name "->" var_name); \
	const auto addr = std::uintptr_t(this) + offset + netvar_manager::get_offset_by_hash_cached<hash>(); \
	return *reinterpret_cast<std::add_pointer_t<type>>(addr); \
}

#define NETVAR(type, funcname, class_name, var_name) \
	NETVAR_OFFSET(type, funcname, class_name, var_name, 0)

#define NETPROP(funcname, class_name, var_name) \
static auto funcname() ->  RecvProp* \
{ \
	constexpr auto hash = fnv::hash_constexpr(class_name "->" var_name); \
	static auto prop_ptr = netvar_manager::get().get_prop(hash); \
	return prop_ptr; \
}

class CCSPlayerAnimState
{
public:
	char pad_0000[120]; //0x0000
	float m_flEyeYaw; //0x0078
	float m_flPitch; //0x007C
	float m_flGoalFeetYaw; //0x0080
	float m_flCurrentFeetYaw; //0x0084
	char pad_0088[28]; //0x0088
	float m_flDuckAmount; //0x00A4
	char pad_00A8[8]; //0x00A8
	Vector m_vecOrigin; //0x00B0
	Vector m_vecLastOrigin; //0x00BC
	char pad_00C8[0x30]; //0x00C8
	float m_flSpeedFraction; //0x00F8
	float m_flSpeedFactor; //0x00FC
	char pad_0100[28]; //0x0100
	float m_flLandingRatio; //0x011C
	char pad_0120[528]; //0x0120
	float m_flMinBodyYawDegrees; //0x0330
	float m_flMaxBodyYawDegrees; //0x0334
};

enum CSWeaponType
{
	WEAPONTYPE_KNIFE = 0,
	WEAPONTYPE_PISTOL,
	WEAPONTYPE_SUBMACHINEGUN,
	WEAPONTYPE_RIFLE,
	WEAPONTYPE_SHOTGUN,
	WEAPONTYPE_SNIPER_RIFLE,
	WEAPONTYPE_MACHINEGUN,
	WEAPONTYPE_C4,
	WEAPONTYPE_PLACEHOLDER,
	WEAPONTYPE_GRENADE,
	WEAPONTYPE_UNKNOWN
};

class c_base_entity : public IClientEntity
{
public:
	NETVAR(int32_t, m_nModelIndex, "CBaseEntity", "m_nModelIndex");
	NETVAR(int32_t, m_iTeamNum, "CBaseEntity", "m_iTeamNum");
	NETVAR(Vector, m_vecOrigin, "CBaseEntity", "m_vecOrigin");
	NETVAR(Vector, m_vecAngles, "CBaseEntity", "m_vecAngles");
	NETVAR(bool, m_bShouldGlow, "CDynamicProp", "m_bShouldGlow");
	NETVAR(CHandle<c_base_player>, m_hOwnerEntity, "CBaseEntity", "m_hOwnerEntity");
	NETVAR(bool, m_bSpotted, "CBaseEntity", "m_bSpotted");
	NETVAR(bool, m_bSpottedByMask, "CBaseEntity", "m_bSpottedByMask");
	NETVAR(float, m_flAnimTime, "CBaseEntity", "m_flAnimTime");
	NETVAR_OFFSET(matrix3x4_t&, m_CollisionGroup, "CBaseEntity", "m_CollisionGroup", -0x30);
	NETVAR_OFFSET(int, GetIndex, "CBaseEntity", "m_bIsAutoaimTarget", +0x4);

	NETVAR(Vector, m_vecMins, "CBaseEntity", "m_vecMins");
	NETVAR(Vector, m_vecMaxs, "CBaseEntity", "m_vecMaxs");

	bool IsPlayer();
	bool IsWeapon();
	bool IsPlantedC4();
	bool IsDefuseKit();
	bool is_dormant();
	Vector& GetAbsOrigin();
	void SetAbsOrigin(const Vector& origin);
	const matrix3x4_t& m_rgflCoordinateFrame();
	void UpdateVisibilityAllEntities();
	int GetSequenceActivity(const int& sequence);

	static __forceinline c_base_entity* GetEntityByIndex(int index)
	{
		return static_cast<c_base_entity*>(interfaces::entity_list->GetClientEntity(index));
	}

	template <typename A>
	static __forceinline A* GetEntityFromHandle(CBaseHandle h)
	{
		return static_cast<A*>(interfaces::entity_list->GetClientEntityFromHandle(h));
	}

	static __forceinline c_base_entity* GetEntityFromHandle(CBaseHandle h) 
	{
		return GetEntityFromHandle<c_base_entity>(h);
	}
};

class c_planted_c4 : public c_base_entity
{
public:
	NETVAR(bool, m_bBombTicking, "CPlantedC4", "m_bBombTicking");
	NETVAR(bool, m_bBombDefused, "CPlantedC4", "m_bBombDefused");
	NETVAR(float, m_flC4Blow, "CPlantedC4", "m_flC4Blow");
	NETVAR(float, m_flTimerLength, "CPlantedC4", "m_flTimerLength");
	NETVAR(float, m_flDefuseLength, "CPlantedC4", "m_flDefuseLength");
	NETVAR(float, m_flDefuseCountDown, "CPlantedC4", "m_flDefuseCountDown");
	NETVAR(CHandle<c_base_player>, m_hBombDefuser, "CPlantedC4", "m_hBombDefuser");
};

class c_base_attributable_item : public c_base_entity
{
private:
	using str_32 = char[32];
public:
	NETVAR(uint64_t, m_OriginalOwnerXuid, "CBaseAttributableItem", "m_OriginalOwnerXuidLow");
	NETVAR(int32_t, m_OriginalOwnerXuidLow, "CBaseAttributableItem", "m_OriginalOwnerXuidLow");
	NETVAR(int32_t, m_OriginalOwnerXuidHigh, "CBaseAttributableItem", "m_OriginalOwnerXuidHigh");
	NETVAR(int32_t, m_nFallbackStatTrak, "CBaseAttributableItem", "m_nFallbackStatTrak");
	NETVAR(int32_t, m_nFallbackPaintKit, "CBaseAttributableItem", "m_nFallbackPaintKit");
	NETVAR(int32_t, m_nFallbackSeed, "CBaseAttributableItem", "m_nFallbackSeed");
	NETVAR(float_t, m_flFallbackWear, "CBaseAttributableItem", "m_flFallbackWear");

	NETVAR(int32_t, m_bInitialized, "CBaseAttributableItem", "m_bInitialized");
	NETVAR(short, m_iItemDefinitionIndex, "CBaseAttributableItem", "m_iItemDefinitionIndex");
	NETVAR(int32_t, m_iEntityLevel, "CBaseAttributableItem", "m_iEntityLevel");
	NETVAR(int32_t, m_iAccountID, "CBaseAttributableItem", "m_iAccountID");
	NETVAR(int32_t, m_iItemIDLow, "CBaseAttributableItem", "m_iItemIDLow");
	NETVAR(int32_t, m_iItemIDHigh, "CBaseAttributableItem", "m_iItemIDHigh");
	NETVAR(int32_t, m_iEntityQuality, "CBaseAttributableItem", "m_iEntityQuality");
	NETVAR(str_32, m_iCustomName, "CBaseAttributableItem", "m_szCustomName");

	void SetModelIndex(int modelIndex);
};

class c_base_weapon_world_model : public c_base_entity
{
public:
	NETVAR(int32_t, m_nModelIndex, "CBaseWeaponWorldModel", "m_nModelIndex");
};

class c_base_combat_weapon : public c_base_attributable_item
{
public:
	NETVAR(float_t, m_flNextPrimaryAttack, "CBaseCombatWeapon", "m_flNextPrimaryAttack");
	NETVAR(float_t, m_flNextSecondaryAttack, "CBaseCombatWeapon", "m_flNextSecondaryAttack");
	NETVAR(int32_t, m_iClip1, "CBaseCombatWeapon", "m_iClip1");
	NETVAR(int32_t, m_iClip2, "CBaseCombatWeapon", "m_iClip2");
	NETVAR(float_t, m_flRecoilIndex, "CWeaponCSBase", "m_flRecoilIndex");
	NETVAR(int32_t, m_iViewModelIndex, "CBaseCombatWeapon", "m_iViewModelIndex");
	NETVAR(int32_t, m_iWorldModelIndex, "CBaseCombatWeapon", "m_iWorldModelIndex");
	NETVAR(int32_t, m_iWorldDroppedModelIndex, "CBaseCombatWeapon", "m_iWorldDroppedModelIndex");
	NETVAR(bool, m_bPinPulled, "CBaseCSGrenade", "m_bPinPulled");
	NETVAR(float_t, m_fThrowTime, "CBaseCSGrenade", "m_fThrowTime");
	NETVAR(float_t, m_flPostponeFireReadyTime, "CBaseCombatWeapon", "m_flPostponeFireReadyTime");
	NETVAR(CHandle<c_base_weapon_world_model>, m_hWeaponWorldModel, "CBaseCombatWeapon", "m_hWeaponWorldModel");

	CCSWeaponInfo* get_weapon_data();
	bool HasBullets();
	bool CanFire();
	bool IsGrenade();
	bool IsKnife();
	bool IsReloading();
	bool IsRifle();
	bool IsPistol();
	bool IsSniper();
	bool IsGun();

	float GetInaccuracy();
	float GetSpread();
	void UpdateAccuracyPenalty();
	bool is_grenade();
	bool check_detonate(const Vector& vecThrow, const trace_t& tr, int tick, float interval);
};

class C_BasePlayerAnimState;
class C_CSPlayerAnimState;

class c_base_player : public c_base_entity
{
public:
	NETVAR(bool, m_bHasDefuser, "CCSPlayer", "m_bHasDefuser");
	NETVAR(bool, m_bGunGameImmunity, "CCSPlayer", "m_bGunGameImmunity");
	NETVAR(int32_t, m_iShotsFired, "CCSPlayer", "m_iShotsFired");
	NETVAR(QAngle, m_angEyeAngles, "CCSPlayer", "m_angEyeAngles[0]");
	NETVAR(int, m_ArmorValue, "CCSPlayer", "m_ArmorValue");
	NETVAR(bool, m_bHasHeavyArmor, "CCSPlayer", "m_bHasHeavyArmor");
	NETVAR(bool, m_bHasHelmet, "CCSPlayer", "m_bHasHelmet");
	NETVAR(bool, m_bIsScoped, "CCSPlayer", "m_bIsScoped");;
	NETVAR(float, m_flLowerBodyYawTarget, "CCSPlayer", "m_flLowerBodyYawTarget");
	NETVAR(int32_t, m_iHealth, "CBasePlayer", "m_iHealth");
	NETVAR(int32_t, m_lifeState, "CBasePlayer", "m_lifeState");
	NETVAR(int32_t, m_fFlags, "CBasePlayer", "m_fFlags");
	NETVAR(int32_t, m_nTickBase, "CBasePlayer", "m_nTickBase");
	NETVAR(Vector, m_vecViewOffset, "CBasePlayer", "m_vecViewOffset[0]");
	NETVAR(QAngle, m_viewPunchAngle, "CBasePlayer", "m_viewPunchAngle");
	NETVAR(QAngle, m_aimPunchAngle, "CBasePlayer", "m_aimPunchAngle");
	NETVAR(CHandle<c_base_view_model>, m_hViewModel, "CBasePlayer", "m_hViewModel[0]");
	NETVAR(Vector, m_vecVelocity, "CBasePlayer", "m_vecVelocity[0]");
	NETVAR(float, m_flMaxspeed, "CBasePlayer", "m_flMaxspeed");
	NETVAR(int32_t, m_iObserverMode, "CBasePlayer", "m_iObserverMode");
	NETVAR(CHandle<c_base_player>, m_hObserverTarget, "CBasePlayer", "m_hObserverTarget");
	NETVAR(float, m_flFlashMaxAlpha, "CCSPlayer", "m_flFlashMaxAlpha");
	NETVAR_OFFSET(float, m_flFlashMaxAlphaOffset, "CCSPlayer", "m_flFlashMaxAlpha", -0x8);
	NETVAR(int32_t, m_nHitboxSet, "CBaseAnimating", "m_nHitboxSet");
	NETVAR(int32_t, m_nForceBone, "CBaseAnimating", "m_nForceBone");
	NETVAR(bool, m_bClientSideAnimation, "CBaseAnimating", "m_bClientSideAnimation");
	NETVAR(CHandle<c_base_combat_weapon>, m_hActiveWeapon, "CBaseCombatCharacter", "m_hActiveWeapon");
	NETVAR(int32_t, m_iAccount, "CCSPlayer", "m_iAccount");
	NETVAR(float, m_flFlashDuration, "CCSPlayer", "m_flFlashDuration");
	NETVAR(float, m_flSimulationTime, "CBaseEntity", "m_flSimulationTime");
	NETVAR(float, m_flCycle, "CServerAnimationData", "m_flCycle");
	NETVAR(int, m_nSequence, "CBaseViewModel", "m_nSequence");
	PNETVAR(char, m_szLastPlaceName, "CBasePlayer", "m_szLastPlaceName");
	NETPROP(m_aimPunchAngleProp, "CBasePlayer", "m_aimPunchAngle");
	NETPROP(m_viewPunchAngleProp, "CBasePlayer", "m_viewPunchAngle");
	NETPROP(m_flLowerBodyYawTargetProp, "CCSPlayer", "m_flLowerBodyYawTarget");
	NETVAR(QAngle, m_angAbsAngles, "CBaseEntity", "m_angAbsAngles");
	NETVAR(Vector, m_angAbsOrigin, "CBaseEntity", "m_angAbsOrigin");
	NETVAR(float, m_flDuckSpeed, "CBaseEntity", "m_flDuckSpeed");
	NETVAR(float, m_flDuckAmount, "CBaseEntity", "m_flDuckAmount");

	std::array<float, 24> m_flPoseParameter() const
	{
		static int _m_flPoseParameter = netvar_manager::get().get_offset(fnv::hash_runtime("CBaseAnimating->m_flPoseParameter"));

		return *(std::array<float, 24>*)((uintptr_t)this + _m_flPoseParameter);
	}

	//NETVAR(float, m_flPoseParameter, "CBaseAnimating", "m_flPoseParameter");
	PNETVAR(CHandle<c_base_combat_weapon>, m_hMyWeapons, "CBaseCombatCharacter", "m_hMyWeapons");
	PNETVAR(CBaseHandle, m_hMyWearables, "CBaseCombatCharacter", "m_hMyWearables");
	NETVAR_OFFSET(QAngle*, GetVAngles, "CBasePlayer", "deadflag", +0x4);

	CUserCmd*& m_pCurrentCommand();
	Vector        GetEyePos();
	player_info_t GetPlayerInfo();
	bool          IsAlive();
	bool          IsUnknown();
	bool		  IsFlashed();
	bool          HasC4();
	bool          CanSeePlayer(c_base_player* player, const Vector& pos);
	int& m_nMoveType();
	QAngle& GetAbsAngles();
	void SetAbsAngles(const QAngle& wantedang);
	void UpdateClientSideAnimation();
	void InvalidateBoneCache();
	void PVSFix();

	CCSPlayerAnimState* GetPlayerAnimState();

	static __forceinline c_base_player* GetPlayerByUserId(int id)
	{
		return static_cast<c_base_player*>(GetEntityByIndex(interfaces::engine_client->GetPlayerForUserID(id)));
	}

	static __forceinline c_base_player* GetPlayerByIndex(int i)
	{
		return static_cast<c_base_player*>(GetEntityByIndex(i));
	}

};

class c_base_view_model : public c_base_entity
{
public:
	NETVAR(int32_t, m_nModelIndex, "CBaseViewModel", "m_nModelIndex");
	NETVAR(int32_t, m_nViewModelIndex, "CBaseViewModel", "m_nViewModelIndex");
	NETVAR(CHandle<c_base_combat_weapon>, m_hWeapon, "CBaseViewModel", "m_hWeapon");
	NETVAR(CHandle<c_base_player>, m_hOwner, "CBaseViewModel", "m_hOwner");
	NETPROP(m_nSequence, "CBaseViewModel", "m_nSequence");

	void SendViewModelMatchingSequence(int sequence);
};

class c_player_resource
{
public:
	NETPROP(GetTeamProp, "CPlayerResource", "m_iTeam");
	NETVAR(int[MAX_PLAYERS], GetPing, "CPlayerResource", "m_iPing");
};

class c_cs_player_resource : public c_player_resource
{
public:
	NETVAR(int[MAX_PLAYERS], GetRank, "CCSPlayerResource", "m_iCompetitiveRanking");
	NETVAR(int[MAX_PLAYERS], GetWins, "CCSPlayerResource", "m_iCompetitiveWins");
	NETVAR(int[MAX_PLAYERS], GetTeamColor, "CCSPlayerResource", "m_iCompTeammateColor");
	NETVAR(char[MAX_PLAYERS][16], GetClanTag, "CCSPlayerResource", "m_szClan");
	NETVAR(unsigned[MAX_PLAYERS], GetCoin, "CCSPlayerResource", "m_nActiveCoinRank");
	NETVAR(unsigned[MAX_PLAYERS], GetMusicKit, "CCSPlayerResource", "m_nMusicID");
};

class c_cs_game_rules_proxy
{
public:
	NETVAR(bool, m_bBombPlanted, "CCSGameRulesProxy", "m_bBombPlanted");
	NETVAR(uint32_t, m_iRoundTime, "CCSGameRulesProxy", "m_iRoundTime");
};