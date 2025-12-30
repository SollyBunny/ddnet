#include "ddnet_db_utils.h"

#include <base/log.h>
#include <base/str.h>
#include <base/system.h>

#include <engine/server/databases/connection.h>

namespace ddnet_db_utils
{

	// hack to avoid editing connection.h in ddnet code
	ESqlBackend DetectBackend(IDbConnection *pSqlServer)
	{
		return str_comp(pSqlServer->BinaryCollate(), "BINARY") == 0 ? ESqlBackend::SQLITE3 : ESqlBackend::MYSQL;
	}

	static const char *BackendName(IDbConnection *pSqlServer)
	{
		ESqlBackend Backend = DetectBackend(pSqlServer);
		switch(Backend)
		{
		case ESqlBackend::MYSQL:
			return "mysql";
		case ESqlBackend::SQLITE3:
			return "sqlite3";
		}
		return "(unknown db backend)";
	}

	static bool HasColumnSqlite3(IDbConnection *pSqlServer, const char *pTableName, const char *pColumnName, char *pError, int ErrorSize)
	{
		char aBuf[4096];
		str_copy(
			aBuf,
			"SELECT COUNT() FROM pragma_table_info(?) WHERE name = ?;");
		if(!pSqlServer->PrepareStatement(aBuf, pError, ErrorSize))
		{
			log_error("ddnet_db_utils", "prepare failed query: %s", aBuf);
			return false;
		}
		pSqlServer->BindString(1, pTableName);
		pSqlServer->BindString(2, pColumnName);
		pSqlServer->Print();

		bool End;
		if(!pSqlServer->Step(&End, pError, ErrorSize))
		{
			log_error("ddnet_db_utils", "step failed query: %s", aBuf);
			return false;
		}

		if(End)
		{
			// we expect 0 or 1 but never nothing
			log_error("ddnet_db_utils", "something went wrong failed query: %s", aBuf);
			return false;
		}
		if(pSqlServer->GetInt(1) == 0)
		{
			return false;
		}
		return true;
	}

	static bool HasColumnMysql(IDbConnection *pSqlServer, const char *pTableName, const char *pColumnName, char *pError, int ErrorSize)
	{
		char aBuf[4096];
		str_format(
			aBuf,
			sizeof(aBuf),
			"show columns from %s where Field = ?;",
			pTableName);
		if(!pSqlServer->PrepareStatement(aBuf, pError, ErrorSize))
		{
			log_error("ddnet_db_utils", "prepare failed query: %s", aBuf);
			return false;
		}
		pSqlServer->BindString(1, pColumnName);
		pSqlServer->Print();

		bool End;
		if(!pSqlServer->Step(&End, pError, ErrorSize))
		{
			log_error("ddnet_db_utils", "step failed query: %s", aBuf);
			return false;
		}
		if(!End)
			return true; // column found
		return false; // column not found
	}

	bool HasColumn(IDbConnection *pSqlServer, const char *pTableName, const char *pColumnName, char *pError, int ErrorSize)
	{
		ESqlBackend Backend = DetectBackend(pSqlServer);
		switch(Backend)
		{
		case ESqlBackend::MYSQL:
			return HasColumnMysql(pSqlServer, pTableName, pColumnName, pError, ErrorSize);
		case ESqlBackend::SQLITE3:
			return HasColumnSqlite3(pSqlServer, pTableName, pColumnName, pError, ErrorSize);
		}
		dbg_assert_failed("Unsupported database backend: %s", BackendName(pSqlServer));
		return false;
	}

	bool AddIntColumn(IDbConnection *pSqlServer, const char *pTableName, const char *pColumnName, int Default, char *pError, int ErrorSize)
	{
		if(HasColumn(pSqlServer, pTableName, pColumnName, pError, ErrorSize))
			return true;

		log_info("ddnet_db_utils", "adding missing integer %s column '%s' to '%s'", BackendName(pSqlServer), pColumnName, pTableName);

		char aBuf[4096];
		str_format(
			aBuf,
			sizeof(aBuf),
			"ALTER TABLE %s ADD COLUMN %s INTEGER DEFAULT %d;", pTableName, pColumnName, Default);

		if(!pSqlServer->PrepareStatement(aBuf, pError, ErrorSize))
		{
			return false;
		}
		pSqlServer->Print();
		int NumInserted;
		return pSqlServer->ExecuteUpdate(&NumInserted, pError, ErrorSize);
	}

	bool AddStrColumn(IDbConnection *pSqlServer, const char *pTableName, const char *pColumnName, int Length, bool Collate, bool NotNull, const char *pDefault, char *pError, int ErrorSize)
	{
		if(HasColumn(pSqlServer, pTableName, pColumnName, pError, ErrorSize))
			return true;

		log_info("ddnet_db_utils", "adding missing string %s column '%s' to '%s'", BackendName(pSqlServer), pColumnName, pTableName);

		char aProperties[512] = "";
		if(Collate)
		{
			str_append(aProperties, " COLLATE ");
			str_append(aProperties, pSqlServer->BinaryCollate());
			str_append(aProperties, " ");
		}
		if(NotNull)
		{
			str_append(aProperties, " NOT NULL ");
		}

		char aBuf[4096];
		str_format(
			aBuf,
			sizeof(aBuf),
			"ALTER TABLE %s ADD COLUMN %s VARCHAR(%d) %sDEFAULT '%s';", pTableName, pColumnName, Length, aProperties, pDefault);

		if(!pSqlServer->PrepareStatement(aBuf, pError, ErrorSize))
		{
			return false;
		}
		pSqlServer->Print();
		int NumInserted;
		return pSqlServer->ExecuteUpdate(&NumInserted, pError, ErrorSize);
	}
}
