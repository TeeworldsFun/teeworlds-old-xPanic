#include <stdio.h>
#include <string.h>

#include <engine/shared/config.h>
#include <engine/server.h>
#include <game/version.h>
#include "cmds.h"
#include "account.h"
#include "hearth.h"
//#include "slowbomb.h"


#define MAX_INT 4000000000
#define DEBUG(A, B) GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, A, B)

CCmd::CCmd(CPlayer *pPlayer, CGameContext *pGameServer)
{
	m_pPlayer = pPlayer;
	m_pGameServer = pGameServer;
}

void CCmd::ChatCmd(CNetMsg_Cl_Say *Msg)
{
	LastChat();
	
	if(!str_comp_nocase_num(Msg->m_pMessage+1, "login", 5))
	{
		if(GameServer()->m_World.m_Paused)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please wait end round!");
			return;
		}

		char user[256], pass[256];
		if(sscanf(Msg->m_pMessage, "/login %s %s", user, pass) != 2)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Use: /login <username> <password>");
			return;
		}

		if(str_length(user) > 15 || str_length(user) < 4 || str_length(pass) > 15 || str_length(pass) < 4)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Username / Password must contain 4-15 characters");
			return;
		}	
	
		m_pPlayer->m_pAccount->Login(user, pass);
		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "register", 8))
	{
		char user[64], pass[64];
		if(sscanf(Msg->m_pMessage, "/register %s %s", user, pass) != 2)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Use /register <username> <password>'");
			return;
		}

		if(str_length(user) > 15 || str_length(user) < 4 || str_length(pass) > 15 || str_length(pass) < 4)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Username / Password must contain 4-15 characters");
			return;
		}

		else if(!str_comp_nocase(user, pass))
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Username and password must be different!");
			return;
		}

		if(GameServer()->m_World.m_Paused)
		{
			m_pPlayer->m_pAccount->Register(user, pass, false);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please wait end round and login!");
			return;
		}

		m_pPlayer->m_pAccount->Register(user, pass, true);
		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "sd", 2))
	{
		if(!GameServer()->Server()->IsAuthed(m_pPlayer->GetCID()))
			return;

		int size = 0;
		if((sscanf(Msg->m_pMessage, "/sd %d", &size)) != 1)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please use: /sd <idsound>");
			return;
		}

		if(m_pPlayer->GetTeam() == TEAM_SPECTATORS || !GameServer()->GetPlayerChar(m_pPlayer->GetCID()))
			return;

		int soundid = clamp(size, 0, 40);
		GameServer()->CreateSound(GameServer()->GetPlayerChar(m_pPlayer->GetCID())->m_Pos, soundid);
		return;
	}

	else if(!str_comp_nocase(Msg->m_pMessage+1, "logout"))
	{
		if(!m_pPlayer->m_AccData.m_UserID)
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You aren't logged in");

		if(GameServer()->m_World.m_Paused)
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please wait end round!");

		if(GameServer()->m_pController->NumZombs() == 1 && m_pPlayer->GetTeam() == TEAM_RED)
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You last zombie!");

		if(GameServer()->m_pController->NumPlayers() < 3 && GameServer()->m_pController->m_Warmup)
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Wait for the beginning of the round");

		m_pPlayer->m_pAccount->Apply(), m_pPlayer->m_pAccount->Reset();

		GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Successes! You logout.");
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You can login again with the command:");
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), "/login <username> <password>");

		if(GameServer()->GetPlayerChar(m_pPlayer->GetCID()) && GameServer()->GetPlayerChar(m_pPlayer->GetCID())->IsAlive())
			GameServer()->GetPlayerChar(m_pPlayer->GetCID())->Die(m_pPlayer->GetCID(), WEAPON_GAME);

		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "upgr", 4))
	{
		if(!m_pPlayer->m_AccData.m_UserID)
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You are not logged in! Type '/account' for more information!");

		char supgr[256], andg[64];
		int kolvo; //  второй аргумент - количество апгрейда, которое хочет игроk

		if(sscanf(Msg->m_pMessage, "/upgr %s %d", supgr, &kolvo) != 2)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "/upgr <type> <amount>");
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Types: dmg, hp, handle, ammoregen, ammo, stats");
			return;
		}

		if(!str_comp_nocase(supgr, "handle"))
		{
			if(m_pPlayer->m_AccData.m_Handle >= 300)
				if(rand() % 2 + 1 == 1)
					GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Did you really think it's not the end? :D Nope, 300 handle is the end.");
				else
					GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Sorry, but you can't get more than 300 lvl of handle :/");
			
			if(m_pPlayer->m_AccData.m_Money < kolvo)
			{
				char aChat[128];
				if(rand() % 3 + 1 == 3)
					str_format(aChat, sizeof(aChat), "Man, are you seriously? Take your unexistance money back. You have just %d money counts! Get more.", m_pPlayer->m_AccData.m_Money);
				else if (rand() % 3 + 1 == 2)
					str_format(aChat, sizeof(aChat), "Oh... do you really checked your money counts? Very bad. It's %d.", m_pPlayer->m_AccData.m_Money);
				else
					str_format(aChat, sizeof(aChat), "Sorry, but you have no enough money. Your money is %d.", m_pPlayer->m_AccData.m_Money);
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), aChat);
			}

			if(m_pPlayer->m_AccData.m_Handle + kolvo > 300)
			{
				char aChat[128];
				if(rand() % 2 + 1 == 2)
					str_format(aChat, sizeof(aChat), "Probability handle lvl is higher then 20, decrease amount to %d (your money is %d).", 300 - m_pPlayer->m_AccData.m_Handle, m_pPlayer->m_AccData.m_Money);
				else
					str_format(aChat, sizeof(aChat), "It doesn't work so, man. Minimize your request to %d (amount of money counts is %d)", 300 - m_pPlayer->m_AccData.m_Handle, m_pPlayer->m_AccData.m_Money);	
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), aChat);
			}

			m_pPlayer->m_AccData.m_Money -= kolvo; // т.к. цена хэндла - 1 монета, мы просто списываем количество захоченного игроком товара :)
			m_pPlayer->m_AccData.m_Handle += kolvo; // и, соответственно, прибавляем

			str_format(andg, sizeof(andg), "Your new handle is: %d, Your money: %d",
				m_pPlayer->m_AccData.m_Handle, m_pPlayer->m_AccData.m_Money);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), andg);

			m_pPlayer->m_pAccount->Apply();
			return;
		}

		else if(!str_comp_nocase(supgr, "dmg"))
		{
			if(m_pPlayer->m_AccData.m_Dmg >= 20)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "20 is a maximal damage level. Don't try to get more.");
	
			if(m_pPlayer->m_AccData.m_Money < kolvo)
			{
				char aChat[128];
				if(rand() % 3 + 1 == 3)
					str_format(aChat, sizeof(aChat), "Man, are you seriously? Take your unexistance money back. You have just %d money counts! Get more.", m_pPlayer->m_AccData.m_Money);
				else if (rand() % 3 + 1 == 2)
					str_format(aChat, sizeof(aChat), "Oh... do you really checked your money counts? Very bad. It's %d.", m_pPlayer->m_AccData.m_Money);
				else
					str_format(aChat, sizeof(aChat), "Sorry, but you have no enough money. Your money is %d.", m_pPlayer->m_AccData.m_Money);
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), aChat);
			}

			if(m_pPlayer->m_AccData.m_Dmg + kolvo > 20)
			{

				char aChat[128];
				if(rand() % 2 + 1 == 2)
					str_format(aChat, sizeof(aChat), "Probability damage lvl is higher then 20, decrease amount to %d (your money is %d).", 20 - m_pPlayer->m_AccData.m_Dmg, m_pPlayer->m_AccData.m_Money);
				else
					str_format(aChat, sizeof(aChat), "It doesn't work so, man. Minimize your request to %d (amount of money counts is %d)", 20 - m_pPlayer->m_AccData.m_Dmg, m_pPlayer->m_AccData.m_Money);	
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), aChat);
			}

			m_pPlayer->m_AccData.m_Money -= kolvo;
			m_pPlayer->m_AccData.m_Dmg += kolvo;

			str_format(andg, sizeof(andg), "Your new damage is: %d, Your money: %d",
				m_pPlayer->m_AccData.m_Dmg, m_pPlayer->m_AccData.m_Money);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), andg);

			m_pPlayer->m_pAccount->Apply();
			return;
		}

		else if(!str_comp_nocase(supgr, "hp"))
		{
			if(m_pPlayer->m_AccData.m_Health >= 100)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Maximal hp level!");

			if(m_pPlayer->m_AccData.m_Money < kolvo)
			{
				char aChat[128];
				if(rand() % 3 + 1 == 3)
					str_format(aChat, sizeof(aChat), "Man, are you seriously? Take your unexistance money back. You have just %d money counts! Get more.", m_pPlayer->m_AccData.m_Money);
				else if (rand() % 3 + 1 == 2)
					str_format(aChat, sizeof(aChat), "Oh... do you really checked your money counts? Very bad. It's %d.", m_pPlayer->m_AccData.m_Money);
				else
					str_format(aChat, sizeof(aChat), "Sorry, but you have no enough money. Your money is %d.", m_pPlayer->m_AccData.m_Money);
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), aChat);
			}

			if(m_pPlayer->m_AccData.m_Health + kolvo > 100)
			{
				char aChat[128];
				if(rand() % 2 + 1 == 2)
					str_format(aChat, sizeof(aChat), "Probability hp lvl is higher then 100, decrease amount to %d (your money is %d).", 100 - m_pPlayer->m_AccData.m_Health, m_pPlayer->m_AccData.m_Money);
				else
					str_format(aChat, sizeof(aChat), "It doesn't work so, man. Minimize your request to %d (amount of money counts is %d)", 100 - m_pPlayer->m_AccData.m_Health, m_pPlayer->m_AccData.m_Money);	
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), aChat);
			}

			m_pPlayer->m_AccData.m_Money -= kolvo;
			m_pPlayer->m_AccData.m_Health += kolvo;

			str_format(andg, sizeof(andg), "Your new health is: %d, Your money: %d",
				m_pPlayer->m_AccData.m_Health, m_pPlayer->m_AccData.m_Money);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), andg);

			m_pPlayer->m_pAccount->Apply();
			return;
		}

		else if(!str_comp_nocase(supgr, "ammoregen"))
		{
			if(m_pPlayer->m_AccData.m_Ammoregen >= 60)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Maximal ammoregen level!");

			if(m_pPlayer->m_AccData.m_Money < kolvo)
			{
				char aChat[128];
				if(rand() % 3 + 1 == 3)
					str_format(aChat, sizeof(aChat), "Man, are you seriously? Take your unexistance money back. You have just %d money counts! Get more.", m_pPlayer->m_AccData.m_Money);
				else if (rand() % 3 + 1 == 2)
					str_format(aChat, sizeof(aChat), "Oh... do you really checked your money counts? Very bad. It's %d.", m_pPlayer->m_AccData.m_Money);
				else
					str_format(aChat, sizeof(aChat), "Sorry, but you have no enough money. Your money is %d.", m_pPlayer->m_AccData.m_Money);
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), aChat);
			}

			if(m_pPlayer->m_AccData.m_Ammoregen + kolvo > 60)
			{
				char aChat[128];
				if(rand() % 2 + 1 == 2)
					str_format(aChat, sizeof(aChat), "Probability ammoregen lvl is higher then 60, decrease amount to %d (your money is %d).", 60 - m_pPlayer->m_AccData.m_Ammoregen, m_pPlayer->m_AccData.m_Money);
				else
					str_format(aChat, sizeof(aChat), "It doesn't work so, man. Minimize your request to %d (amount of money counts is %d)", 60 - m_pPlayer->m_AccData.m_Ammoregen, m_pPlayer->m_AccData.m_Money);	
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), aChat);
			}

			m_pPlayer->m_AccData.m_Money -= kolvo;
			m_pPlayer->m_AccData.m_Ammoregen += kolvo;

			str_format(andg, sizeof(andg), "Your new ammoregen is: %d, Your money: %d",
				m_pPlayer->m_AccData.m_Ammoregen, m_pPlayer->m_AccData.m_Money);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), andg);

			m_pPlayer->m_pAccount->Apply();
			return;
		}

		else if(!str_comp_nocase(supgr, "ammo"))
		{
			if(m_pPlayer->m_AccData.m_Ammo >= 20)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Maximal ammo level!");
			
			if(m_pPlayer->m_AccData.m_Money < kolvo)
			{
				char aChat[128];
				if(rand() % 3 + 1 == 3)
					str_format(aChat, sizeof(aChat), "Man, are you seriously? Take your unexistance money back. You have just %d money counts! Get more.", m_pPlayer->m_AccData.m_Money);
				else if (rand() % 3 + 1 == 2)
					str_format(aChat, sizeof(aChat), "Oh... do you really checked your money counts? Very bad. It's %d.", m_pPlayer->m_AccData.m_Money);
				else
					str_format(aChat, sizeof(aChat), "Sorry, but you have no enough money. Your money is %d.", m_pPlayer->m_AccData.m_Money);
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), aChat);
			}

			if(m_pPlayer->m_AccData.m_Ammo + kolvo > 20)
			{
				char aChat[128];
				if(rand() % 2 + 1 == 2)
					str_format(aChat, sizeof(aChat), "Probability ammo lvl is higher then 20, decrease amount to %d (your money is %d).", 20 - m_pPlayer->m_AccData.m_Ammo, m_pPlayer->m_AccData.m_Money);
				else
					str_format(aChat, sizeof(aChat), "It doesn't work so, man. Minimize your request to %d (amount of money counts is %d)", 20 - m_pPlayer->m_AccData.m_Ammo, m_pPlayer->m_AccData.m_Money);	
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), aChat);
			}

			m_pPlayer->m_AccData.m_Money -= kolvo*10;
			m_pPlayer->m_AccData.m_Ammo += kolvo;

			str_format(andg, sizeof(andg), "Your new amount of ammo is: %d, Your money: %d",
				GameServer()->GetPlayerChar(m_pPlayer->GetCID())->m_mAmmo + m_pPlayer->m_AccData.m_Ammo, m_pPlayer->m_AccData.m_Money);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), andg);

			m_pPlayer->m_pAccount->Apply();
			return;
		}
		else return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "This type doesn't exist.");
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "stats", 5)) // upgr stats
	{
		if(!m_pPlayer->m_AccData.m_UserID)
			return GameServer()->SendMotd(m_pPlayer->GetCID(), "You are not logged in, so I will show just max player stats:\n\nDamage - 20\nHP - 100\nHandle - 300\nAmmoregen - 60\nAmmo - 20\n\n\nTurret max stats:\n\nDamage - 100\nSpeed - 150\nAmmo - 100\nAmmoregen - 75\nRange - 200");

		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "Your stats:\nDamage: %d       HP: %d\nHandle: %d       Ammoregen: %d\nAmmo: %d\n\nYour turret stats:\nDamage - %d       Speed - %d\nAmmo - %d       Ammoregen - %d\nRange - %d\n\n\n\nMax stats of player:\nDamage - 20       HP - 100\nHandle - 300       Ammoregen - 60\nAmmo - 20\n\nMax turret stats:\nDamage - 100       Speed - 150\nAmmo - 45       Ammoregen - 75\nRange - 200", m_pPlayer->m_AccData.m_Dmg, m_pPlayer->m_AccData.m_Health, m_pPlayer->m_AccData.m_Handle, m_pPlayer->m_AccData.m_Ammoregen, m_pPlayer->m_AccData.m_Ammo, m_pPlayer->m_AccData.m_TurretDmg,  m_pPlayer->m_AccData.m_TurretSpeed,  m_pPlayer->m_AccData.m_TurretAmmo,  m_pPlayer->m_AccData.m_TurretShotgun,  m_pPlayer->m_AccData.m_TurretRange);
		GameServer()->SendMotd(m_pPlayer->GetCID(), aBuf);
		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "tupgr", 5))
	{
		if(!m_pPlayer->m_AccData.m_UserID) 
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You are not authed :c Type '/account' for more information.");

		char supgr[256], andg[64];
		int kolvo;

		if(sscanf(Msg->m_pMessage, "/tupgr %s %d", supgr, &kolvo) != 2)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Use /tupgr <type> <amount>"); 
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Types: dmg, speed, ammo, ammoregen, range.");
			return;

		}

		else if(!str_comp_nocase(supgr, "dmg"))
		{
			if(m_pPlayer->m_AccData.m_TurretDmg >= 100)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "100 damage's level is the maximal :/");

			if(m_pPlayer->m_AccData.m_TurretMoney < kolvo)
			{
				char aChat[128];
				if(rand() % 3 + 1 == 3)
					str_format(aChat, sizeof(aChat), "You have not enough money (your money is %d).", m_pPlayer->m_AccData.m_TurretMoney);
				else if (rand() % 3 + 1 == 2)
					str_format(aChat, sizeof(aChat), "Unfortunately, there is no enough money. (you have money %d money counts).", m_pPlayer->m_AccData.m_TurretMoney);
				else
					str_format(aChat, sizeof(aChat), "Oh no! Your money is %d. It's not enough :(", m_pPlayer->m_AccData.m_TurretMoney);
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), aChat);
			}

			if(m_pPlayer->m_AccData.m_TurretDmg + kolvo > 100)
			{

				char aChat[128];
				str_format(aChat, sizeof(aChat), "Probability damage lvl is higher then 100, decrease amount to %d (your money is %d).", 100 - m_pPlayer->m_AccData.m_TurretDmg, m_pPlayer->m_AccData.m_TurretMoney);			
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), aChat);
			}

			m_pPlayer->m_AccData.m_TurretMoney -= kolvo;
			m_pPlayer->m_AccData.m_TurretDmg += kolvo;

			str_format(andg, sizeof(andg), "You new turret's damage is: %d. Your turret's money is %d", m_pPlayer->m_AccData.m_TurretDmg, m_pPlayer->m_AccData.m_TurretMoney);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), andg);

			m_pPlayer->m_pAccount->Apply();
			return;
		}

		else if(!str_comp_nocase(supgr, "speed"))
		{
			if(m_pPlayer->m_AccData.m_TurretSpeed >= 150)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "150 speed level is the maximal :/");

			if(m_pPlayer->m_AccData.m_TurretMoney < kolvo)
			{
				char aChat[128];
				if(rand() % 3 + 1 == 3)
					str_format(aChat, sizeof(aChat), "You have not enough money (your money is %d).", m_pPlayer->m_AccData.m_TurretMoney);
				else if (rand() % 3 + 1 == 2)
					str_format(aChat, sizeof(aChat), "Unfortunately, there is no enough money. (you have money %d money counts).", m_pPlayer->m_AccData.m_TurretMoney);
				else
					str_format(aChat, sizeof(aChat), "Oh no! Your money is %d. It's not enough :(", m_pPlayer->m_AccData.m_TurretMoney);
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), aChat);
			}

			if(m_pPlayer->m_AccData.m_TurretSpeed + kolvo > 150)
			{

				char aChat[128];
				str_format(aChat, sizeof(aChat), "Probability speed lvl is higher then 150, decrease amount to %d (your money is %d).", 150 - m_pPlayer->m_AccData.m_TurretSpeed, m_pPlayer->m_AccData.m_TurretMoney);			
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), aChat);
			}

			m_pPlayer->m_AccData.m_TurretMoney -= kolvo;
			m_pPlayer->m_AccData.m_TurretSpeed += kolvo;

			str_format(andg, sizeof(andg), "You new turret's speed is: %d. Your turret's money is %d", m_pPlayer->m_AccData.m_TurretSpeed, m_pPlayer->m_AccData.m_TurretMoney);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), andg);

			m_pPlayer->m_pAccount->Apply();
			return;
		}

		else if(!str_comp_nocase(supgr, "ammo"))
		{
			if(m_pPlayer->m_AccData.m_TurretAmmo >= 45)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "45 ammo level is the maximal :/");

			if(m_pPlayer->m_AccData.m_TurretMoney < kolvo)
			{
				char aChat[128];
				if(rand() % 3 + 1 == 3)
					str_format(aChat, sizeof(aChat), "You have not enough money (your money is %d).", m_pPlayer->m_AccData.m_TurretMoney);
				else if (rand() % 3 + 1 == 2)
					str_format(aChat, sizeof(aChat), "Unfortunately, there is no enough money. (you have money %d money counts).", m_pPlayer->m_AccData.m_TurretMoney);
				else
					str_format(aChat, sizeof(aChat), "Oh no! Your money is %d. It's not enough :(", m_pPlayer->m_AccData.m_TurretMoney);
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), aChat);
			}

			if(m_pPlayer->m_AccData.m_TurretAmmo + kolvo > 45)
			{

				char aChat[128];
				str_format(aChat, sizeof(aChat), "Probability ammo lvl is higher then 45, decrease amount to %d (your money is %d).", 45 - m_pPlayer->m_AccData.m_TurretAmmo, m_pPlayer->m_AccData.m_TurretMoney);			
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), aChat);
			}

			m_pPlayer->m_AccData.m_TurretMoney -= kolvo;
			m_pPlayer->m_AccData.m_TurretAmmo += kolvo;

			str_format(andg, sizeof(andg), "You new turret's ammo is: %d. Your turret's money is %d", m_pPlayer->m_AccData.m_TurretAmmo, m_pPlayer->m_AccData.m_TurretMoney);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), andg);

			m_pPlayer->m_pAccount->Apply();
			return;
		}

		else if(!str_comp_nocase(supgr, "ammoregen"))
		{
			if(m_pPlayer->m_AccData.m_TurretShotgun >= 75)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "75 ammoregen level is the maximal :/");

			if(m_pPlayer->m_AccData.m_TurretMoney < kolvo)
			{
				char aChat[128];
				if(rand() % 3 + 1 == 3)
					str_format(aChat, sizeof(aChat), "You have not enough money (your money is %d).", m_pPlayer->m_AccData.m_TurretMoney);
				else if (rand() % 3 + 1 == 2)
					str_format(aChat, sizeof(aChat), "Unfortunately, there is no enough money. (you have money %d money counts).", m_pPlayer->m_AccData.m_TurretMoney);
				else
					str_format(aChat, sizeof(aChat), "Oh no! Your money is %d. It's not enough :(", m_pPlayer->m_AccData.m_TurretMoney);
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), aChat);
			}

			if(m_pPlayer->m_AccData.m_TurretShotgun + kolvo > 75)
			{

				char aChat[128];
				str_format(aChat, sizeof(aChat), "Probability ammoregen lvl is higher then 75, decrease amount to %d (your money is %d).", 75 - m_pPlayer->m_AccData.m_TurretShotgun, m_pPlayer->m_AccData.m_TurretMoney);			
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), aChat);
			}

			m_pPlayer->m_AccData.m_TurretMoney -= kolvo;
			m_pPlayer->m_AccData.m_TurretShotgun += kolvo;

			str_format(andg, sizeof(andg), "You new turret's ammoregen is: %d. Your turret's money is %d", m_pPlayer->m_AccData.m_TurretShotgun, m_pPlayer->m_AccData.m_TurretMoney);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), andg);

			m_pPlayer->m_pAccount->Apply();
			return;
		}

		else if(!str_comp_nocase(supgr, "range"))
		{
			if(m_pPlayer->m_AccData.m_TurretRange >= 200)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "200 range level is the maximal :/");
			
			if(m_pPlayer->m_AccData.m_TurretMoney < kolvo)
			{
				char aChat[128];
				if(rand() % 3 + 1 == 3)
					str_format(aChat, sizeof(aChat), "You have not enough money (your money is %d).", m_pPlayer->m_AccData.m_TurretMoney);
				else if (rand() % 3 + 1 == 2)
					str_format(aChat, sizeof(aChat), "Unfortunately, there is no enough money. (you have money %d money counts).", m_pPlayer->m_AccData.m_TurretMoney);
				else
					str_format(aChat, sizeof(aChat), "Oh no! Your money is %d. It's not enough :(", m_pPlayer->m_AccData.m_TurretMoney);
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), aChat);
			}


			if(m_pPlayer->m_AccData.m_TurretRange + kolvo > 200)
			{

				char aChat[128];
				str_format(aChat, sizeof(aChat), "Probability range lvl is higher then 200, decrease amount to %d (your money is %d).", 200 - m_pPlayer->m_AccData.m_TurretRange, m_pPlayer->m_AccData.m_TurretMoney);			
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), aChat);
			}

			m_pPlayer->m_AccData.m_TurretMoney -= kolvo;
			m_pPlayer->m_AccData.m_TurretRange += kolvo;

			str_format(andg, sizeof(andg), "You new turret's range is: %d. Your turret's money is %d", m_pPlayer->m_AccData.m_TurretRange, m_pPlayer->m_AccData.m_TurretMoney);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), andg);

			m_pPlayer->m_pAccount->Apply();
			return;
		}
		else
		{
			if(rand() % 2 + 1 == 1)
				str_format(andg, sizeof(andg), "No such turret upgrade: '%s'!", supgr);
			else 
				str_format(andg, sizeof(andg), "There is no turret upgrade '%s'", supgr);

			GameServer()->SendChatTarget(m_pPlayer->GetCID(), andg);
			return;
		}
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "t", 1))
	{
		if(!m_pPlayer->m_AccData.m_UserID) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You are not logged in! Type '/account' for more information!");

		char supgr[256], andg[64];
		if(sscanf(Msg->m_pMessage, "/t %s", supgr) != 1)
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Use /t info");

		if(!str_comp_nocase(supgr, "info"))
		{		
			GameServer()->SendMotd(m_pPlayer->GetCID(), "           Welcome to info about turrets\n\n/t info - show information\n/t help - show turret help\n/stats - view stats of turret's upgrade\n\n        Upgrade attack speed:\n/tupgr speed\n\n        Upgrade turret's damage\n/tupgr dmg\n\n        Increase turret's amount of ammo\n/tupgr ammo\n\n        Do more speedy an ammoregen\n/tupgr ammoregen\n\n        Increase radius to shoot\n/tupgr range");
			return;
		}

		else if(!str_comp_nocase(supgr, "help"))
		{
			GameServer()->SendMotd(m_pPlayer->GetCID(), "                Turret's help\n\nTurret can be placed by ghost emoticon.  \n\nTurrets can be created by 5 different weapons.\nHammer turret pulls to itself and beats.\n\nGun turret just shoots.\n\nShotgun turret too just shoots.\n\nGrenade turret puts grenades in her way. Creating by two-directed ghost-emote.\n\nLaser turret places a laser each 40 sec if the zombie reach it's line.");
			return;
		}
	}

	else if(!str_comp_nocase(Msg->m_pMessage+1, "me") || !str_comp_nocase(Msg->m_pMessage+1, "status"))
	{
		if(!m_pPlayer->m_AccData.m_UserID)
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You are not logged in! Type '/account' for more information!");

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "                      Account\n\n\nLogin: %s\nPassword: %s\nLevel: %d\nExp: %d\nMoney: %d\n\nTurret level: %d\nTurret exp: %d\nTurret money: %d\n\nFreeze: %s\n\nLoses [Z]: %d, Wins [Z]: %d",
			m_pPlayer->m_AccData.m_Username,
			m_pPlayer->m_AccData.m_Password,
			m_pPlayer->m_AccData.m_Level,
			m_pPlayer->m_AccData.m_Exp,
			m_pPlayer->m_AccData.m_Money,
			m_pPlayer->m_AccData.m_TurretLevel,
			m_pPlayer->m_AccData.m_TurretExp,
			m_pPlayer->m_AccData.m_TurretMoney,
			m_pPlayer->m_AccData.m_Freeze ? "yes" : "no",
			m_pPlayer->m_AccData.m_Winner,
			m_pPlayer->m_AccData.m_Luser);
		
		GameServer()->SendMotd(m_pPlayer->GetCID(), aBuf);
		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "password", 8))
	{
		if(!m_pPlayer->m_AccData.m_UserID)
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You are not logged in! Type '/account' for more information!");

		char NewPassword[256];
		if(sscanf(Msg->m_pMessage, "/password %s", NewPassword) != 1)
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Use \"/password <new password>\"");

		m_pPlayer->m_pAccount->NewPassword(NewPassword);
		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "vip", 3))
	{
		GameServer()->SendMotd(m_pPlayer->GetCID(), "Sorry, we are not selling VIP-status. Only one player have this - Qywinc. He have VIP-status because 2 years ago he bought permanent VIP status, and we can't deny to him.");
		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "info", 4))
	{
		GameServer()->SendMotd(m_pPlayer->GetCID(), "                      Information\n\nxPanic is an old project which some programmers want to revive, and our try haven't another way. We want to feel the nostalgy and get fun from playing xPanic with some new features and nice support. If you need help, write on discord server or DM us.\n\nOwners: gerdoe & Ωυaηtυm\nDiscord Server: discord.gg/9TstDDR");
		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "help", 4))
    {
		GameServer()->SendMotd(m_pPlayer->GetCID(), "            Welcome to help!\n\n/account - acc system help\n/news - check new features\n/rules - for read the rules of xPanic\n/w - send personal msg to somebody\n/cmdlist - cmds of server\n/turret info - info about turrets\n/levels - info about level\n/shop - shop score tees");
		return;
    }

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "levels", 6))
	{
		GameServer()->SendMotd(m_pPlayer->GetCID(), "                        Levels:\n\nSince 10 lvl you have autohammer\n\nSince 40: your shotgun spread growing by 1 bullet per 10 levels\n\nSince 50: +10 ammo\n\nSince 100: +10 ammo");
		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "account", 7))
	{
		GameServer()->SendMotd(m_pPlayer->GetCID(), "                   Account help\n\n/register <username> <pass>\n    Register new account\n\n/login <username> <pass>\n    Log into your account\n\n/logout\n    Log out from your account\n\n/password\n    Change account password");
		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "shop", 4))
	{
		GameServer()->SendMotd(m_pPlayer->GetCID(), "                      Shop\n\n/range [10 score]   [Zombie]\nGet more range of hammer.\n\n/heart [15 score]   [Zombie]\nA heart of a first zombie. \n\n/slowbomb [15 score]   [Zombie]\nBuy a bomb which slows a human.\n\n/jump [3 score]   [Neutral]\n+1 jump\n\n/armorwall [10 score]   [Human]\n+10 seconds of armorwall\n\n/heartshield [15 score]   [Human]\nBuy protection from zombie's heart");
		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "news", 4))
	{
		GameServer()->SendMotd(m_pPlayer->GetCID(), "                        News\n\n[02.07.2020] Third restart of xPanic (added /upgr <type> <amount> and some features)\n\n[11.08.2020] Fixed announcements, added armorwall, added a counter for doors (see your armor before opening the door)\n\n[12.08.2020] Hearts shows how much hp do you have (genius xd)\n\n[13.08.2020] Added heartshield\n\n[16.08.2020] VERY tryharded and added a slowbomb");

		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "features", 8))
	{
		GameServer()->SendMotd(m_pPlayer->GetCID(), "                      Features\nHeart is a entity of hp rounding around of you and if you scroll down, it goes to crossfire mode. If you clicking fire, tee throws it and heart targets to closest human. If it touches the human, he is infecting.\n\nArmorwall is a shield created by 3 armor entities, activating via sound emote. It protects you and pushing out zombies like a door.\n\nHeartshield is an armor from zombie's heart. If zombie throwing heart and you are target of heart (and you have heartshield), heart and heartshield will explode.\n\nSlowbomb is a grenade which slows a human in some radius. You can throw it like heart and you have 3 seconds to explode it manually by hammer or it will explode automatically.");

		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "policehelp", 10))
	{
		if(!m_pPlayer->m_AccData.m_UserID) 
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You are not logged in! Type '/account' for more information!");

		if(m_pPlayer->m_AccData.m_PlayerState == 1 || GameServer()->Server()->IsAuthed(m_pPlayer->GetCID()))
			GameServer()->SendMotd(m_pPlayer->GetCID(), "                Help for police\n\n     /freeze <id>\nfreeze/unfreeze player\n\n     /mute <id> <time>\nMute somebody for some time");
		else
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Access denied :(");
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "cmdlist", 7)) //TODO: rework
	{
		if(m_pPlayer->m_AccData.m_PlayerState != 1)
		GameServer()->SendMotd(m_pPlayer->GetCID(), "                  Command list\n\n     Account system:\n/register, /login, /logout\n\n     Information:\n/rules, /help, /info\n\n     Upgrade system:\n/stats, /upgr\n\n     Scoreshop for tees:\n/shop");
		else
			GameServer()->SendMotd(m_pPlayer->GetCID(), "                  Command list\n\n     Account system:\n/register, /login, /logout\n\n     Information:\n/rules, /help, /info\n\n     Upgrade system:\n/stats, /upgr\n\n     Scoreshop for tees:\n/shop\n\n     Help for policemans:\n/policehelp");
		return;
	}	

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "rules", 5))
	{
		GameServer()->SendMotd(m_pPlayer->GetCID(), "                  Welcome to rules!\n\n1 - If you don't know rules, you won't be able to avoid the punishment.\n\n2 - You can't sell anything for real money (freeze account).\n\n3 - Don't insult other players (mute).\n\n4 - Don't advertise other servers (long mute).\n\n5 - Don't use bugs, except the case of showing it to admin (ban or bonus).\n\n6 - Don't farm to get more experience or money (freeze).");
		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "heartshield", 11))
	{
		if(!m_pPlayer->m_AccData.m_UserID)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You are not logged in! Type '/account' for more information!");
			return;
		}
		if(!GameServer()->GetPlayerChar(m_pPlayer->GetCID()))
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Use only if you alive!");
			return;
		}
		if(GameServer()->m_World.m_Paused)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please, wait for end of round.");
			return;
		}
		if(m_pPlayer->m_Score < 15)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You need 15 score.");
			return;
		}
		if(m_pPlayer->GetTeam() == TEAM_RED)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "But... you don't really need this, you are zombie...");
			return;
		}
		if(GameServer()->GetPlayerChar(m_pPlayer->GetCID())->HeartShield)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You can't buy second heart shield, sorry.");
			return;
		}

		GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Done!");
		GameServer()->GetPlayerChar(m_pPlayer->GetCID())->SwitchHeartShield(1);
		m_pPlayer->m_Score -= 15;
		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "heart", 5))
	{
		if(!m_pPlayer->m_AccData.m_UserID)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You are not logged in! Type '/account' for more information!");
			return;
		}

		if(!GameServer()->GetPlayerChar(m_pPlayer->GetCID()))
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "It's usable only if you alive!");
			return;
		}

		if(GameServer()->m_World.m_Paused)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please wait for end of round!");
			return;
		}

		if(m_pPlayer->GetTeam() != TEAM_RED)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Available only for zombies!");
			return;
		}

		if(m_pPlayer->m_Score < 15)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Not so fast, man. You need 15 score.");
			return;
		}
		
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Done! :)");
		m_pPlayer->m_Score -= 15;
		m_pPlayer->m_ActivesLife = false;
		m_pPlayer->m_LifeActives = false;
		new CLifeHearth(&GameServer()->m_World, vec2(0, 0), m_pPlayer->GetCID());
		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "slowbomb", 8))
	{
		if(!m_pPlayer->m_AccData.m_UserID)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You are not logged in! Type '/account' for more information!");
			return;
		}

		if(!GameServer()->GetPlayerChar(m_pPlayer->GetCID()))
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "It's usable only if you alive!");
			return;
		}

		if(GameServer()->m_World.m_Paused)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please wait for end of round!");
			return;
		}

		if(m_pPlayer->GetTeam() != TEAM_RED)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Available only for zombies!");
			return;
		}

		if(m_pPlayer->m_Score < 15)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Not so fast, man. You need 15 score.");
			return;
		}
		if(GameServer()->GetPlayerChar(m_pPlayer->GetCID())->slowBomb || GameServer()->GetPlayerChar(m_pPlayer->GetCID())->ThrownBomb)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Sorry, but you can't buy second slowbomb till first will not detonate.");
			return;
		}

		GameServer()->GetPlayerChar(m_pPlayer->GetCID())->SwitchSlowBomb(1);
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Done! :)");
		m_pPlayer->m_Score -= 15;
		
		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "jump", 4))
	{
		if(!m_pPlayer->m_AccData.m_UserID)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You are not logged in! Type '/account' for more information!");
			return;
		}
		if(!GameServer()->GetPlayerChar(m_pPlayer->GetCID()))
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Use only if you alive!");
			return;
		}
		if(GameServer()->m_World.m_Paused)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please, wait for end of round.");
			return;
		}
		if(m_pPlayer->m_Score < 3)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You need 3 score.");
			return;
		}

		GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Done!");
		m_pPlayer->m_JumpsShop++;
		m_pPlayer->m_Score -= 3;
		GameServer()->GetPlayerChar(m_pPlayer->GetCID())->m_Core.m_Jumps += 1;
		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "roundtoint", 10)) // округлятор228
	{
		float value;

		if(!m_pPlayer->m_AccData.m_UserID)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "залогинься заебал");
			return;
		}
		if(!GameServer()->GetPlayerChar(m_pPlayer->GetCID()))
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "я чё нахуй долбоёб округлять пустому месту? а ну-ка возродись нахуй слыш");
			return;
		}
		if(GameServer()->m_World.m_Paused)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "блять подожди пока начнётся раунд ты ебанутый?");
			return;
		}

		if(sscanf(Msg->m_pMessage, "/roundtoint %f", &value) != 1)
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Use /roundtoint <value>");
		
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "ты конечно ебанутый, но %f округляется до %d", value, round_to_int(value));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);

	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "armorwall", 9))
	{
		if(!m_pPlayer->m_AccData.m_UserID)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You are not logged in! Type '/account' for more information!");
			return;
		}
		if(!GameServer()->GetPlayerChar(m_pPlayer->GetCID()))
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Use only if you alive!");
			return;
		}
		if(GameServer()->m_World.m_Paused)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please, wait for end of round.");
			return;
		}
		if(m_pPlayer->m_Score < 10)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You need 10 score.");
			return;
		}
		if(m_pPlayer->GetTeam() == TEAM_RED)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Are you really sure? Zombie can't have an armorwall...");
			return;
		}

		GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Done!");
		GameServer()->GetPlayerChar(m_pPlayer->GetCID())->IncreaseArmorwall(10);
		m_pPlayer->m_Score -= 10;
		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "range", 5))
	{
		if(!m_pPlayer->m_AccData.m_UserID)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You are not logged in! Type '/account' for more information!");
			return;
		}
		if(!GameServer()->GetPlayerChar(m_pPlayer->GetCID()))
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Use only if you alive!");
			return;
		}
		if(GameServer()->m_World.m_Paused)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please wait for end of round!");
			return;
		}
		if(m_pPlayer->GetTeam() != TEAM_RED)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Available only for zombies!");
			return;
		}
		if(m_pPlayer->m_Score < 10)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You need 10 score!");
			return;
		}

		GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Done!");
		m_pPlayer->m_RangeShop = true;
		m_pPlayer->m_Score -= 10;
		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "prefix", 6))
	{
		if(!m_pPlayer->m_AccData.m_UserID) 
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You are not logged in! Type '/account' for more information!");

		if(m_pPlayer->m_AccData.m_PlayerState == 2 || GameServer()->Server()->IsAuthed(m_pPlayer->GetCID()))
		{
			char aBuf[36];
			m_pPlayer->m_Prefix ^= true;
			str_format(aBuf, sizeof(aBuf), "Your prefix has been %s", m_pPlayer->m_Prefix ? "enabled :)" : "disabled :(");
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
		}
		return;
	}
	
	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "w", 1))
	{
		int ClientID;
		if(sscanf(Msg->m_pMessage, "/w %d", &ClientID) != 1)
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Use /w <id> <text>");
		
		if(ClientID < 0 || ClientID >= MAX_CLIENTS || !GameServer()->m_apPlayers[ClientID])
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "There is no such player!");
			return;
		}
		
		char aBuf[128];
		str_copy(aBuf, Msg->m_pMessage+4, 128);
		GameServer()->SendPM(m_pPlayer->GetCID(), ClientID, aBuf);
		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "setlvl", 6))
	{
		if(!GameServer()->Server()->IsAuthed(m_pPlayer->GetCID()))
			return;

		int ClientID, Level;
		if((sscanf(Msg->m_pMessage, "/setlvl %d %d", &ClientID, &Level)) != 2)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please use: /setlvl <id> <lvl>");
			return;
		}
		
		if(ClientID < 0 || ClientID >= MAX_CLIENTS || !GameServer()->m_apPlayers[ClientID])
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "There is no such player!");
			return;
		}
		
		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "You had set %s's level to %d",
			GameServer()->Server()->ClientName(ClientID), Level);
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);

		str_format(aBuf, sizeof(aBuf), "Your level was changed to %d!", Level);
		GameServer()->SendChatTarget(ClientID, aBuf);
		
		if(Level > MAX_INT)
			GameServer()->m_apPlayers[ClientID]->m_AccData.m_Level = MAX_INT;
		else if(Level <= 0)
			GameServer()->m_apPlayers[ClientID]->m_AccData.m_Level = 0;
		else GameServer()->m_apPlayers[ClientID]->m_AccData.m_Level = Level;
		
		GameServer()->m_apPlayers[ClientID]->m_AccData.m_Exp = 0;
		GameServer()->m_apPlayers[ClientID]->m_pAccount->Apply();
		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "reset", 5))
	{
		if(!GameServer()->Server()->IsAuthed(m_pPlayer->GetCID()))
			return;

		int ClientID;
		if((sscanf(Msg->m_pMessage, "/reset %d", &ClientID)) != 1)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please use: /reset <id>");
			return;
		}
		
		if(ClientID < 0 || ClientID >= MAX_CLIENTS || !GameServer()->m_apPlayers[ClientID])
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "There is no such player!");
			return;
		}
		
		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "You have already reseted %s's account.",
			GameServer()->Server()->ClientName(ClientID));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);

		str_format(aBuf, sizeof(aBuf), "Your account was reseted!");
		GameServer()->SendChatTarget(ClientID, aBuf);
		
		GameServer()->m_apPlayers[ClientID]->m_AccData.m_Money = GameServer()->m_apPlayers[ClientID]->m_AccData.m_Level;

		GameServer()->m_apPlayers[ClientID]->m_AccData.m_Dmg = 0;
		GameServer()->m_apPlayers[ClientID]->m_AccData.m_Health = 0;
		GameServer()->m_apPlayers[ClientID]->m_AccData.m_Ammoregen = 0;
		GameServer()->m_apPlayers[ClientID]->m_AccData.m_Ammo = 0;
		GameServer()->m_apPlayers[ClientID]->m_AccData.m_Handle = 0;\

		GameServer()->m_apPlayers[ClientID]->m_AccData.m_TurretMoney = GameServer()->m_apPlayers[ClientID]->m_AccData.m_TurretLevel;
		GameServer()->m_apPlayers[ClientID]->m_AccData.m_TurretDmg = 0;
		GameServer()->m_apPlayers[ClientID]->m_AccData.m_TurretSpeed = 0;
		GameServer()->m_apPlayers[ClientID]->m_AccData.m_TurretRange = 0;
		GameServer()->m_apPlayers[ClientID]->m_AccData.m_TurretShotgun = 0;
		
		GameServer()->m_apPlayers[ClientID]->m_AccData.m_Exp = 0;
		GameServer()->m_apPlayers[ClientID]->m_pAccount->Apply();
		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "settlvl", 7))
	{
		if(!GameServer()->Server()->IsAuthed(m_pPlayer->GetCID()))
			return;

		int ClientID, Level;
		if((sscanf(Msg->m_pMessage, "/settlvl %d %d", &ClientID, &Level)) != 2)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please use: /settlvl <id> <lvl>");
			return;
		}
		
		if(ClientID < 0 || ClientID >= MAX_CLIENTS || !GameServer()->m_apPlayers[ClientID])
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "There is no such player!");
			return;
		}
		
		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "You had set %s's turret level to %d",
			GameServer()->Server()->ClientName(ClientID), Level);
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);

		str_format(aBuf, sizeof(aBuf), "Your turret's level was changed to %d!", Level);
		GameServer()->SendChatTarget(ClientID, aBuf);
		
		if(Level > MAX_INT)
			GameServer()->m_apPlayers[ClientID]->m_AccData.m_TurretLevel = MAX_INT;
		else if(Level <= 0)
			GameServer()->m_apPlayers[ClientID]->m_AccData.m_TurretLevel = 0;
		else GameServer()->m_apPlayers[ClientID]->m_AccData.m_TurretLevel = Level;
		
		GameServer()->m_apPlayers[ClientID]->m_AccData.m_TurretExp = 0;
		GameServer()->m_apPlayers[ClientID]->m_pAccount->Apply();
		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "setmoney", 8))
	{
		if(!GameServer()->Server()->IsAuthed(m_pPlayer->GetCID()))
			return;
		
		int ClientID, Amount;
		if((sscanf(Msg->m_pMessage, "/setmoney %d %d", &ClientID, &Amount)) != 2)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please use: /setmoney <id> <amount>");
			return;
		} 

		if(ClientID < 0 || ClientID >= MAX_CLIENTS || !GameServer()->m_apPlayers[ClientID])
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "There is no such player!");
			return;
		}

		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "You had set %s's money to %d",
			GameServer()->Server()->ClientName(ClientID), Amount);
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);

		str_format(aBuf, sizeof(aBuf), "Your money was changed to %d!", Amount);
		GameServer()->SendChatTarget(ClientID, aBuf);
		
		if(Amount > MAX_INT)
			GameServer()->m_apPlayers[ClientID]->m_AccData.m_Money = MAX_INT;
		else if(Amount <= 0)
			GameServer()->m_apPlayers[ClientID]->m_AccData.m_Money = 0;
		else GameServer()->m_apPlayers[ClientID]->m_AccData.m_Money = Amount;

		GameServer()->m_apPlayers[ClientID]->m_pAccount->Apply();
		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "settmoney", 9))
	{
		if(!GameServer()->Server()->IsAuthed(m_pPlayer->GetCID()))
			return;

		int ClientID, TurretAmount;
		if((sscanf(Msg->m_pMessage, "/settmoney %d %d", &ClientID, &TurretAmount)) != 2)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please use: /settmoney <id> <amount>");
			return;
		}
		
		if(ClientID < 0 || ClientID >= MAX_CLIENTS || !GameServer()->m_apPlayers[ClientID])
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "There is no such player!");
			return;
		}
		
		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "You had set %s's turret money to %d",
			GameServer()->Server()->ClientName(ClientID), TurretAmount);
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);

		str_format(aBuf, sizeof(aBuf), "Your turret's money was changed to %d!", TurretAmount);
		GameServer()->SendChatTarget(ClientID, aBuf);
		
		if(TurretAmount > MAX_INT)
			GameServer()->m_apPlayers[ClientID]->m_AccData.m_TurretMoney = MAX_INT;
		else if(TurretAmount <= 0)
			GameServer()->m_apPlayers[ClientID]->m_AccData.m_TurretMoney = 0;
		else GameServer()->m_apPlayers[ClientID]->m_AccData.m_TurretMoney = TurretAmount;
		
		GameServer()->m_apPlayers[ClientID]->m_pAccount->Apply();
		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "getmoney", 8))
	{
		int ClientID, size;
		if((sscanf(Msg->m_pMessage, "/getmoney %d", &ClientID)) != 1)
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please use: /getmoney <id>");
		
		if(ClientID < 0 || ClientID >= MAX_CLIENTS || !GameServer()->m_apPlayers[ClientID])
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "There is no such player!");
			return;
		}

		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "'%s' money is %d", 
			GameServer()->Server()->ClientName(ClientID), GameServer()->m_apPlayers[ClientID]->m_AccData.m_Money);
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);

		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "freeze", 6))
	{
		if(GameServer()->Server()->IsAuthed(m_pPlayer->GetCID()) || m_pPlayer->m_AccData.m_PlayerState == 1)
		{
			int ClientID;
			if(sscanf(Msg->m_pMessage, "/freeze %d", &ClientID) != 1) 
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please use: /freeze <id>");
			
			if(ClientID < 0 || ClientID >= MAX_CLIENTS || !GameServer()->m_apPlayers[ClientID])
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "There is no such player!");
				return;
			}
			if(ClientID == m_pPlayer->GetCID())
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You can not freeze your account!");
				return;
			}

			if(!GameServer()->m_apPlayers[ClientID]->m_AccData.m_UserID)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "The player is not logged in account!");
				return;
			}
			
			GameServer()->m_apPlayers[ClientID]->m_AccData.m_Freeze ^= true;
			GameServer()->m_apPlayers[ClientID]->m_pAccount->Apply();

			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "You have already %s account of player %s",
				GameServer()->m_apPlayers[ClientID]->m_AccData.m_Freeze ? "freezed":"unfreezed", GameServer()->Server()->ClientName(ClientID));
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);

			str_format(aBuf, sizeof(aBuf), "Your account has been %s by police - %s",
				GameServer()->m_apPlayers[ClientID]->m_AccData.m_Freeze ? "freezed":"unfreezed", GameServer()->Server()->ClientName(m_pPlayer->GetCID()));
			GameServer()->SendChatTarget(ClientID, aBuf);

			str_format(aBuf, sizeof(aBuf), "Police '%s' freezed account of player '%s', his login - '%s'",
				GameServer()->Server()->ClientName(m_pPlayer->GetCID()), GameServer()->Server()->ClientName(ClientID), GameServer()->m_apPlayers[ClientID]->m_AccData.m_Username);

			dbg_msg("police", aBuf);
			
			str_format(aBuf, sizeof(aBuf), "'%s' was %s by '%s'", GameServer()->Server()->ClientName(ClientID),
				 GameServer()->m_apPlayers[ClientID]->m_AccData.m_Freeze ? "freeed":"unfreezed", GameServer()->Server()->ClientName(m_pPlayer->GetCID()));		
			DEBUG("cmds", aBuf);
		}
		
		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "kick", 4))
	{
		if(GameServer()->Server()->IsAuthed(m_pPlayer->GetCID()) || m_pPlayer->m_AccData.m_PlayerState == 1)
		{
			int ClientID;
			if((sscanf(Msg->m_pMessage, "/kick %d", &ClientID)) != 1)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please use: /kick <id>");
			
			if(ClientID < 0 || ClientID >= MAX_CLIENTS || !GameServer()->m_apPlayers[ClientID])
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "There is no such player!");
				return;
			}

			GameServer()->Server()->Kick(ClientID, "You was kicked by the Police!");

			char aBuf[64];
			str_format(aBuf, sizeof(aBuf), "'%s' was kicked by '%s'",
				GameServer()->Server()->ClientName(ClientID), GameServer()->Server()->ClientName(m_pPlayer->GetCID()));		
			DEBUG("cmds", aBuf);
		}
		
		return;
	}
	
	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "setscore", 8))
	{
		if(!GameServer()->Server()->IsAuthed(m_pPlayer->GetCID()))
			return;
		
			int score;
			int ClientID;			
			if((sscanf(Msg->m_pMessage, "/setscore %d %d", &ClientID, &score)) != 2)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please use: /score <id> <score>");
			
			if(ClientID < 0 || ClientID >= MAX_CLIENTS || !GameServer()->m_apPlayers[ClientID])
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "There is no such player!");
				return;
			}
			char aBuf[64];
			str_format(aBuf, sizeof(aBuf), "You had set %s's score to %d",
			GameServer()->Server()->ClientName(ClientID), score);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);

			str_format(aBuf, sizeof(aBuf), "Your score's was changed to %d!", score);
			GameServer()->SendChatTarget(ClientID, aBuf);
			GameServer()->m_apPlayers[ClientID]->m_Score = score;		
		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "ban", 3))
	{
		if(!GameServer()->Server()->IsAuthed(m_pPlayer->GetCID()))
			return;

		int ClientID;
		int time;
		if(sscanf(Msg->m_pMessage, "/ban %d %d", &ClientID, &time) != 2)
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please use: /ban <id> <time>");

		if(ClientID < 0 || ClientID >= MAX_CLIENTS || !GameServer()->m_apPlayers[ClientID])
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "There is no such player!");
			return;
		}

		char aBuf[128];
		char aAddrStr[NETADDR_MAXSTRSIZE] = {0};
		GameServer()->Server()->GetClientAddr(ClientID, aAddrStr, sizeof(aAddrStr));
		str_format(aBuf, sizeof(aBuf), "ban %s %d You was banned by the MVD", aAddrStr, time);
		GameServer()->Console()->ExecuteLine(aBuf);

		str_format(aBuf, sizeof(aBuf), "'%s' was banned by '%s'",
			GameServer()->Server()->ClientName(ClientID), GameServer()->Server()->ClientName(m_pPlayer->GetCID()));	
		DEBUG("cmds", aBuf);
		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "mute", 4))
	{
		int ClientID, Time;
		char aBuf[512], Reason[512];

		if(sscanf(Msg->m_pMessage, "/mute %d %d %s", &ClientID, &Time, &Reason) < 2)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please use: /mute <id> <time> [<reason>]");
			return;
		}
		
		if(sscanf(Msg->m_pMessage, "/mute %d %d %s", &ClientID, &Time, &Reason) < 3)
			str_format(Reason, sizeof(Reason), "no reason");
		
		if(ClientID < 0 || ClientID >= MAX_CLIENTS || !GameServer()->m_apPlayers[ClientID])
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "There is no such player!");
			return;
		}
		
		if(Time <= 0)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Cannot mute for 0 seconds!");
			return;
		}

		char aVote[VOTE_DESC_LENGTH] = {0};
		char aCommand[VOTE_CMD_LENGTH] = {0};

		str_format(aVote, sizeof(aVote), "%dsec mute %s", Time, GameServer()->Server()->ClientName(ClientID));

		str_format(aCommand, sizeof(aCommand), "muteid %d %d", ClientID, Time);
		str_format(aBuf, sizeof(aBuf), "'%s' called for vote to mute '%s' for %d seconds (%s)",
				GameServer()->Server()->ClientName(m_pPlayer->GetCID()), GameServer()->Server()->ClientName(ClientID), Time, Reason);
		
		if(GameServer()->Server()->IsAuthed(m_pPlayer->GetCID()) || m_pPlayer->m_AccData.m_PlayerState == 1)
		{
			if(sscanf(Msg->m_pMessage, "/mute %d %d %s", &ClientID, &Time, &Reason) > 2)
			{
				str_format(aBuf, sizeof(aBuf), "You have been muted for %d seconds (%s)", Time, Reason);
				GameServer()->SendChatTarget(ClientID, aBuf);
			}
						
			GameServer()->CallVote(m_pPlayer->GetCID(), aVote, aCommand, Reason, aBuf, true);
			GameServer()->m_VoteEnforce = 5;
		}
		else GameServer()->CallVote(m_pPlayer->GetCID(), aVote, aCommand, Reason, aBuf, false);
		
		str_format(aBuf, sizeof(aBuf), "'%s' was muted by '%s'",
			GameServer()->Server()->ClientName(ClientID), GameServer()->Server()->ClientName(m_pPlayer->GetCID()));		
		DEBUG("cmds", aBuf);
		return;
	}

	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "getgroup", 8))
	{	
		if(!GameServer()->Server()->IsAuthed(m_pPlayer->GetCID()))
			return;

		int ClientID;
		if(sscanf(Msg->m_pMessage, "/getgroup %d", &ClientID) != 1)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please use: /getgroup <id>");
			return;
		}
		
		if(ClientID < 0 || ClientID >= MAX_CLIENTS || !GameServer()->m_apPlayers[ClientID])
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "There is no such player!");
			return;
		}

		char aBuf[128];
		char gname[4][12] = {"player", "police", "vip", "helper"};
		
		str_format(aBuf, sizeof(aBuf), "'%s' group is '%s'",
			GameServer()->Server()->ClientName(ClientID), gname[GameServer()->m_apPlayers[ClientID]->m_AccData.m_PlayerState]);
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
		return;
	}
	
	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "setgroup", 8))
	{
		if(!GameServer()->Server()->IsAuthed(m_pPlayer->GetCID()))
			return;

		int ClientID, Group;
		if(sscanf(Msg->m_pMessage, "/setgroup %d %d", &ClientID, &Group) != 2)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please use: /setgroup <id> <groupid>"); 
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Group ID: 0 - Removed, 1 - Police, 2 - VIP, 3 - Helper");
			return;
		}
		
		if(ClientID < 0 || ClientID >= MAX_CLIENTS || !GameServer()->m_apPlayers[ClientID])
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "There is no such player!");
			return;
		}
		
		char gname[4][12] = {"", "police", "vip", "helper"}, aBuf[64];

		if(!GameServer()->m_apPlayers[ClientID]->m_AccData.m_UserID)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "The player is not logged in account!");
			return;
		}

		switch (Group)
		{
			case 0:
			{
				if(GameServer()->m_apPlayers[ClientID]->m_AccData.m_PlayerState)
				{
					str_format(aBuf, sizeof(aBuf), "Removed group %s for player '%s'",
						gname[GameServer()->m_apPlayers[ClientID]->m_AccData.m_PlayerState], GameServer()->Server()->ClientName(ClientID));
					GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
					
					str_format(aBuf, sizeof(aBuf), "Your group removed %s!",
						gname[GameServer()->m_apPlayers[ClientID]->m_AccData.m_PlayerState]);
					GameServer()->SendChatTarget(ClientID, aBuf);
					
					GameServer()->m_apPlayers[ClientID]->m_AccData.m_PlayerState = 0;
				}
				else GameServer()->SendChatTarget(m_pPlayer->GetCID(), "This player already no in group!");
			} break;
				
			default:
			{
				if(Group > 3 || Group < 0)
				{
					GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Group ID not found!");
					return;
				}
				
				GameServer()->m_apPlayers[ClientID]->m_AccData.m_PlayerState = Group;
				
				str_format(aBuf, sizeof(aBuf), "Set group %s for player '%s'",
					gname[GameServer()->m_apPlayers[ClientID]->m_AccData.m_PlayerState], GameServer()->Server()->ClientName(ClientID));
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
				
				str_format(aBuf, sizeof(aBuf), "Your group is set to %s!",
					gname[GameServer()->m_apPlayers[ClientID]->m_AccData.m_PlayerState]);
				GameServer()->SendChatTarget(ClientID, aBuf);
			} break;
		}
		
		str_format(aBuf, sizeof(aBuf), "'%s' group changed to '%s'",
			GameServer()->Server()->ClientName(ClientID), gname[GameServer()->m_apPlayers[ClientID]->m_AccData.m_PlayerState]);		
		DEBUG("cmds", aBuf);
		return;
	}
	
	else if(!str_comp_nocase_num(Msg->m_pMessage+1, "move", 4))
	{
		if(!GameServer()->Server()->IsAuthed(m_pPlayer->GetCID()))
			return;
		
		int Steps = 1;
		char Direction[64];
		
		if(sscanf(Msg->m_pMessage, "/move %s %d", &Direction, &Steps) < 1)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Please use: /move <direction> [<steps>]");
			return;
		}
		
		int MoveX = 0, MoveY = 0;
		
		if(str_find_nocase(Direction, "left"))
		{
			MoveX = -1; MoveY = 0;
		}
		else if(str_find_nocase(Direction, "right"))
		{
			MoveX = 1; MoveY = 0;
		}
		else if(str_find_nocase(Direction, "up"))
		{
			MoveX = 0; MoveY = -1;
		}
		else if(str_find_nocase(Direction, "down"))
		{
			MoveX = 0; MoveY = 1;
		}
		
		GameServer()->MoveCharacter(m_pPlayer->GetCID(), MoveX*Steps, MoveY*Steps, false);
		
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "'%s' moved %d steps %s",
			GameServer()->Server()->ClientName(m_pPlayer->GetCID()), Steps, Direction);		
		DEBUG("cmds", aBuf);
		return;
	}

	else
	{
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Wrong command. Use /cmdlist");
		return;
	}
}

void CCmd::LastChat()
{
	m_pPlayer->m_LastChat = GameServer()->Server()->Tick();
}

#undef DEBUG
