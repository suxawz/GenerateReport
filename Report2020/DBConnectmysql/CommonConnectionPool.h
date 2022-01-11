#pragma once

#include <string>
#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
#include <memory>
#include <functional>
#include <condition_variable>	//variable of condition for communicate with different threads
#include "connection.h"
using namespace std;

	/*
	singal mode
	1.private constract function
	2.static function get class object
	*/

	/*
	connection pool
	*/

	class ConnectionPoll
	{
	public:
		//get connection pool object
		static ConnectionPoll* getConnectionPoll();
		//interface to privade a vonnection
		shared_ptr<Connection> getConnection();
	private:
		//private constract 
		ConnectionPoll();

		//get info from CFG file
		bool loadConfigFile();

		/*
		run in new thread , responsible for generate new connection(when connections not enough)
		make thread function to member function easy to get member variable
		*/
		void produceConnectionTask();

		//scanner connection time > _maxIdleTime no used connection, relase this connection
		void scannerConnectionTask();

		string _ip;				//MySQL IP Address
		//unsigned short _port;	//Port defalt 3306
		int		_port;
		string _username;		//user name
		string _password;		//password
		string _dbname;			//DB name
		int _initSize;			//inite connection number
		int _maxSize;			//Max connection numer
		int _maxIdleTime;		//Max time for no used connection
		int _connectionTimeOut;	//connection pool connect time out

		queue<Connection*> _connectionQueue;	//save MySQL connection queue
		mutex _queueMutex;						//Matian threads safe lock
		atomic_int _connectionCount;			//sum connection number,atomic_int for threads safe
		condition_variable cv;					//set condition variable , use for connect generate thread and consume thread
	};