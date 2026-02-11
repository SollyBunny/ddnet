#include <engine/shared/config.h>

#include <game/server/gamecontroller.h>
#include <game/server/player.h>

void IGameController::GetPlayerStatsStr(CPlayer *pPlayer, char *pBuf, size_t Size)
{
	pBuf[0] = '\0';
	if(!pPlayer || !IsPlaying(pPlayer))
		return;

	CJsonStringWriter Writer;
	Writer.BeginObject();
	{
		Writer.WriteAttribute("server");
		Writer.WriteStrValue(g_Config.m_SvName);
		Writer.WriteAttribute("map");
		Writer.WriteStrValue(g_Config.m_SvMap);
		Writer.WriteAttribute("game_type");
		Writer.WriteStrValue(g_Config.m_SvGametype);

		Writer.WriteAttribute("player");
		Writer.BeginObject();
		Writer.WriteAttribute("id");
		Writer.WriteIntValue(pPlayer->GetCid());
		if(IsTeamPlay())
		{
			Writer.WriteAttribute("team");
			Writer.WriteStrValue(pPlayer->GetTeamStr());
		}
		if(WinType() == WIN_BY_SURVIVAL)
		{
			Writer.WriteAttribute("alive");
			Writer.WriteBoolValue(!pPlayer->m_IsDead);
		}
		Writer.WriteAttribute("name");
		Writer.WriteStrValue(Server()->ClientName(pPlayer->GetCid()));
		// currently block is the only ddrace gametype with kills
		if(!IsDDRaceGameType() || IsBlockGameType())
		{
			Writer.WriteAttribute("score");
			Writer.WriteIntValue(pPlayer->m_Score);
			Writer.WriteAttribute("kills");
			Writer.WriteIntValue(pPlayer->m_RoundStats.m_Kills);
			Writer.WriteAttribute("deaths");
			Writer.WriteIntValue(pPlayer->m_RoundStats.m_Deaths);
		}
		Writer.WriteAttribute("hooks");
		Writer.WriteIntValue(pPlayer->m_RoundStats.m_Hooks);
		Writer.WriteAttribute("hooks_missed");
		Writer.WriteIntValue(pPlayer->m_RoundStats.m_HooksMissed);
		Writer.WriteAttribute("hooks_hit_player");
		Writer.WriteIntValue(pPlayer->m_RoundStats.m_HooksHitPlayer);
		Writer.WriteAttribute("shots_fired");
		Writer.WriteIntValue(pPlayer->m_Stats.m_ShotsFired);
		Writer.WriteAttribute("shots_hit");
		Writer.WriteIntValue(pPlayer->m_Stats.m_ShotsHit);
		if(GameFlags() & GAMEFLAG_FLAGS)
		{
			Writer.WriteAttribute("flag_grabs");
			Writer.WriteIntValue(pPlayer->m_Stats.m_FlagGrabs);
			Writer.WriteAttribute("flag_captures");
			Writer.WriteIntValue(pPlayer->m_Stats.m_FlagCaptures);
			Writer.WriteAttribute("flag_returns");
			Writer.WriteIntValue(pPlayer->m_RoundStats.m_FlagReturns);
			Writer.WriteAttribute("flagger_kills");
			Writer.WriteIntValue(pPlayer->m_Stats.m_FlaggerKills);
		}
		Writer.EndObject();
	}
	Writer.EndObject();
	str_copy(pBuf, Writer.GetOutputString().c_str(), Size);
}
