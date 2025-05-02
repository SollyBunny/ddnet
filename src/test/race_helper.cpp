#include <gtest/gtest.h>

#include <game/client/race.h>

TEST(RaceHelper, TimeFromStr)
{
	EXPECT_EQ(CRaceHelper::TimeFromStr("invalid"), -1);
	EXPECT_EQ(CRaceHelper::TimeFromStr("1 second(s)"), 1000);
	EXPECT_EQ(CRaceHelper::TimeFromStr("1.432 second(s)"), 1432);
	EXPECT_EQ(CRaceHelper::TimeFromStr("5 minute(s)"), -1); // No seconds is invalid
	EXPECT_EQ(CRaceHelper::TimeFromStr("5 minute(s) 1.432 second(s)"), 5 * 60 * 1000 + 1432);
	EXPECT_EQ(CRaceHelper::TimeFromStr("5 minute(s) -1 second(s)"), 5 * 60 * 1000); // Min 0
	EXPECT_EQ(CRaceHelper::TimeFromStr("6 hour(s) 5 minute(s) 1.432 second(s)"), 6 * 60 * 60 * 1000 + 5 * 60 * 1000 + 1432);
	EXPECT_EQ(CRaceHelper::TimeFromStr("1 day(s) 6 hour(s) 5 minute(s) 1.432 second(s)"), 1 * 24 * 60 * 60 * 1000 + 6 * 60 * 60 * 1000 + 5 * 60 * 1000 + 1432);
	EXPECT_EQ(CRaceHelper::TimeFromStr("1 day(s) invalid hour(s) 5 minute(s) 1.432 second(s)"), 1 * 24 * 60 * 60 * 1000 + 5 * 60 * 1000 + 1432);
}
