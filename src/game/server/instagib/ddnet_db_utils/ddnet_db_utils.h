// This file can be included several times.
//  ^
//  |
// this is a hack for the lint so we can use
// pragma once instead of using a include guard with
// path name, because the path name is chosen by the
// user of this library
#pragma once

#include <engine/server/databases/connection.h>

namespace ddnet_db_utils
{
	enum class ESqlBackend
	{
		SQLITE3,
		MYSQL
	};

	// hack to avoid editing connection.h in ddnet code
	ESqlBackend DetectBackend(IDbConnection *pSqlServer);
	bool HasColumn(IDbConnection *pSqlServer, const char *pTableName, const char *pColumnName, char *pError, int ErrorSize);

	/**
	 * Adds `pColumnName` to database table `pTableName`
	 * with default value being `Default`.
	 * This is safe to run multiple times.
	 * It only adds the column if it is not there already.
	 * Supported backends are mariadb and sqlite3.
	 *
	 * @param pSqlServer
	 * @param pTableName name of the target database table that will be written to
	 * @param pColumnName name of the new database column that will be created
	 * @param Default the default integer value the database will use for new entries in that column
	 * @param pError in case of error this buffer will be written to
	 * @param ErrorSize size of the error buffer in bytes
	 */
	bool AddIntColumn(IDbConnection *pSqlServer, const char *pTableName, const char *pColumnName, int Default, char *pError, int ErrorSize);

	/**
	 * Adds `pColumnName` to database table `pTableName`
	 * with default value being `pDefault`.
	 * This is safe to run multiple times.
	 * It only adds the column if it is not there already.
	 * Supported backends are mariadb and sqlite3.
	 *
	 * @param pSqlServer
	 * @param pTableName name of the target database table that will be written to
	 * @param pColumnName name of the new database column that will be created
	 * @param Length maximum length of the char type in the database
	 * @param Collate will add the binary collate property for utf8 strings
	 * @param NotNull if set to true it will at the `NOT NULL` property
	 * @param pDefault the default string value the database will use for new entries in that column
	 * @param pError in case of error this buffer will be written to
	 * @param ErrorSize size of the error buffer in bytes
	 */
	bool AddStrColumn(IDbConnection *pSqlServer, const char *pTableName, const char *pColumnName, int Length, bool Collate, bool NotNull, const char *pDefault, char *pError, int ErrorSize);
}
