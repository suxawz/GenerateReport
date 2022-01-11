#pragma once
/*
Mysql database operation
*/

#include <stdlib.h>
#include "ocilib.h"
#include <string>
#include <ctime>
#include <map>
#include <vector>

using namespace std;

	/*********************
	*@brief Exception class
	**********************/
	struct Connection_Exception //: public TC_Exception
	{
		Connection_Exception(const std::string& sBuffer) :errorInfo(sBuffer) {}; //: TC_Exception(sBuffer){};
		~Connection_Exception() throw() {};
		std::string errorInfo;
	};

	class Connection
	{
	public:
		//Init database connection
		Connection();

		//relase database connection
		~Connection();

		//connect database
		bool connect(std::string ip, unsigned short port, std::string user, std::string password, std::string dbname);

		//  insert£¬update£¬delete
		bool update(std::string sql);

		//select
		OCI_Resultset* querey(std::string sql);

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
		OCI_Statement* getMysql();

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
			* @return vector<map<std::string,std::string>>&
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
		* DB_STR:std::string
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
		* @return std::string insert-SQL
		*/
		std::string buildInsertSQL(const std::string& sTableName, const std::map<std::string, std::pair<FT, std::string> >& mpColumns);

		/**
		* @brief Replace-SQL.
		*
		* @param sTableName
		* @param mpColumns
		* @return std::string insert-SQL
		*/
		std::string buildReplaceSQL(const std::string& sTableName, const std::map<std::string, std::pair<FT, std::string> >& mpColumns);

		/**
		* @brief Update-SQL.
		*
		* @param sTableName
		* @param mpColumns
		* @param sCondition where
		* @return std::string Update-SQL
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


		//Refresh alive time from connect start
		void refreshAliveTime() { _aliveTime = clock(); }

		//return alive time
		clock_t getAliveTime() const { return clock() - _aliveTime; }		

	//protected:
		///**
		//* @brief copy contructor
		//*/
		//MysqlHelper(const MysqlHelper& tcMysql);

		///**
		//*
		//* @brief only init no define
		//*/
		//MysqlHelper& operator=(const MysqlHelper& tcMysql);

	private:
		OCI_Connection* _cn;
		OCI_Statement* _conn;
		OCI_Resultset* rs;
		//MYSQL* _conn;		//a connection to MySQL
		clock_t _aliveTime;	//start time begina alive
		/**
		* pointer DB
		*/
		//MYSQL* _pstMql;

		///**
		//* CFG DB
		//*/
		//DBConf _dbConf;

		/**
		* flag connection
		*/
		bool _bConnected;

		/**
		* Last exect sql
		*/
		std::string _sLastSql;
	};