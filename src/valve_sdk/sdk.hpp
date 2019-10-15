#pragma once

#include "patterns.h"
#include "../security/importer.h"

#define NOMINMAX
#include <Windows.h>

#include "misc/vfunc.hpp"

#include "misc/Enums.hpp"

#include "math/VMatrix.hpp"
#include "math/QAngle.hpp"
#include "math/Vectors.hpp"
#include "misc/Studio.hpp"
#include "..//glow_helper.h"

#include "interfaces/IRefCounted.h"
#include "interfaces/IAppSystem.h"
#include "interfaces/IClientEntity.hpp"
#include "interfaces/IClientEntityList.hpp"
#include "interfaces/IClientMode.hpp"
#include "interfaces/IConVar.hpp"
#include "misc/ClientClass.hpp"
#include "interfaces/IEngineTrace.hpp"
#include "interfaces/IVEngineClient.hpp"
#include "interfaces/CInput.hpp"
#include "interfaces/IGameEvent.hpp"
#include "interfaces/IMaterialSystem.hpp"
#include "interfaces/IMoveHelper.hpp"
#include "interfaces/IMDLCache.hpp"
#include "interfaces/IPrediction.hpp"
#include "interfaces/IPanel.hpp"
#include "interfaces/IViewRender.hpp"
#include "interfaces/IPhysics.hpp"
#include "interfaces/IMaterial.h"
#include "interfaces/IVModelRender.h"
#include "interfaces/IBaseClientDLL.h"
#include "interfaces/IVRenderView.h"
#include "interfaces/ISurface.h"
#include "interfaces/IVModelInfo.h"
#include "interfaces/CGlobalVarsBase.h"
#include "interfaces/CClientState.h"
#include "interfaces/ILocalize.h"
#include "interfaces/IMemAlloc.h"
#include "interfaces/IFileSystem.h"
#include "interfaces/IViewRenderBeams.h"
#include "..//IStudioRender.h"

#include "misc/Convar.hpp"
#include "interfaces/ICvar.h"
#include "misc/CUserCmd.hpp"

#include "netvars.hpp"
#include "steam.h"
#include "interfaces/CGlowObjectManager.h"

struct SndInfo_t;
class IRecipientFilter;

enum soundlevel_t
{
	SNDLVL_NONE = 0,
	SNDLVL_20dB = 20,       // rustling leaves
	SNDLVL_25dB = 25,       // whispering
	SNDLVL_30dB = 30,       // library
	SNDLVL_35dB = 35,
	SNDLVL_40dB = 40,
	SNDLVL_45dB = 45,       // refrigerator
	SNDLVL_50dB = 50,       // 3.9    // average home
	SNDLVL_55dB = 55,       // 3.0
	SNDLVL_IDLE = 60,       // 2.0 
	SNDLVL_60dB = 60,       // 2.0  // normal conversation, clothes dryer
	SNDLVL_65dB = 65,       // 1.5  // washing machine, dishwasher
	SNDLVL_STATIC = 66,     // 1.25
	SNDLVL_70dB = 70,       // 1.0  // car, vacuum cleaner, mixer, electric sewing machine
	SNDLVL_NORM = 75,
	SNDLVL_75dB = 75,       // 0.8  // busy traffic
	SNDLVL_80dB = 80,       // 0.7  // mini-bike, alarm clock, noisy restaurant, office tabulator, outboard motor, passing snowmobile
	SNDLVL_TALKING = 80,    // 0.7
	SNDLVL_85dB = 85,       // 0.6  // average factory, electric shaver
	SNDLVL_90dB = 90,       // 0.5  // screaming child, passing motorcycle, convertible ride on frw
	SNDLVL_95dB = 95,
	SNDLVL_100dB = 100,     // 0.4  // subway train, diesel truck, woodworking shop, pneumatic drill, boiler shop, jackhammer
	SNDLVL_105dB = 105,     // helicopter, power mower
	SNDLVL_110dB = 110,     // snowmobile drvrs seat, inboard motorboat, sandblasting
	SNDLVL_120dB = 120,     // auto horn, propeller aircraft
	SNDLVL_130dB = 130,     // air raid siren
	SNDLVL_GUNFIRE = 140,   // 0.27 // THRESHOLD OF PAIN, gunshot, jet engine
	SNDLVL_140dB = 140,     // 0.2
	SNDLVL_150dB = 150,     // 0.2
	SNDLVL_180dB = 180,     // rocket launching

