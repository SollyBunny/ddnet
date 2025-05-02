#include <gtest/gtest.h>

#include <game/client/race.h>

TEST(RaceHelper, TimeFromSecondsStr)
{
	// x.xxx second(s)
	EXPECT_EQ(CRaceHelper::TimeFromSecondsStr("invalid"), -1);
	EXPECT_EQ(CRaceHelper::TimeFromSecondsStr("-1"), 0);
	EXPECT_EQ(CRaceHelper::TimeFromSecondsStr(" 10"), 10 * 1000);
	EXPECT_EQ(CRaceHelper::TimeFromSecondsStr("1.234"), 1234);
	EXPECT_EQ(CRaceHelper::TimeFromSecondsStr("1.2349"), 1235);
	EXPECT_EQ(CRaceHelper::TimeFromSecondsStr("1.2341"), 1234);
	EXPECT_EQ(CRaceHelper::TimeFromSecondsStr("999999.2341"), 10 * 999999 + 234);
}

TEST(RaceHelper, TimeFromStr)
{
	// x days(s) x hour(s) x minute(s) x.xxx second(s)
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

TEST(RaceHelper, TimeFromFinishMessage)
{
	// xxx finished in: x days(s) x hour(s) x minute(s) x.xxx second(s)
	char aName[256];
	EXPECT_EQ(CRaceHelper::TimeFromFinishMessage("bun bun finished in: invalid", aName, sizeof(aName)), -1);
	EXPECT_EQ(aName, "bun bun");
	EXPECT_EQ(CRaceHelper::TimeFromFinishMessage("roby finished in: 40 second(s)", aName, sizeof(aName)), 40 * 1000);
	EXPECT_EQ(aName, "roby");
	EXPECT_EQ(CRaceHelper::TimeFromFinishMessage("finished in finished in: 2 minutes(s) 1 second(s)", aName, sizeof(aName)), 2 * 60 * 1000 + 1000);
	EXPECT_EQ(aName, "finished in");
	EXPECT_EQ(CRaceHelper::TimeFromFinishMessage("4.321 second(s) in finished in: 1.234 second(s)", aName, sizeof(aName)), 1234);
	EXPECT_EQ(aName, "finished in");
}
