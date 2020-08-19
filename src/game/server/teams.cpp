/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */
#include "teams.h"
#include <engine/shared/config.h>
#include <engine/server/server.h>

CGameTeams::CGameTeams(CGameContext *pGameContext) :
		m_pGameContext(pGameContext)
{
	Reset();
}

void CGameTeams::Reset()
{
	m_Core.Reset();
	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		m_TeamState[i] = TEAMSTATE_EMPTY;
		m_MembersCount[i] = 0;
		m_LastChat[i] = 0;
	}
}

int64_t CGameTeams::TeamMask(int Team, int ExceptID, int Asker)
{
	int64_t Mask = 0;

	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (i == ExceptID)
			continue; // Explicitly excluded
		if (!GetPlayer(i))
			continue; // Player doesn't exist

		if (GetPlayer(i)->GetTeam() != -1)
		{ // Not spectator
			if (i != Asker)
			{ // Actions of other players
				if (!Character(i))
					continue; // Player is currently dead
			} // See everything of yourself
		}
		else if (GetPlayer(i)->m_SpectatorID != SPEC_FREEVIEW)
		{ // Spectating specific player
			if (GetPlayer(i)->m_SpectatorID != Asker)
			{ // Actions of other players
				if (!Character(GetPlayer(i)->m_SpectatorID))
					continue; // Player is currently dead
			} // See everything of player you're spectating
		}

		Mask |= 1LL << i;
	}
	return Mask;
}
