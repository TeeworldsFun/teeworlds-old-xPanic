/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */
#ifndef DDRACE_H
#define DDRACE_H
#include <game/server/gamecontroller.h>

#include <vector>
#include <map>

class CGameControllerDDRace: public IGameController
{
public:

	CGameControllerDDRace(class CGameContext *pGameServer);
	~CGameControllerDDRace();

	std::map<int, std::vector<vec2> > m_TeleOuts;
	std::map<int, std::vector<vec2> > m_TeleCheckOuts;

	void InitTeleporter();
	virtual void Tick();
};
#endif
