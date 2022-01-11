#include "connection.h"
#include "public.h"
#include <string.h>
#include <sstream>
using namespace std;


	Connection::Connection()
	{
		OCI_Initialize(NULL, NULL, OCI_ENV_DEFAULT);	//init database connection
	}

	Connection::~Connection()
	{
		if (_conn != nullptr)
			OCI_ConnectionFree(_cn);	//relase database connection
	}

	bool Connection::connect(std::string ip, unsigned short port, std::string user, std::string password, std::string dbname)
	{
		//connect to DB
		//"172.17.216.38:1521/rmbtbssvc"
		stringstream temp;
		temp << port;
		string strport;
		temp>>strport;
		std::string ConnStr = ip + ':' + strport + '/' + dbname;
		_cn = OCI_ConnectionCreate((otext*)ConnStr.c_str(), (otext*)user.c_str(), (otext*)password.c_str() , OCI_SESSION_DEFAULT);
		_conn = OCI_StatementCreate(_cn);
		return _conn != nullptr;
	}

	bool Connection::update(std::string sql)
	{
		if (OCI_ExecuteStmt(_conn, sql.c_str()))	//database update,insert,delete
		{
			LOG("¸update error" + sql);
			return false;
		}
		return true;
	}

	OCI_Resultset* Connection::querey(std::string sql)
	{
		if (OCI_ExecuteStmt(_conn, sql.c_str()))	//database query,select
		{
			LOG("query error" + sql);
			return nullptr;
		}
		return rs = OCI_GetResultset(_conn);;
	}

	std::string Connection::escapeString(const std::string& sFrom)
	{
		std::string sTo;
		std::string::size_type iLen = sFrom.length() * 2 + 1;
		char* pTo = (char*)malloc(iLen);

		memset(pTo, 0x00, iLen);

		//mysql_real_escape_string(_conn, pTo, sFrom.c_str(), sFrom.length());


		sTo = sFrom;

		free(pTo);

		return sTo;
	}
	OCI_Statement* Connection::getMysql(void)
	{
		return _conn;
	}

	std::string Connection::buildInsertSQL(const std::string& sTableName, const RECORD_DATA& mpColumns)
	{
		std::ostringstream sColumnNames;
		ostringstream sColumnValues;

		map<std::string, pair<FT, std::string> >::const_iterator itEnd = mpColumns.end();

		for (std::map<std::string, pair<FT, std::string> >::const_iterator it = mpColumns.begin(); it != itEnd; ++it)
		{
			if (it == mpColumns.begin())
			{
				sColumnNames << "`" << it->first << "`";
				if (it->second.first == DB_INT)
				{
					sColumnValues << it->second.second;
				}
				else
				{
					sColumnValues << "'" << escapeString(it->second.second) << "'";
				}
			}
			else
			{
				sColumnNames << ",`" << it->first << "`";
				if (it->second.first == DB_INT)
				{
					sColumnValues << "," + it->second.second;
				}
				else
				{
					sColumnValues << ",'" + escapeString(it->second.second) << "'";
				}
			}
		}

		ostringstream os;
		os << "insert into " << sTableName << " (" << sColumnNames.str() << ") values (" << sColumnValues.str() << ")";
		return os.str();
	}

	std::string Connection::buildReplaceSQL(const std::string& sTableName, const RECORD_DATA& mpColumns)
	{
		ostringstream sColumnNames;
		ostringstream sColumnValues;

		map<std::string, pair<FT, std::string> >::const_iterator itEnd = mpColumns.end();
		for (map<std::string, pair<FT, std::string> >::const_iterator it = mpColumns.begin(); it != itEnd; ++it)
		{
			if (it == mpColumns.begin())
			{
				sColumnNames << "`" << it->first << "`";
				if (it->second.first == DB_INT)
				{
					sColumnValues << it->second.second;
				}
				else
				{
					sColumnValues << "'" << escapeString(it->second.second) << "'";
				}
			}
			else
			{
				sColumnNames << ",`" << it->first << "`";
				if (it->second.first == DB_INT)
				{
					sColumnValues << "," + it->second.second;
				}
				else
				{
					sColumnValues << ",'" << escapeString(it->second.second) << "'";
				}
			}
		}

		ostringstream os;
		os << "replace into " << sTableName << " (" << sColumnNames.str() << ") values (" << sColumnValues.str() << ")";
		return os.str();
	}

	std::string Connection::buildUpdateSQL(const std::string& sTableName, const RECORD_DATA& mpColumns, const std::string& sWhereFilter)
	{
		ostringstream sColumnNameValueSet;

		std::map<std::string, pair<FT, std::string> >::const_iterator itEnd = mpColumns.end();

		for (map<std::string, pair<FT, std::string> >::const_iterator it = mpColumns.begin(); it != itEnd; ++it)
		{
			if (it == mpColumns.begin())
			{
				sColumnNameValueSet << "`" << it->first << "`";
			}
			else
			{
				sColumnNameValueSet << ",`" << it->first << "`";
			}

			if (it->second.first == DB_INT)
			{
				sColumnNameValueSet << "= " << it->second.second;
			}
			else
			{
				sColumnNameValueSet << "= '" << escapeString(it->second.second) << "'";
			}
		}

		ostringstream os;
		os << "update " << sTableName << " set " << sColumnNameValueSet.str() << " " << sWhereFilter;

		return os.str();
	}

	std::string Connection::getVariables(const std::string& sName)
	{
		std::string sql = "SHOW VARIABLES LIKE '" + sName + "'";

		MysqlData data = queryRecord(sql);
		if (data.size() == 0)
		{
			return "";
		}

		if (sName == data[0]["Variable_name"])
		{
			return data[0]["Value"];
		}

		return "";
	}

	void Connection::execute(const std::string& sSql)
	{
		///**
		//????, ?????
		//*/
		//if(!_bConnected)
		//{
		//connect();
		//}

		_sLastSql = sSql;

		int iRet = OCI_ExecuteStmt(_conn, sSql.data());
		if (iRet != 0)
		{
			///**
			//??????
			//*/
			//int iErrno = mysql_errno(_conn);
			//if (iErrno == 2013 || iErrno == 2006)
			//{
			//connect();
			//iRet = mysql_real_query(_conn, sSql.c_str(), sSql.length());
			//}
		}

		if (iRet != 0)
		{
			throw Connection_Exception("[Connection::execute]: mysql_query: [ " + sSql + " ] :");
		}
	}

	Connection::MysqlData Connection::queryRecord(const std::string& sSql)
	{
		MysqlData data;

		///**
		//????, ?????
		//*/
		//if(!_bConnected)
		//{
		//connect();
		//}

		_sLastSql = sSql;

		int iRet = OCI_ExecuteStmt(_conn, sSql.data());
		if (iRet != 0)
		{
			///**
			//??????
			//*/
			//int iErrno = mysql_errno(_conn);
			//if (iErrno == 2013 || iErrno == 2006)
			//{
			//connect();
			//iRet = mysql_real_query(_conn, sSql.c_str(), sSql.length());
			//}
		}

		if (iRet != 0)
		{
			throw Connection_Exception("[Connection::execute]: mysql_query: [ " + sSql + " ] :");
		}

		OCI_Resultset* pstRes = OCI_GetResultset(_conn);

		if (pstRes == NULL)
		{
			throw Connection_Exception("[Connection::queryRecord]: mysql_store_result: " + sSql + " : ");


			std::vector<std::string> vtFields;
			OCI_Column* field;
			int Column_Index;
			while ((field = OCI_GetColumn(pstRes, Column_Index)))
			{
				vtFields.push_back(OCI_ColumnGetName(field));
			}

			map<std::string, std::string> mpRow;
			//const auto stRow;
			int index = 0;

			while (OCI_FetchNext(pstRes))
			{
				index++;
				mpRow.clear();
				const auto coll = OCI_GetColl(pstRes, 1);
				const auto iter = OCI_IterCreate(coll);
				auto elem = OCI_IterGetNext(iter);
				int itemIndex = 0;
				while (elem)
				{
					itemIndex++;

					const auto obj = OCI_ElemGetObject(elem);
					const auto data = OCI_ObjectGetString(obj, OTEXT("name"));
					if (data)
					{
						mpRow[vtFields[itemIndex]] = std::string(data, string(data).size());
					}
					else
					{
						mpRow[vtFields[itemIndex]] = "";
					}
					elem = OCI_IterGetNext(iter);
				}
				OCI_IterFree(iter);

				data.data().push_back(mpRow);
			}

			OCI_StatementFree(_conn);

			return data;
		}
	}
	size_t Connection::updateRecord(const std::string& sTableName, const RECORD_DATA& mpColumns, const std::string& sCondition)
	{
		std::string sSql = buildUpdateSQL(sTableName, mpColumns, sCondition);
		execute(sSql);

		return OCI_GetAffectedRows(_conn);
	}

	size_t Connection::insertRecord(const std::string& sTableName, const RECORD_DATA& mpColumns)
	{
		std::string sSql = buildInsertSQL(sTableName, mpColumns);
		execute(sSql);

		return OCI_GetAffectedRows(_conn);
	}

	size_t Connection::replaceRecord(const std::string& sTableName, const RECORD_DATA& mpColumns)
	{
		std::string sSql = buildReplaceSQL(sTableName, mpColumns);
		execute(sSql);

		return OCI_GetAffectedRows(_conn);
	}

	size_t Connection::deleteRecord(const std::string& sTableName, const std::string& sCondition)
	{
		ostringstream sSql;
		sSql << "delete from " << sTableName << " " << sCondition;

		execute(sSql.str());

		return OCI_GetAffectedRows(_conn);
	}

	size_t Connection::getRecordCount(const std::string& sTableName, const std::string& sCondition)
	{
		ostringstream sSql;
		sSql << "select count(*) as num from " << sTableName << " " << sCondition;

		MysqlData data = queryRecord(sSql.str());

		long n = atol(data[0]["num"].c_str());

		return n;

	}

	size_t Connection::getSqlCount(const std::string& sCondition)
	{
		ostringstream sSql;
		sSql << "select count(*) as num " << sCondition;

		MysqlData data = queryRecord(sSql.str());

		long n = atol(data[0]["num"].c_str());

		return n;
	}

	int Connection::getMaxValue(const std::string& sTableName, const std::string& sFieldName, const std::string& sCondition)
	{
		ostringstream sSql;
		sSql << "select " << sFieldName << " as f from " << sTableName << " " << sCondition << " order by f desc limit 1";

		MysqlData data = queryRecord(sSql.str());

		int n = 0;

		if (data.size() == 0)
		{
			n = 0;
		}
		else
		{
			n = atol(data[0]["f"].c_str());
		}

		return n;
	}

	bool Connection::existRecord(const std::string& sql)
	{
		return queryRecord(sql).size() > 0;
	}

	long Connection::lastInsertID()
	{
		return 0;
	}

	size_t Connection::getAffectedRows()
	{
		return OCI_GetAffectedRows(_conn);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	Connection::MysqlRecord::MysqlRecord(const map<string, string>& record) :_record(record) {}

	const std::string& Connection::MysqlRecord::operator[](const std::string& s)
	{
		std::map<std::string, std::string>::const_iterator it = _record.find(s);
		if (it == _record.end())
		{
			throw Connection_Exception("field '" + s + "' not exists.");
		}
		return it->second;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::vector<std::map<std::string, std::string> >& Connection::MysqlData::data()
	{
		return _data;
	}

	size_t Connection::MysqlData::size()
	{
		return _data.size();
	}

	Connection::MysqlRecord Connection::MysqlData::operator[](size_t i)
	{
		return MysqlRecord(_data[i]);
	}