							// NOTE: Valid soundlevel_t values are 0-255.
							//       256-511 are reserved for sounds using goldsrc compatibility attenuation.
};

//-----------------------------------------------------------------------------
// common pitch values
//-----------------------------------------------------------------------------
#define PITCH_NORM      100           // non-pitch shifted
#define PITCH_LOW       95              // other values are possible - 0-255, where 255 is very high
#define PITCH_HIGH      120

class IEngineSound
{
public:
	// Precache a particular sample
	virtual bool PrecacheSound(const char* pSample, bool bPreload = false, bool bIsUISound = false) = 0;
	virtual bool IsSoundPrecached(const char* pSample) = 0;
	virtual void PrefetchSound(const char* pSample) = 0;
	virtual bool IsLoopingSound(const char* pSample) = 0;

	// Just loads the file header and checks for duration (not hooked up for .mp3's yet)
	// Is accessible to server and client though
	virtual float GetSoundDuration(const char* pSample) = 0;

	// Pitch of 100 is no pitch shift.  Pitch > 100 up to 255 is a higher pitch, pitch < 100
	// down to 1 is a lower pitch.   150 to 70 is the realistic range.
	// EmitSound with pitch != 100 should be used sparingly, as it's not quite as
	// fast (the pitchshift mixer is not native coded).

	// NOTE: setting iEntIndex to -1 will cause the sound to be emitted from the local
	// player (client-side only)
	virtual int EmitSound(IRecipientFilter& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char* pSample,
		float flVolume, float flAttenuation, int nSeed, int iFlags = 0, int iPitch = PITCH_NORM,
		const Vector * pOrigin = NULL, const Vector * pDirection = NULL, CUtlVector< Vector > * pUtlVecOrigins = NULL, bool bUpdatePositions = true, float soundtime = 0.0f, int speakerentity = -1) = 0;

	virtual int EmitSound(IRecipientFilter& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char* pSample,
		float flVolume, soundlevel_t iSoundlevel, int nSeed, int iFlags = 0, int iPitch = PITCH_NORM,
		const Vector * pOrigin = NULL, const Vector * pDirection = NULL, CUtlVector< Vector > * pUtlVecOrigins = NULL, bool bUpdatePositions = true, float soundtime = 0.0f, int speakerentity = -1) = 0;

	virtual void EmitSentenceByIndex(IRecipientFilter& filter, int iEntIndex, int iChannel, int iSentenceIndex,
		float flVolume, soundlevel_t iSoundlevel, int nSeed, int iFlags = 0, int iPitch = PITCH_NORM,
		const Vector * pOrigin = NULL, const Vector * pDirection = NULL, CUtlVector< Vector > * pUtlVecOrigins = NULL, bool bUpdatePositions = true, float soundtime = 0.0f, int speakerentity = -1) = 0;

	virtual void    StopSound(int iEntIndex, int iChannel, const char* pSample, unsigned int nSoundEntryHash) = 0;
	virtual void    StopAllSounds(bool bClearBuffers) = 0;
	virtual void    SetRoomType(IRecipientFilter& filter, int roomType) = 0;
	virtual void    SetPlayerDSP(IRecipientFilter& filter, int dspType, bool fastReset) = 0;
	virtual int     EmitAmbientSound(const char* pSample, float flVolume, int iPitch = PITCH_NORM, int flags = 0, float soundtime = 0.0f) = 0;
	virtual float   GetDistGainFromSoundLevel(soundlevel_t soundlevel, float dist) = 0;
	virtual int     GetGuidForLastSoundEmitted() = 0;
	virtual bool    IsSoundStillPlaying(int guid) = 0;
	virtual void    StopSoundByGuid(int guid, bool bForceSync) = 0;
	virtual void    SetVolumeByGuid(int guid, float fvol) = 0;
	virtual void    unk() = 0;
	virtual void    GetActiveSounds(CUtlVector<SndInfo_t>& sndlist) = 0;
	virtual void    PrecacheSentenceGroup(const char* pGroupName) = 0;
	virtual void    NotifyBeginMoviePlayback() = 0;
	virtual void    NotifyEndMoviePlayback() = 0;
	virtual bool    GetSoundChannelVolume(const char* sound, float& flVolumeLeft, float& flVolumeRight) = 0;
	virtual float   GetElapsedTimeByGuid(int guid) = 0;
};

