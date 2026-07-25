// xcloud_node.cpp: CXcloudLibrary, the XAP-callable view over Xbox Cloud
// Gaming (Game Pass streaming) and your own consoles (remote play). Two flat
// lists, box art disk-cached via xcloud_client, and a stream launch per item.
// Companion to xcloud_client.cpp; DEF it in a scene as "XcloudLibrary".

#include "std.h"
#include "dashapp.h"
#include "node.h"
#include "runner.h"
#include "xcloud_client.h"

class CXcloudLibrary : public CNode
{
public:
	CXcloudLibrary() {}

	DECLARE_NODE(CXcloudLibrary, CNode)
	DECLARE_NODE_PROPS()
	DECLARE_NODE_FUNCTIONS()

	// ---- account / session ----
	int         IsSignedIn()      { return Xcloud_HasToken() ? 1 : 0; }
	void        StartSession()    { Xcloud_StartSession(); }
	int         IsSessionReady()  { return Xcloud_SessionReady() ? 1 : 0; }
	CStrObject* GetSessionPhase() { return new CStrObject(_T(Xcloud_SessionPhase().c_str())); }
	CStrObject* GetGamertag()     { return new CStrObject(_T(Xcloud_GetGamertag().c_str())); }

	// ---- Cloud Gaming (Game Pass) section ----
	void RefreshGames()  { Xcloud_FetchGames(); }
	int  GamesReady()    { return Xcloud_GamesReady() ? 1 : 0; }
	int  GetGameCount()  { return (int)Xcloud_GetGames().size(); }

	CStrObject* GetGameName(int i)
	{
		std::vector<XcloudGame> g = Xcloud_GetGames();
		if (i < 0 || i >= (int)g.size()) return new CStrObject;
		return new CStrObject(_T(g[i].name.c_str()));
	}

	int IsGamePlayable(int i)
	{
		std::vector<XcloudGame> g = Xcloud_GetGames();
		if (i < 0 || i >= (int)g.size()) return 0;
		return g[i].playable ? 1 : 0;
	}

	void QueueGameArt(int i)
	{
		std::vector<XcloudGame> g = Xcloud_GetGames();
		if (i < 0 || i >= (int)g.size()) return;
		Xcloud_QueueArtDownload(g[i].titleId, g[i].artUrl);
	}

	CStrObject* GetGameArtPath(int i)
	{
		std::vector<XcloudGame> g = Xcloud_GetGames();
		if (i < 0 || i >= (int)g.size()) return new CStrObject;
		return new CStrObject(_T(Xcloud_ArtCachePath(g[i].titleId).c_str()));
	}

	void LaunchGame(int i)
	{
		std::vector<XcloudGame> g = Xcloud_GetGames();
		if (i < 0 || i >= (int)g.size()) return;
		Xcloud_StartStreamSession("cloud", g[i].titleId);
	}

	// ---- Your Xbox (remote play) section ----
	void RefreshConsoles() { Xcloud_FetchConsoles(); }
	int  ConsolesReady()   { return Xcloud_ConsolesReady() ? 1 : 0; }
	int  GetConsoleCount() { return (int)Xcloud_GetConsoles().size(); }

	CStrObject* GetConsoleName(int i)
	{
		std::vector<XcloudConsole> c = Xcloud_GetConsoles();
		if (i < 0 || i >= (int)c.size()) return new CStrObject;
		return new CStrObject(_T(c[i].name.c_str()));
	}

	CStrObject* GetConsolePower(int i)
	{
		std::vector<XcloudConsole> c = Xcloud_GetConsoles();
		if (i < 0 || i >= (int)c.size()) return new CStrObject;
		return new CStrObject(_T(c[i].powerState.c_str()));
	}

	void LaunchConsole(int i)
	{
		std::vector<XcloudConsole> c = Xcloud_GetConsoles();
		if (i < 0 || i >= (int)c.size()) return;
		Xcloud_StartStreamSession("home", c[i].serverId);
	}

	// ---- stream state (shared by both sections) ----
	int         IsStreamRunning() { return Xcloud_StreamRunning() ? 1 : 0; }
	int         IsStreamReady()   { return Xcloud_StreamReady() ? 1 : 0; }
	CStrObject* GetStreamPhase()  { return new CStrObject(_T(Xcloud_StreamPhase().c_str())); }
	void        StopStream()      { Xcloud_StopStreamSession(); }
};

IMPLEMENT_NODE("XcloudLibrary", CXcloudLibrary, CNode)

START_NODE_PROPS(CXcloudLibrary, CNode)
END_NODE_PROPS()

#undef _FND_CLASS
#define _FND_CLASS CXcloudLibrary
START_NODE_FUN(CXcloudLibrary, CNode)
	NODE_FUN_IV(IsSignedIn)
	NODE_FUN_VV(StartSession)
	NODE_FUN_IV(IsSessionReady)
	NODE_FUN_SV(GetSessionPhase)
	NODE_FUN_SV(GetGamertag)
	NODE_FUN_VV(RefreshGames)
	NODE_FUN_IV(GamesReady)
	NODE_FUN_IV(GetGameCount)
	NODE_FUN_SI(GetGameName)
	NODE_FUN_II(IsGamePlayable)
	NODE_FUN_VI(QueueGameArt)
	NODE_FUN_SI(GetGameArtPath)
	NODE_FUN_VI(LaunchGame)
	NODE_FUN_VV(RefreshConsoles)
	NODE_FUN_IV(ConsolesReady)
	NODE_FUN_IV(GetConsoleCount)
	NODE_FUN_SI(GetConsoleName)
	NODE_FUN_SI(GetConsolePower)
	NODE_FUN_VI(LaunchConsole)
	NODE_FUN_IV(IsStreamRunning)
	NODE_FUN_IV(IsStreamReady)
	NODE_FUN_SV(GetStreamPhase)
	NODE_FUN_VV(StopStream)
END_NODE_FUN()
