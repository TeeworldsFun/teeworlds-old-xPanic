/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "projectile.h"

#define M_PI 3.14159265358979323846

CProjectile::CProjectile(CGameWorld *pGameWorld, int Type, int Owner, vec2 Pos, vec2 Dir, int Span,
		int Damage, bool Explosive, float Force, int SoundImpact, int Weapon)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_PROJECTILE)
{
	m_Type = Type;
	m_Pos = Pos;
	m_Direction = Dir;
	m_LifeSpan = Span;
	m_Owner = Owner; 
	m_Force = Force;
	m_Damage = Damage;
	m_SoundImpact = SoundImpact;
	m_Weapon = Weapon;
	m_StartTick = Server()->Tick();
	m_Explosive = Explosive;
	
	if(m_Type == WEAPON_GRENADE)
		GameServer()->m_pController->m_GrenadeLimit++;
	
	GameWorld()->InsertEntity(this); 
}

void CProjectile::Reset()
{
	if(m_Type == WEAPON_GRENADE && GameServer()->m_pController->m_GrenadeLimit)
		GameServer()->m_pController->m_GrenadeLimit--;
	
	GameServer()->m_World.DestroyEntity(this);
}

vec2 CProjectile::GetPos(float Time)
{
	float Curvature = 0;
	float Speed = 0;

	switch(m_Type)
	{
		case WEAPON_GRENADE:
		if(m_Explosive)
		{
			Curvature = GameServer()->Tuning()->m_GrenadeCurvature;
			Speed = GameServer()->Tuning()->m_GrenadeSpeed;
		}
		else
		{
			Curvature = 0;
			Speed = 500.0f;
		} 
			break;

		case WEAPON_SHOTGUN:
			Curvature = GameServer()->Tuning()->m_ShotgunCurvature;
			Speed = GameServer()->Tuning()->m_ShotgunSpeed;
			break;

		case WEAPON_GUN:
			Curvature = GameServer()->Tuning()->m_GunCurvature;
			Speed = GameServer()->Tuning()->m_GunSpeed;
			break;
	}

	return CalcPos(m_Pos, m_Direction, Curvature, Speed, Time);
}


void CProjectile::Tick()
{	
	float Ot = (Server()->Tick()-m_StartTick-3)/(float)Server()->TickSpeed();
	float Pt = (Server()->Tick()-m_StartTick-1)/(float)Server()->TickSpeed();
	float Ct = (Server()->Tick()-m_StartTick)/(float)Server()->TickSpeed();
	vec2 PrevPos = GetPos(Pt);
	vec2 CurPos = GetPos(Ct);
	vec2 OldPos = GetPos(Ot);
	int Collide = GameServer()->Collision()->IntersectLine(PrevPos, CurPos, &CurPos, 0, false);
	CCharacter *OwnerChar = GameServer()->GetPlayerChar(m_Owner);
	CCharacter *TargetChr = GameServer()->m_World.IntersectCharacter(PrevPos, CurPos, 6.0f, CurPos, OwnerChar);

	m_LifeSpan--;
	if(!OwnerChar || m_LifeSpan < 0 ||
		m_Type == WEAPON_GRENADE && (GameServer()->m_pController->m_GrenadeLimit > g_Config.m_SvGrenadeLimit && m_Type == WEAPON_GRENADE && m_LifeSpan < Server()->TickSpeed()*(GameServer()->Tuning()->m_GrenadeLifetime-g_Config.m_SvGrenadeLifeRem) 
		|| GameServer()->m_pController->m_GrenadeLimit > g_Config.m_SvGrenadeLimit+g_Config.m_SvGrenadeWarningLimit))
	{	
		Reset();
		return;
	}

	if(Collide && !m_Explosive && m_Type == WEAPON_GRENADE)
	{
		GameServer()->Collision()->MovePoint(&PrevPos, &m_Direction, 1);
		m_Pos = PrevPos;
		m_StartTick = Server()->Tick();
		GameServer()->CreateSound(CurPos, 1);
		return;
	}
	if((TargetChr && TargetChr->GetPlayer() && TargetChr->GetPlayer()->GetTeam() == TEAM_RED) || Collide || GameLayerClipped(CurPos) || OwnerChar->HammeredBomb && OwnerChar->ThrownBomb || (!OwnerChar->m_SlowBombTick && OwnerChar->GetPlayer()->GetTeam() == TEAM_RED))
	{
		if(m_LifeSpan >= 0 || m_Weapon == WEAPON_GRENADE)
			GameServer()->CreateSound(CurPos, m_SoundImpact);

		if(m_Explosive)
			GameServer()->CreateExplosion(CurPos, m_Owner, m_Weapon, false, -1, -1);
		else if(!m_Explosive && m_Type == WEAPON_GRENADE)
		{
			GameServer()->DetonateSlowBomb(CurPos);
			Reset();
			OwnerChar->ThrownBomb = false;
		}

		else if(TargetChr && TargetChr->GetPlayer())
			TargetChr->TakeDamage(m_Direction * max(0.001f, m_Force), m_Damage, m_Owner, m_Weapon);

		Reset();
	} 
}

void CProjectile::TickPaused()
{
	++m_StartTick;
}

void CProjectile::Snap(int SnappingClient)
{
	float Ct = (Server()->Tick()-m_StartTick)/(float)Server()->TickSpeed();
	vec2 CurPos = GetPos(Ct);

	if(NetworkClipped(SnappingClient, GetPos(Ct)))
		return;


	if(m_Type == WEAPON_GRENADE)
	{
		CNetObj_Projectile *pProj = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_ID, sizeof(CNetObj_Projectile)));
		if(!pProj)
			return;

		pProj->m_X = (int)CurPos.x;
		pProj->m_Y = (int)CurPos.y;
		pProj->m_VelX = (int)(m_Direction.x*100.0f);
		pProj->m_VelY = (int)(m_Direction.y*100.0f);
		pProj->m_StartTick = m_StartTick;
		pProj->m_Type = m_Type;
	}
	else
	{
		CNetObj_Projectile *pProj = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_ID, sizeof(CNetObj_Projectile)));
		if(!pProj)
			return;

		pProj->m_X = (int)m_Pos.x;
		pProj->m_Y = (int)m_Pos.y;
		pProj->m_VelX = (int)(m_Direction.x*100.0f);
		pProj->m_VelY = (int)(m_Direction.y*100.0f);
		pProj->m_StartTick = m_StartTick;
		pProj->m_Type = m_Type;
	}
		
}