struct IDirect3DDevice9;

class CLocalPlayer
{
	friend bool operator==(const CLocalPlayer& lhs, void* rhs);
public:
	CLocalPlayer() : m_local(nullptr) {}

	operator bool() const { return *m_local != nullptr; }
	operator c_base_player* () const { return *m_local; }

	c_base_player* operator->() { return *m_local; }

private:
	c_base_player** m_local;
};

class c_cs_player_resource;
class CRender;
class IVEngineVGui;
class IVDebugOverlay;
class IRecipientFilter;
class IEngineSound;
class IInputSystem
{
public:
	void EnableInput(bool enable)
	{
		return CallVFunction<void(__thiscall*)(void*, bool)>(this, 11)(this, enable);
	}

	void*& get_window()
	{
		static uint32_t offset = 0;
		if (!offset)
			offset = *reinterpret_cast<uint32_t*>((*reinterpret_cast<char***>(this))[10] + 5);

		return *reinterpret_cast<void**>(reinterpret_cast<char*>(this) + offset);
	}
};

class INetMessage
{
public:
	virtual	~INetMessage() {};
	virtual void SetNetChannel(INetChannel* netchan) = 0; // netchannel this message is from/for
	virtual void SetReliable(bool state) = 0;	// set to true if it's a reliable message
	virtual bool Process(void) = 0; // calles the recently set handler to process this message
	virtual	bool ReadFromBuffer(bf_read& buffer) = 0; // returns true if parsing was OK
	virtual	bool WriteToBuffer(bf_write& buffer) = 0;	// returns true if writing was OK
	virtual bool IsReliable(void) const = 0;  // true, if message needs reliable handling
	virtual int GetType(void) const = 0; // returns module specific header tag eg svc_serverinfo
	virtual int GetGroup(void) const = 0;	// returns net message group of this message
};

class CHud
{
public:
	template <typename T>
	T* FindHudElement(const char* name)
	{
		static auto ptr = utils::pattern_scan(FIND_HUD);
		static auto fn = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(ptr);

		return (T*)fn(this, name);
	}
};

class CCSGO_HudChat : public CHud
{
private:
	char pad_0x0000[0x4C]; //0x0000
public:
	int m_timesOpened; //0x004C is 0 when m_isOpen = false
private:
	char pad_0x0050[0x8]; //0x0050
public:
	bool m_isOpen; //0x0058 
private:
	char pad_0x0059[0x427]; //0x0059
};

class C_TEFireBullets
{
public:
	char pad_0000[8]; //0x0000
	int32_t m_iPlayer1; //0x000C
	int32_t m_iPlayer; //0x000C
	int32_t m_iItemDefinitionIndex; //0x0010
	Vector m_vecOrigin; //0x0014
	QAngle m_absAngles; //0x0020
};


class CModelRenderSystem;
class c_cs_game_rules_proxy;

