#include "gamecontext.h"

void CGameContext::ConSound(IConsole::IResult* pResult, void* pUserData)
{
	CGameContext* pSelf = (CGameContext*)pUserData;

	if (pResult->NumArguments() < 1)
		return;

	if (pSelf->m_apPlayers[pResult->m_ClientID]->GetTeam() == TEAM_SPECTATORS || !pSelf->GetPlayerChar(pResult->m_ClientID))
		return;

	int SoundID = pResult->GetInteger(0);

	SoundID = clamp(SoundID, 0, 40);

	pSelf->CreateSound(pSelf->GetPlayerChar(pResult->m_ClientID)->m_Pos, SoundID);
}

void CGameContext::ConFreeze(IConsole::IResult* pResult, void* pUserData)
{
	CGameContext* pSelf = (CGameContext*)pUserData;

	if (pResult->NumArguments() < 1)
		return;

	int ClientID = pResult->GetInteger(0);

	if (ClientID < 0 || ClientID >= MAX_CLIENTS || !pSelf->m_apPlayers[ClientID])
		return pSelf->SendChatTarget(pResult->m_ClientID, "There is no such player!");

	if (ClientID == pResult->m_ClientID)
		return pSelf->SendChatTarget(pResult->m_ClientID, "You can not freeze your account!");

	if (!pSelf->m_apPlayers[ClientID]->m_AccData.m_UserID)
		return pSelf->SendChatTarget(pResult->m_ClientID, "The player is not logged in account!");

	pSelf->m_apPlayers[ClientID]->m_AccData.m_Freeze ^= true;
	pSelf->m_apPlayers[ClientID]->m_pAccount->Apply();

	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "You have already %s account of player %s",
		pSelf->m_apPlayers[ClientID]->m_AccData.m_Freeze ? "freezed" : "unfreezed", pSelf->Server()->ClientName(ClientID));
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

	str_format(aBuf, sizeof(aBuf), "Your account has been %s by police - %s",
		pSelf->m_apPlayers[ClientID]->m_AccData.m_Freeze ? "freezed" : "unfreezed", pSelf->Server()->ClientName(pResult->m_ClientID));
	pSelf->SendChatTarget(ClientID, aBuf);

	str_format(aBuf, sizeof(aBuf), "Police '%s' freezed account of player '%s', his login - '%s'",
		pSelf->Server()->ClientName(pResult->m_ClientID), pSelf->Server()->ClientName(ClientID), pSelf->m_apPlayers[ClientID]->m_AccData.m_Username);

	dbg_msg("police", aBuf);

	str_format(aBuf, sizeof(aBuf), "'%s' was %s by '%s'", pSelf->Server()->ClientName(ClientID),
		pSelf->m_apPlayers[ClientID]->m_AccData.m_Freeze ? "freezed" : "unfreezed", pSelf->Server()->ClientName(pResult->m_ClientID));
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "xpanic", aBuf);
}

void CGameContext::ConMoney(IConsole::IResult* pResult, void* pUserData)
{
	CGameContext* pSelf = (CGameContext*)pUserData;

	if (pResult->NumArguments() < 1)
		return;

	int ClientID = pResult->GetInteger(0);

	if (ClientID < 0 || ClientID >= MAX_CLIENTS || !pSelf->m_apPlayers[ClientID])
		return pSelf->SendChatTarget(pResult->m_ClientID, "There is no such player!");

	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "'%s' money is %d",
		pSelf->Server()->ClientName(ClientID), pSelf->m_apPlayers[ClientID]->m_AccData.m_Money);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
}

void CGameContext::ConGroup(IConsole::IResult* pResult, void* pUserData)
{
	CGameContext* pSelf = (CGameContext*)pUserData;

	if (pResult->NumArguments() < 1)
		return;

	int ClientID = pResult->GetInteger(0);

	if (ClientID < 0 || ClientID >= MAX_CLIENTS || !pSelf->m_apPlayers[ClientID])
		return pSelf->SendChatTarget(pResult->m_ClientID, "There is no such player!");

	char aBuf[128];
	char gname[4][12] = { "player", "police", "vip", "helper" };

	str_format(aBuf, sizeof(aBuf), "'%s' group is '%s'",
		pSelf->Server()->ClientName(ClientID), gname[pSelf->m_apPlayers[ClientID]->m_AccData.m_PlayerState]);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
}

