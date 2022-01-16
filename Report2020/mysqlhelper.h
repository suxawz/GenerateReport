//mysqlhelper.h

#ifndef __MYSQL_HELPER_H__
#define __MYSQL_HELPER_H__
//#ifdef __HAS_STD_BYTE
//_HAS_STD_BYTE=0
//#endif
#include <stdlib.h>

#include <map>
#include <vector>
#include <string>
#include "nanodbc.h"

//using namespace std;

namespace mysqlhelper
{
	///***********************
//* @brief DB CFG interface
//***********************/
	struct DBConf
	{

		std::string _host;//IP
		std::string _user; //User name 
		std::string _password;//PW
		std::string _database; //DB name
		std::string _charset; //char 
		int _port;//port
		int _flag; //client connect flag

		/*****************
		* @brief constract function
		*****************/
		DBConf() :_port(0), _flag(0) {}

		/**********************************
		* @brief read CFG file
		* @param mpParam save DB CFG map
		* dbhost: IP
		* dbuser:user name
		* dbpass:PW
		* dbname:DB name
		* dbport:port
		**********************************/
		void loadFromMap(const std::map<std::string, std::string>& mpParam)
		{
			std::map<std::string, std::string> mpTmp = mpParam;

			_host = mpTmp["dbhost"];
			_user = mpTmp["dbuser"];
			_password = mpTmp["dbpass"];
			_database = mpTmp["dbname"];
			_charset = mpTmp["charset"];
			_port = atoi(mpTmp["dbport"].c_str());
			_flag = 0;

			if (mpTmp["dbport"] == "")
			{
				_port = 3306;
			}
		}
	};

	/**************************************************************
	* @brief:MySQL DB operate class
	* @feature:not thread safe,normal one thread one MysqlHelper object;
	* for insert/update better ,SQL;
	* MysqlHelper::DB_INTconstrace sql,without ”” and transfer;
	* MysqlHelper::DB_STR constract sql,use ”” and transfer;
	**************************************************************/
	class MysqlHelper {

	public:
		/**
		* @brief COnstract
		*/
		MysqlHelper();

		/**
		* @brief constract function.
		* @param: sHost: IP
		* @param sUser
		* @param sPasswd
		* @param sDatebase
		* @param port
		* @param iUnixSocket socket
		* @param iFlag
		*/
		MysqlHelper(const std::string& sHost, const std::string& sUser = "", const std::string& sPasswd = "", const std::string& sDatabase = "", const std::string& sCharSet = "", int port = 0, int iFlag = 0);

		/**
		* @brief constracter.
		* @param tcDBConf DB CFG
		*/
		MysqlHelper(const DBConf& tcDBConf);

		/**
		* @brief Deconstract .
		*/
		~MysqlHelper();

		/**
		* @brief init.
		*
		* @param sHost IP
		* @param sUser
		* @param sPasswd
		* @param sDatebase
		* @param port
		* @param iUnixSocket socket
		* @param iFlag
		* @return none
		*/
		void init(const std::string& sHost, const std::string& sUser = "", const std::string& sPasswd = "", const std::string& sDatabase = "", const std::string& sCharSet = "", int port = 0, int iFlag = 0);

		/**
		* @brief init.
		*
		* @param tcDBConf DB CFG
		*/
		void init(const DBConf& tcDBConf);

		/**
		* @brief Connect DB.
		*
		* @throws MysqlHelper_Exception
		* @return none
		*/
		bool connect();

		/**
		* @brief Deconnect DB connect.
		* @return
		*/
		void disconnect();

		/**
		* @brief get DB variable.
		* @return DB variable
		*/
		std::string getVariables(const std::string& sName);

		/**
		* @brief get DB pointer.
		*
		* @return MYSQL*
		*/
		nanodbc::connection* getMysql();

		/**
		* @brief Char transfer.
		*
		* @param sFrom
		* @param sTo
		* @return
		*/
		std::string escapeString(const std::string& sFrom);

		/**
		* @brief update or insert.
		*
		* @param sSql sql
		* @throws MysqlHelper_Exception
		* @return
		*/
		void execute(const std::string& sSql);

		//Refresh alive time from connect start
		void refreshAliveTime() { _aliveTime = clock(); }

		//return alive time
		clock_t getAliveTime() const { return clock() - _aliveTime; }

		/**
		* @brief mysql one record
		*/
		class MysqlRecord
		{
		public:
			/**
			* @brief constract.
			*
			* @param record
			*/
			MysqlRecord(const std::map<std::string, std::string>& record);

			/**
			* @brief get data,s db table colume name
			* @param s
			* @return data
			*/
			const std::string& operator[](const std::string& s);
		protected:
			const std::map<std::string, std::string>& _record;
		};

		/**
		* @brief mysql data
		*/
		class MysqlData
		{
		public:
			/**
			* @brief all data.
			*
			* @return vector<map<string,string>>&
			*/
			std::vector<std::map<std::string, std::string> >& data();

			/**
			* count of records
			*
			* @return size_t
			*/
			size_t size();