namespace interfaces
{
	extern CInput* Input;
	extern IDirect3DDevice9* d3_device;
	extern IMDLCache* mdl_cache;
	extern IPrediction* prediction;
	extern IClientMode* client_mode;
	extern CGlobalVarsBase* global_vars;
	extern IMoveHelper* move_helper;
	extern IViewRender* view_render;
	extern IBaseClientDLL* base_client;
	extern IClientEntityList* entity_list;
	extern CClientState* client_state;
	extern CGameMovement* game_movement;
	extern IVEngineClient* engine_client;
	extern IVModelInfo* mdl_info;
	extern IVModelRender* mdl_render;
	extern IVRenderView* render_view;
	extern IEngineTrace* engine_trace;
	extern IVDebugOverlay* debug_overlay;
	extern IGameEventManager2* game_events;
	extern IMaterialSystem* mat_system;
	extern ICvar* cvar;
	extern IPanel* vgui_panel;
	extern ISurface* surface;
	extern IPhysicsSurfaceProps* physics_surface;
	extern CLocalPlayer local_player;
	extern IEngineSound* engine_sound;
	extern IInputSystem* input_system;
	extern IVEngineVGui* engine_vgui;
	extern CRender* render;
	extern c_cs_player_resource** player_resource;
	extern CHud* Hud;
	extern CGlowObjectManager* glow_obj_manager;
	extern ILocalize* localize;
	extern IMemAlloc* mem_alloc;
	extern IWeaponSystem* weapon_system;
	extern IFileSystem* file_system;
	extern CModelRenderSystem* model_render_system;
	extern IViewRenderBeams* view_render_beams;
	extern CCSGO_HudChat* hud_chat;
	extern C_TEFireBullets* fire_bullets;
	extern c_cs_game_rules_proxy* game_rules_proxy;
	extern glow_manager_t* glow_manager;
	extern uintptr_t* g_SpatialPartition;
	extern IStudioRender* g_studiorender;

	extern ISteamUser* steam_user;
	extern ISteamHTTP* steam_http;
	extern ISteamClient* steam_client;
	extern ISteamFriends* steam_friends;
	extern ISteamGameCoordinator* game_coordinator;

	void initialize();
}

#pragma pack(push, 1)
struct mat_vide_mode_t
{
	int m_Width;
	int m_Height;
	int m_Format;
	int m_RefreshRate;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct matsystem_config_t
{
	mat_vide_mode_t m_VideoMode;
	float m_fMonitorGamma;
	float m_fGammaTVRangeMin;
	float m_fGammaTVRangeMax;
	float m_fGammaTVExponent;
	bool m_bGammaTVEnabled;
	bool m_bTripleBuffered;
	int m_nAASamples;
	int m_nForceAnisotropicLevel;
	int m_nSkipMipLevels;
	int m_nDxSupportLevel;
	int m_nFlags;
	bool m_bEditMode;
	char m_nProxiesTestMode;
	bool m_bCompressedTextures;
	bool m_bFilterLightmaps;
	bool m_bFilterTextures;
	bool m_bReverseDepth;
	bool m_bBufferPrimitives;
	bool m_bDrawFlat;
	bool m_bMeasureFillRate;
	bool m_bVisualizeFillRate;
	bool m_bNoTransparency;
	bool m_bSoftwareLighting;
	bool m_bAllowCheats;
	char m_nShowMipLevels;
	bool m_bShowLowResImage;
	bool m_bShowNormalMap;
	bool m_bMipMapTextures;
	char m_nFullbright;
	bool m_bFastNoBump;
	bool m_bSuppressRendering;
	bool m_bDrawGray;
	bool m_bShowSpecular;
	bool m_bShowDiffuse;
	int m_nWindowedSizeLimitWidth;
	int m_nWindowedSizeLimitHeight;
	int m_nAAQuality;
	bool m_bShadowDepthTexture;
	bool m_bMotionBlur;
	bool m_bSupportFlashlight;
	bool m_bPaintEnabled;
	char pad[0xC];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SndInfo_t
{
	int m_nGuid;
	void* m_filenameHandle;
	int m_nSoundSource;
	int m_nChannel;
	int m_nSpeakerEntity;
	float m_flVolume;
	float m_flLastSpatializedVolume;
	float m_flRadius;
	int m_nPitch;
	Vector* m_pOrigin;
	Vector* m_pDirection;
	bool m_bUpdatePositions;
	bool m_bIsSentence;
	bool m_bDryMix;
	bool m_bSpeaker;
	bool m_bSpecialDSP;
	bool m_bFromServer;
};
#pragma pack(pop)

#include "misc/EHandle.hpp"

namespace g  = interfaces; //You dont need now to type interfaces:: , "g::" is shorter,cause why not //MJ409 