void CGameContext::ConSetLevel(IConsole::IResult* pResult, void* pUserData)
{
	CGameContext* pSelf = (CGameContext*)pUserData;

	if (pResult->NumArguments() < 2)
		return;

	int ClientID = pResult->GetInteger(0);
	int Level = pResult->GetInteger(1);

	if (ClientID < 0 || ClientID >= MAX_CLIENTS || !pSelf->m_apPlayers[ClientID])
		return pSelf->SendChatTarget(pResult->m_ClientID, "There is no such player!");

	char aBuf[64];

	str_format(aBuf, sizeof(aBuf), "You had set %s's level to %d", pSelf->Server()->ClientName(ClientID), Level);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

	str_format(aBuf, sizeof(aBuf), "Your level was changed to %d!", Level);
	pSelf->SendChatTarget(ClientID, aBuf);

	pSelf->m_apPlayers[ClientID]->m_AccData.m_Level = clamp(Level, 0, 2147483647);

	pSelf->m_apPlayers[ClientID]->m_AccData.m_Exp = 0;
	pSelf->m_apPlayers[ClientID]->m_pAccount->Apply();
}

void CGameContext::ConSetTurretLevel(IConsole::IResult* pResult, void* pUserData)
{
	CGameContext* pSelf = (CGameContext*)pUserData;

	if (pResult->NumArguments() < 2)
		return;

	int ClientID = pResult->GetInteger(0);
	int Level = pResult->GetInteger(1);

	if (ClientID < 0 || ClientID >= MAX_CLIENTS || !pSelf->m_apPlayers[ClientID])
		return pSelf->SendChatTarget(pResult->m_ClientID, "There is no such player!");

	char aBuf[64];
	str_format(aBuf, sizeof(aBuf), "You had set %s's turret level to %d",
		pSelf->Server()->ClientName(ClientID), Level);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

	str_format(aBuf, sizeof(aBuf), "Your turret's level was changed to %d!", Level);
	pSelf->SendChatTarget(ClientID, aBuf);

	pSelf->m_apPlayers[ClientID]->m_AccData.m_TurretLevel = clamp(Level, 0, 2147483647);
	pSelf->m_apPlayers[ClientID]->m_AccData.m_TurretExp = 0;
	pSelf->m_apPlayers[ClientID]->m_pAccount->Apply();
}

void CGameContext::ConSetMoney(IConsole::IResult* pResult, void* pUserData)
{
	CGameContext* pSelf = (CGameContext*)pUserData;

	if (pResult->NumArguments() < 2)
		return;

	int ClientID = pResult->GetInteger(0);
	int Amount = pResult->GetInteger(1);

	if (ClientID < 0 || ClientID >= MAX_CLIENTS || !pSelf->m_apPlayers[ClientID])
		return pSelf->SendChatTarget(pResult->m_ClientID, "There is no such player!");

	char aBuf[64];
	str_format(aBuf, sizeof(aBuf), "You had set %s's money to %d",
		pSelf->Server()->ClientName(ClientID), Amount);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

	str_format(aBuf, sizeof(aBuf), "Your money was changed to %d!", Amount);
	pSelf->SendChatTarget(ClientID, aBuf);

	pSelf->m_apPlayers[ClientID]->m_AccData.m_Money = clamp(Amount, 0, 2147483647);
	pSelf->m_apPlayers[ClientID]->m_pAccount->Apply();
}

void CGameContext::ConSetTurretMoney(IConsole::IResult* pResult, void* pUserData)
{
	CGameContext* pSelf = (CGameContext*)pUserData;

	if (pResult->NumArguments() < 2)
		return;

	int ClientID = pResult->GetInteger(0);
	int Amount = pResult->GetInteger(1);

	if (ClientID < 0 || ClientID >= MAX_CLIENTS || !pSelf->m_apPlayers[ClientID])
		return pSelf->SendChatTarget(pResult->m_ClientID, "There is no such player!");

	char aBuf[64];
	str_format(aBuf, sizeof(aBuf), "You had set %s's turret money to %d",
		pSelf->Server()->ClientName(ClientID), Amount);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

	str_format(aBuf, sizeof(aBuf), "Your turret's money was changed to %d!", Amount);
	pSelf->SendChatTarget(ClientID, aBuf);

	pSelf->m_apPlayers[ClientID]->m_AccData.m_TurretMoney = clamp(Amount, 0, 2147483647);
	pSelf->m_apPlayers[ClientID]->m_pAccount->Apply();
}

