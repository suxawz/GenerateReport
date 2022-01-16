//mysqlhelper.cpp

#include "mysqlHelper.h"

#include <string>
#include <sstream>

//using namespace std;

namespace mysqlhelper{

MysqlHelper::MysqlHelper():_bConnected(false)
{

 //_pstMql = mysql_init(NULL);
}

MysqlHelper::MysqlHelper(const std::string& sHost, const std::string& sUser, const std::string& sPasswd, const std::string& sDatabase, const std::string &sCharSet, int port, int iFlag)
:_bConnected(false)
{
 init(sHost, sUser, sPasswd, sDatabase, sCharSet, port, iFlag);

 //_pstMql = mysql_init(NULL);
}

MysqlHelper::MysqlHelper(const DBConf& tcDBConf)
:_bConnected(false)
{
 _dbConf = tcDBConf;

 //_pstMql = mysql_init(NULL); 
}

MysqlHelper::~MysqlHelper()
{
 if (_pstMql != NULL)
 {

 _pstMql = NULL;
 }
}

void MysqlHelper::init(const std::string& sHost, const std::string& sUser, const std::string& sPasswd, const std::string& sDatabase, const std::string &sCharSet, int port, int iFlag)
{
 _dbConf._host = sHost;
 _dbConf._user = sUser;
 _dbConf._password = sPasswd;
 _dbConf._database = sDatabase;
 _dbConf._charset = sCharSet;
 _dbConf._port = port;
 _dbConf._flag = iFlag;
}

void MysqlHelper::init(const DBConf& tcDBConf)
{
 _dbConf = tcDBConf;
}

bool MysqlHelper::connect()
{
	try
	{ 
		
		disconnect();
		_pstMql->connect(_dbConf._database.c_str(), _dbConf._user.c_str(), _dbConf._password.c_str());
		_bConnected = _pstMql->connected();
	}
	catch (const nanodbc::database_error e)
	{
		throw MysqlHelper_Exception("[MysqlHelper::connect]:" + std::string(e.what()));
		_bConnected = false;
	}
 return _bConnected;
}

void MysqlHelper::disconnect()
{
	try
	{
		_pstMql->disconnect();
	}
	catch (const nanodbc::database_error e)
	{
		throw MysqlHelper_Exception("[MysqlHelper::disconnect]:" + std::string(e.what()));
	}
}

std::string MysqlHelper::escapeString(const std::string& sFrom)
{
 if(!_bConnected)
 {
 connect();
 }

 std::string sTo;
 std::string::size_type iLen = sFrom.length() * 2 + 1;
 char *pTo = (char *)malloc(iLen);

 memset(pTo, 0x00, iLen);

 sTo = sFrom;

 free(pTo);

 return sTo;
}

nanodbc::connection* MysqlHelper::getMysql(void)
{
 return _pstMql;
}

std::string MysqlHelper::buildInsertSQL(const std::string &sTableName, const RECORD_DATA &mpColumns)
{
 std::ostringstream sColumnNames;
 std::ostringstream sColumnValues;

 std::map<std::string, std::pair<FT, std::string> >::const_iterator itEnd = mpColumns.end();

 for(std::map<std::string, std::pair<FT, std::string> >::const_iterator it = mpColumns.begin(); it != itEnd; ++it)
 {
 if (it == mpColumns.begin())
 {
 sColumnNames << "`" << it->first << "`";
 if(it->second.first == DB_INT)
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
 if(it->second.first == DB_INT)
 {
 sColumnValues << "," + it->second.second;
 }
 else
 {
 sColumnValues << ",'" + escapeString(it->second.second) << "'";
 }
 }
 }

 std::ostringstream os;
 os << "insert into " << sTableName << " (" << sColumnNames.str() << ") values (" << sColumnValues.str() << ")";
 return os.str();
}

std::string MysqlHelper::buildReplaceSQL(const std::string &sTableName, const RECORD_DATA &mpColumns)
{
 std::ostringstream sColumnNames;
 std::ostringstream sColumnValues;

 std::map<std::string, std::pair<FT, std::string> >::const_iterator itEnd = mpColumns.end();
 for(std::map<std::string, std::pair<FT, std::string> >::const_iterator it = mpColumns.begin(); it != itEnd; ++it)
 {
 if (it == mpColumns.begin())
 {
 sColumnNames << "`" << it->first << "`";
 if(it->second.first == DB_INT)
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
 if(it->second.first == DB_INT)
 {
 sColumnValues << "," + it->second.second;
 }
 else
 {
 sColumnValues << ",'" << escapeString(it->second.second) << "'";
 }
 }
 }

 std::ostringstream os;
 os << "replace into " << sTableName << " (" << sColumnNames.str() << ") values (" << sColumnValues.str() << ")";
 return os.str();
}

std::string MysqlHelper::buildUpdateSQL(const std::string &sTableName,const RECORD_DATA &mpColumns, const std::string &sWhereFilter)
{
 std::ostringstream sColumnNameValueSet;

 std::map<std::string, std::pair<FT, std::string> >::const_iterator itEnd = mpColumns.end();

 for(std::map<std::string, std::pair<FT, std::string> >::const_iterator it = mpColumns.begin(); it != itEnd; ++it)
 {
 if (it == mpColumns.begin())
 {
 sColumnNameValueSet << "`" << it->first << "`";
 }
 else
 {
 sColumnNameValueSet << ",`" << it->first << "`";
 }

 if(it->second.first == DB_INT)
 {
 sColumnNameValueSet << "= " << it->second.second;
 }
 else
 {
 sColumnNameValueSet << "= '" << escapeString(it->second.second) << "'";
 }
 }

 std::ostringstream os;
 os << "update " << sTableName << " set " << sColumnNameValueSet.str() << " " << sWhereFilter;

 return os.str();
}

std::string MysqlHelper::getVariables(const std::string &sName)
{
 std::string sql = "SHOW VARIABLES LIKE '" + sName + "'";

 MysqlData data = queryRecord(sql);
 if(data.size() == 0)
 {
 return "";
 }

 if(sName == data[0]["Variable_name"])
 {
 return data[0]["Value"];
 }

 return "";
}

void MysqlHelper::execute(const std::string& sSql)
{
	/**
	????, ?????
	*/
	if (!_bConnected)
	{
		connect();
	}
	try
	{
		_sLastSql = sSql;

		nanodbc::result results = nanodbc::execute(*_pstMql, sSql);
		_affected_row_Number = results.affected_rows();
	}

	catch (nanodbc::database_error e)
	{
		throw MysqlHelper_Exception("[MysqlHelper::execute]: mysql_query: [ " + sSql + " ] :" + std::string(e.what()));
	}


}


MysqlHelper::MysqlData MysqlHelper::queryRecord(const std::string& sSql)
{
	MysqlData data;

	/**
	????, ?????
	*/
	if (!_bConnected)
	{
		connect();
	}

	try {

		_sLastSql = sSql;
		nanodbc::result results = nanodbc::execute(*_pstMql, sSql);


		std::vector<std::string> vtFields;
		for (int i = 0; i < results.columns(); i++)
		{
			vtFields.push_back(results.column_name(i));
		}

		std::map<std::string, std::string> mpRow;

		while (results.next())
		{
			mpRow.clear();
			//unsigned long* lengths = mysql_fetch_lengths(pstRes);
			for (size_t i = 0; i < vtFields.size(); i++)
			{
				std::string Temp;
				switch (results.column_c_datatype(vtFields[i]))
				{
				case 1:
					Temp = results.get<int>(vtFields[i]);
					break;
				case 2:
					Temp = results.get<int>(vtFields[i]);
					break;
				}
				Temp = results.get<int>(vtFields[i]);
				if (!Temp.empty())
				{
					mpRow[vtFields[i]] = std::string(Temp);
				}
				else
				{
					mpRow[vtFields[i]] = "";
				}
			}

			data.data().push_back(mpRow);
		}
	}
	catch (nanodbc::database_error e)
	{
		throw MysqlHelper_Exception("[MysqlHelper::queryRecord]: mysql_query: [ " + sSql + " ] :" + std::string(e.what()));
	}

	return data;
}

size_t MysqlHelper::updateRecord(const std::string &sTableName, const RECORD_DATA &mpColumns, const std::string &sCondition)
{
 std::string sSql = buildUpdateSQL(sTableName, mpColumns, sCondition);
 execute(sSql);

 return _affected_row_Number;
}

size_t MysqlHelper::insertRecord(const std::string &sTableName, const RECORD_DATA &mpColumns)
{
 std::string sSql = buildInsertSQL(sTableName, mpColumns);
 execute(sSql);

 return _affected_row_Number;
}

size_t MysqlHelper::replaceRecord(const std::string &sTableName, const RECORD_DATA &mpColumns)
{
 std::string sSql = buildReplaceSQL(sTableName, mpColumns);
 execute(sSql);

 return _affected_row_Number;
}

size_t MysqlHelper::deleteRecord(const std::string &sTableName, const std::string &sCondition)
{
 std::ostringstream sSql;
 sSql << "delete from " << sTableName << " " << sCondition;

 execute(sSql.str());

 return _affected_row_Number;
}

size_t MysqlHelper::getRecordCount(const std::string& sTableName, const std::string &sCondition)
{
 std::ostringstream sSql;
 sSql << "select count(*) as num from " << sTableName << " " << sCondition;

 MysqlData data = queryRecord(sSql.str());

 long n = atol(data[0]["num"].c_str());

 return n;

}

size_t MysqlHelper::getSqlCount(const std::string &sCondition)
{
 std::ostringstream sSql;
 sSql << "select count(*) as num " << sCondition;

 MysqlData data = queryRecord(sSql.str());

 long n = atol(data[0]["num"].c_str());

 return n;
}

int MysqlHelper::getMaxValue(const std::string& sTableName, const std::string& sFieldName,const std::string &sCondition)
{
 std::ostringstream sSql;
 sSql << "select " << sFieldName << " as f from " << sTableName << " " << sCondition << " order by f desc limit 1";

 MysqlData data = queryRecord(sSql.str());

 int n = 0;

 if(data.size() == 0)
 {
 n = 0;
 }
 else
 {
 n = atol(data[0]["f"].c_str());
 }

 return n;
}

bool MysqlHelper::existRecord(const std::string& sql)
{
 return queryRecord(sql).size() > 0;
}

long MysqlHelper::lastInsertID()
{
 return 0;
}

size_t MysqlHelper::getAffectedRows()
{
 return _affected_row_Number;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
MysqlHelper::MysqlRecord::MysqlRecord(const std::map<std::string, std::string> &record):_record(record){}

const std::string& MysqlHelper::MysqlRecord::operator[](const std::string &s)
{
 std::map<std::string, std::string>::const_iterator it = _record.find(s);
 if(it == _record.end())
 {
 throw MysqlHelper_Exception("field '" + s + "' not exists.");
 }
 return it->second;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<std::map<std::string, std::string> >& MysqlHelper::MysqlData::data()
{
 return _data;
}

size_t MysqlHelper::MysqlData::size()
{
 return _data.size();
}

MysqlHelper::MysqlRecord MysqlHelper::MysqlData::operator[](size_t i)
{
 return MysqlRecord(_data[i]);
}

}//end namespace 