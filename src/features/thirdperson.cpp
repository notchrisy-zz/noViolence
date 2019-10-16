#include "features.h"
#include "../globals.h"

void features::thirdperson()
{
	if (!interfaces::local_player)
		return;

	interfaces::Input->m_fCameraInThirdPerson = globals::binds::thirdperson::enabled && interfaces::local_player->IsAlive();
	if (!interfaces::Input->m_fCameraInThirdPerson)
		return;

	QAngle angles;
	interfaces::engine_client->GetViewAngles(angles);

	QAngle backward(angles.pitch, angles.yaw + 180.f, angles.roll);
	backward.NormalizeClamp();

	Vector range;
	math::angle2vectors(backward, range);
	range *= 8192.f;

	const auto start = interfaces::local_player->GetEyePos();

	CTraceFilter filter;
	filter.pSkip = interfaces::local_player;

	Ray_t ray;
	ray.Init(start, start + range);

	trace_t tr;
	interfaces::engine_trace->trace_ray(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr);

	angles.roll = std::min<int>(start.DistTo(tr.endpos), 150); // 150 is better distance

	interfaces::Input->m_vecCameraOffset = angles;
}