void CGameContext::ConSetGroup(IConsole::IResult* pResult, void* pUserData)
{
	CGameContext* pSelf = (CGameContext*)pUserData;

	if (pResult->NumArguments() < 2)
		return;

	int ClientID = pResult->GetInteger(0);
	int Group = pResult->GetInteger(1);

	if (ClientID < 0 || ClientID >= MAX_CLIENTS || !pSelf->m_apPlayers[ClientID])
		return pSelf->SendChatTarget(pResult->m_ClientID, "There is no such player!");

	if (!pSelf->m_apPlayers[ClientID]->m_AccData.m_UserID)
		return pSelf->SendChatTarget(pResult->m_ClientID, "The player is not logged in account!");

	char gname[4][12] = { "", "police", "vip", "helper" }, aBuf[64];

	if (Group == 0)
	{
		if (pSelf->m_apPlayers[ClientID]->m_AccData.m_PlayerState)
		{
			str_format(aBuf, sizeof(aBuf), "'%s' kicked out from the group %s",
				gname[pSelf->m_apPlayers[ClientID]->m_AccData.m_PlayerState], pSelf->Server()->ClientName(ClientID));
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

			str_format(aBuf, sizeof(aBuf), "You had kicked out from %s!",
				gname[pSelf->m_apPlayers[ClientID]->m_AccData.m_PlayerState]);
			pSelf->SendChatTarget(ClientID, aBuf);

			pSelf->m_apPlayers[ClientID]->m_AccData.m_PlayerState = 0;
		}
		else pSelf->SendChatTarget(pResult->m_ClientID, "This player is not a member of the group already!");
	}

	else
	{
		if (Group > 3 || Group < 0)
			return pSelf->SendChatTarget(pResult->m_ClientID, "Group ID not found!");

		pSelf->m_apPlayers[ClientID]->m_AccData.m_PlayerState = Group;

		str_format(aBuf, sizeof(aBuf), "Setted group %s for player '%s'",
			gname[pSelf->m_apPlayers[ClientID]->m_AccData.m_PlayerState], pSelf->Server()->ClientName(ClientID));
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

		str_format(aBuf, sizeof(aBuf), "Your have joined %s!",
			gname[pSelf->m_apPlayers[ClientID]->m_AccData.m_PlayerState]);
		pSelf->SendChatTarget(ClientID, aBuf);
	}

	str_format(aBuf, sizeof(aBuf), "'%s' group changed to '%s'",
		pSelf->Server()->ClientName(ClientID), gname[pSelf->m_apPlayers[ClientID]->m_AccData.m_PlayerState]);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "xpanic", aBuf);
}

void CGameContext::ConSetScore(IConsole::IResult* pResult, void* pUserData)
{
	CGameContext* pSelf = (CGameContext*)pUserData;

	if (pResult->NumArguments() < 2)
		return;

	int ClientID = pResult->GetInteger(0);
	int Amount = pResult->GetInteger(1);

	if (ClientID < 0 || ClientID >= MAX_CLIENTS || !pSelf->m_apPlayers[ClientID])
		return pSelf->SendChatTarget(pResult->m_ClientID, "There is no such player!");

	char aBuf[64];
	str_format(aBuf, sizeof(aBuf), "You had set %s's score to %d",
		pSelf->Server()->ClientName(ClientID), Amount);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

	str_format(aBuf, sizeof(aBuf), "Your score's was changed to %d!", Amount);
	pSelf->SendChatTarget(ClientID, aBuf);
	pSelf->m_apPlayers[ClientID]->m_Score = Amount;
}

void CGameContext::ConResetAccount(IConsole::IResult* pResult, void* pUserData)
{
	CGameContext* pSelf = (CGameContext*)pUserData;

	if (pResult->NumArguments() < 1)
		return;

	int ClientID = pResult->GetInteger(0);

	if (ClientID < 0 || ClientID >= MAX_CLIENTS || !pSelf->m_apPlayers[ClientID])
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "There is no such player!");
		return;
	}

	char aBuf[64];
	str_format(aBuf, sizeof(aBuf), "You have reseted %s's account.",
		pSelf->Server()->ClientName(ClientID));
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

	str_format(aBuf, sizeof(aBuf), "Your account was reseted!");
	pSelf->SendChatTarget(ClientID, aBuf);

	pSelf->m_apPlayers[ClientID]->m_AccData.m_Money = pSelf->m_apPlayers[ClientID]->m_AccData.m_Level;

	pSelf->m_apPlayers[ClientID]->m_AccData.m_Dmg = 0;
	pSelf->m_apPlayers[ClientID]->m_AccData.m_Health = 0;
	pSelf->m_apPlayers[ClientID]->m_AccData.m_Ammoregen = 0;
	pSelf->m_apPlayers[ClientID]->m_AccData.m_Ammo = 0;
	pSelf->m_apPlayers[ClientID]->m_AccData.m_Handle = 0;

	pSelf->m_apPlayers[ClientID]->m_AccData.m_TurretMoney = pSelf->m_apPlayers[ClientID]->m_AccData.m_TurretLevel;
	pSelf->m_apPlayers[ClientID]->m_AccData.m_TurretDmg = 0;
	pSelf->m_apPlayers[ClientID]->m_AccData.m_TurretSpeed = 0;
	pSelf->m_apPlayers[ClientID]->m_AccData.m_TurretRange = 0;
	pSelf->m_apPlayers[ClientID]->m_AccData.m_TurretShotgun = 0;

	pSelf->m_apPlayers[ClientID]->m_AccData.m_Exp = 0;
	pSelf->m_apPlayers[ClientID]->m_pAccount->Apply();
}