			/**
			* @brief get one of records.
			*
			* @param i
			* @return MysqlRecord type data,get data use colume name,
			*/
			MysqlRecord operator[](size_t i);

		protected:
			std::vector<std::map<std::string, std::string> > _data;
		};

		/**
		* @brief Query Record.
		*
		* @param sSql sql
		* @throws MysqlHelper_Exception
		* @return MysqlData type
		*/
		MysqlData queryRecord(const std::string& sSql);

		/**
		* @brief define type of data,
		* DB_INT:number
		* DB_STR:string
		*/
		enum FT
		{
			DB_INT,
			DB_STR,
		};

		/**
		* data records
		*/
		typedef std::map<std::string, std::pair<FT, std::string> > RECORD_DATA;

		/**
		* @brief update records.
		*
		* @param sTableName
		* @param mpColumns
		* @param sCondition where info,duch as:where A = B
		* @throws MysqlHelper_Exception
		* @return size_t modify row count
		*/
		size_t updateRecord(const std::string& sTableName, const std::map<std::string, std::pair<FT, std::string> >& mpColumns, const std::string& sCondition);

		/**
		* @brief insert.
		*
		* @param sTableName
		* @param mpColumns
		* @throws MysqlHelper_Exception
		* @return size_t
		*/
		size_t insertRecord(const std::string& sTableName, const std::map<std::string, std::pair<FT, std::string> >& mpColumns);

		/**
		* @brief replace.
		*
		* @param sTableName
		* @param mpColumns
		* @throws MysqlHelper_Exception
		* @return size_t
		*/
		size_t replaceRecord(const std::string& sTableName, const std::map<std::string, std::pair<FT, std::string> >& mpColumns);

		/**
		* @brief delete.
		*
		* @param sTableName
		* @param sCondition where,such as:where A = B
		* @throws MysqlHelper_Exception
		* @return size_t
		*/
		size_t deleteRecord(const std::string& sTableName, const std::string& sCondition = "");

		/**
		* @brief get Table query count.
		*
		* @param sTableName
		* @param sCondition where,such as:where A = B
		* @throws MysqlHelper_Exception
		* @return size_t
		*/
		size_t getRecordCount(const std::string& sTableName, const std::string& sCondition = "");

		/**
		* @brief get Sql return count number.
		*
		* @param sCondition where?,:where A = B
		* @throws MysqlHelper_Exception
		* @return
		*/
		size_t getSqlCount(const std::string& sCondition = "");

		/**
		* @brief exist records.
		*
		* @param sql sql
		* @throws MysqlHelper_Exception
		* @return
		*/
		bool existRecord(const std::string& sql);

		/**
		* @brief get max data records.
		*
		* @param sTableName
		* @param sFieldName
		* @param sCondition where,:where A = B
		* @throws MysqlHelper_Exception
		* @return count number
		*/
		int getMaxValue(const std::string& sTableName, const std::string& sFieldName, const std::string& sCondition = "");

		/**
		* @brief ??auto_increment?????ID.
		*
		* @return ID?
		*/
		long lastInsertID();

		/**
		* @brief  constract Insert-SQL.
		*
		* @param sTableName
		* @param mpColumns
		* @return string insert-SQL
		*/
		std::string buildInsertSQL(const std::string& sTableName, const std::map<std::string, std::pair<FT, std::string> >& mpColumns);

		/**
		* @brief Replace-SQL.
		*
		* @param sTableName
		* @param mpColumns
		* @return string insert-SQL
		*/
		std::string buildReplaceSQL(const std::string& sTableName, const std::map<std::string, std::pair<FT, std::string> >& mpColumns);

		/**
		* @brief Update-SQL.
		*
		* @param sTableName
		* @param mpColumns
		* @param sCondition where
		* @return string Update-SQL
		*/
		std::string buildUpdateSQL(const std::string& sTableName, const std::map<std::string, std::pair<FT, std::string> >& mpColumns, const std::string& sCondition);

		/**
		* @brief get last exect SQL.
		*
		* @return SQL
		*/
		std::string getLastSQL() { return _sLastSql; }

		/**
		* @brief get query rows number
		* @return int
		*/
		size_t getAffectedRows();
	protected:
		/**
		* @brief copy contructor
		*/
		MysqlHelper(const MysqlHelper& tcMysql);

		/**
		*
		* @brief only init no define
		*/
		MysqlHelper& operator=(const MysqlHelper& tcMysql);


	private:

		/**
		* pointer DB
		*/
		nanodbc::connection *_pstMql;

		size_t _affected_row_Number;

		/**
		* CFG DB
		*/
		DBConf _dbConf;

		/**
		* flag connection
		*/
		bool _bConnected;

		/**
		* Last exect sql
		*/
		std::string _sLastSql;

		clock_t _aliveTime;	//start time begina alive

	};
/*********************
*@brief Exception class
**********************/
struct MysqlHelper_Exception //: public TC_Exception
{
 MysqlHelper_Exception(const std::string &sBuffer):errorInfo(sBuffer){}; //: TC_Exception(sBuffer){};
 ~MysqlHelper_Exception() throw(){}; 

 std::string errorInfo;
};

}
#endif //__MYSQL_HELPER_H__