#include "features/features.h"
#include "helpers/autowall.h"
#include "globals.h"
#include "helpers/input.h"
#include "helpers/console.h"
#include "helpers/entities.h"

bool IntersectionBoundingBox(const Vector& src, const Vector& dir, const Vector& min, const Vector& max, Vector* hit_point)
{
	/*
	Fast Ray-Box Intersection
	by Andrew Woo
	from "Graphics Gems", Academic Press, 1990
	*/

	constexpr int NUMDIM = 3;
	constexpr int RIGHT = 0;
	constexpr int LEFT = 1;
	constexpr int MIDDLE = 2;

	bool inside = true;
	char quadrant[NUMDIM];
	int i;

	// Rind candidate planes; this loop can be avoided if
	// rays cast all from the eye(assume perpsective view)
	Vector candidatePlane;
	for (i = 0; i < NUMDIM; i++)
	{
		if (src[i] < min[i])
		{
			quadrant[i] = LEFT;
			candidatePlane[i] = min[i];
			inside = false;
		}
		else if (src[i] > max[i])
		{
			quadrant[i] = RIGHT;
			candidatePlane[i] = max[i];
			inside = false;
		}
		else
		{
			quadrant[i] = MIDDLE;
		}
	}

	// Ray origin inside bounding box
	if (inside)
	{
		if (hit_point)
			*hit_point = src;
		return true;
	}

	// Calculate T distances to candidate planes
	Vector maxT;
	for (i = 0; i < NUMDIM; i++)
	{
		if (quadrant[i] != MIDDLE && dir[i] != 0.f)
			maxT[i] = (candidatePlane[i] - src[i]) / dir[i];
		else
			maxT[i] = -1.f;
	}

	// Get largest of the maxT's for final choice of intersection
	int whichPlane = 0;
	for (i = 1; i < NUMDIM; i++)
	{
		if (maxT[whichPlane] < maxT[i])
			whichPlane = i;
	}

	// Check final candidate actually inside box
	if (maxT[whichPlane] < 0.f)
		return false;

	for (i = 0; i < NUMDIM; i++)
	{
		if (whichPlane != i)
		{
			float temp = src[i] + maxT[whichPlane] * dir[i];
			if (temp < min[i] || temp > max[i])
			{
				return false;
			}
			else if (hit_point)
			{
				(*hit_point)[i] = temp;
			}
		}
		else if (hit_point)
		{
			(*hit_point)[i] = candidatePlane[i];
		}
	}

	// ray hits box
	return true;
}

c_base_entity* pLocalPlayer;
bool bSendPacket;
int choke_factor;

void features::LegitPeek(CUserCmd* pCmd)
{
	
	if (!pLocalPlayer)
		return;

	static bool m_bIsPeeking = false;

	if (m_bIsPeeking)
	{
		bSendPacket = !(g::client_state->chokedcommands < choke_factor);
		if (bSendPacket)
			m_bIsPeeking = false;
		return;
	}

	auto velocity = interfaces::local_player->m_vecVelocity();
	float speed = velocity.Length();

	if (speed <= 100.0f)
		return;

	ICollideable* pCollidable = pLocalPlayer->GetCollideable();

	if (!pCollidable)
		return;

	Vector min, max;
	min = pCollidable->OBBMins();
	max = pCollidable->OBBMaxs();

	min += interfaces::local_player->m_vecOrigin();
	max += interfaces::local_player->m_vecOrigin();

	Vector center = (min + max) * 0.5f;

	for (int i = 0; i <= interfaces::entity_list->GetHighestEntityIndex(); i++)
	{
		c_base_player* pEntity = (c_base_player*)interfaces::entity_list->GetClientEntity(i);

		if (!!pEntity)
			continue;

		if (pEntity->IsDead() || pEntity->IsDormant())
			continue;

		if (pEntity == pLocalPlayer || pLocalPlayer->m_iTeamNum() == pEntity->m_iTeamNum())
			continue;

		c_base_combat_weapon* pWeapon = pEntity->m_hActiveWeapon();

		if (!pWeapon)
			continue;

		if (pWeapon->m_iClip1() <= 0)
			continue;

		auto pWeaponInfo = pWeapon->get_weapon_data();

		if (!pWeaponInfo)
			continue;

		if (pWeaponInfo->WeaponType <= CSWeaponType::WEAPONTYPE_KNIFE || pWeaponInfo->WeaponType >= CSWeaponType::WEAPONTYPE_C4)
			continue;

		Vector eye_pos = pEntity->GetEyePos();

		Vector direction;
		//math::AngleVectors(pEntity->GetPlayerXY(), direction);
		math::angle2vectors(pEntity->m_angEyeAngles(), direction);	
		direction.NormalizeInPlace();

		Vector hit_point;

		bool hit = IntersectionBoundingBox(eye_pos, direction, min, max, &hit_point);

		if (hit && eye_pos.DistTo(hit_point) <= pWeaponInfo->flRange)
		{
			Ray_t ray;
			trace_t tr;
			CTraceFilterSkipEntity filter((c_base_entity*)pEntity);
			//CTraceFilter filter ((c_base_entity*)pEntity);
			ray.Init(eye_pos, hit_point);

			g::engine_trace->trace_ray(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &tr);

			if (tr.contents & CONTENTS_WINDOW) //skip windows XPPPP
			{ 																						// at this moment, we dont care about local player
				filter.pSkip = tr.hit_entity;
				ray.Init(tr.endpos, hit_point);
				g::engine_trace->trace_ray(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &tr);
			}

			if (tr.fraction == 1.0f || tr.hit_entity == g::local_player)
			{
				m_bIsPeeking = true;
				break;
			}
		}
	}
}