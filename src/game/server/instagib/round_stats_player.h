#ifndef GAME_SERVER_INSTAGIB_ROUND_STATS_PLAYER_H
#define GAME_SERVER_INSTAGIB_ROUND_STATS_PLAYER_H

// See also `CSqlStatsPlayer` for stats that get stored in the database
//
// The round stats get reset on round start and are tracked at all times.
// They have overlapping values with the sql stats. But sql stats are
// tracked conditionally to avoid farming. Round stats are always tracked.
class CRoundStatsPlayer
{
public:
	int m_Kills;
	int m_Deaths;

	// Called on round end/start and player join
	void Reset()
	{
		m_Kills = 0;
		m_Deaths = 0;
	}

	CRoundStatsPlayer()
	{
		Reset();
	}
};

#endif
