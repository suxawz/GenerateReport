#include "CommonConnectionPool.h"
#include "public.h"


	//THreads safe singal function
	ConnectionPoll* ConnectionPoll::getConnectionPoll()
	{
		static ConnectionPoll pool;
		return &pool;
	}

	//get CFG info
	bool ConnectionPoll::loadConfigFile()
	{
		FILE* fp;
		errno_t err = fopen_s(&fp, "D:\\users.txt", "r");
		//FILE* fp = fopen_s("mysql.ini", "r");
		if (fp == nullptr)
		{
			LOG("mysql.ini file is not exist");
			return false;
		}

		while (!feof(fp))
		{
			char line[1024] = { 0 };
			fgets(line, 1024, fp);
			string str = line;
			int idx = str.find('=', 0);
			if (idx == -1)	//invalide cfg value
			{
				continue;
			}
			int endidx = str.find('\n', idx);

			string key = str.substr(0, idx);
			string value = str.substr(idx + 1, endidx - idx - 1);

			if (key == "ip")
			{
				_ip = value;
			}
			else if (key == "port")
			{
				_port = atoi(value.c_str());
			}
			else if (key == "username")
			{
				_username = value;
			}
			else if (key == "password")
			{
				_password = value;
			}
			else if (key == "dbname")
			{
				_dbname = value;
			}
			else if (key == "initSize")
			{
				_initSize = atoi(value.c_str());
			}
			else if (key == "maxSize")
			{
				_maxSize = atoi(value.c_str());
			}
			else if (key == "maxIdelTime")
			{
				_maxIdleTime = atoi(value.c_str());
			}
			else if (key == "connectionTimeOut")
			{
				_connectionTimeOut = atoi(value.c_str());
			}
		}
		return true;
	}

	//connection pool 
	ConnectionPoll::ConnectionPoll()
	{
		//get CFG
		if (!loadConfigFile())
			return;

		//creat init connection 
		for (int i = 0; i < _initSize; ++i)
		{
			Connection* p = new Connection();
			p->connect(_ip, _port, _username, _password, _dbname);
			p->refreshAliveTime();		//refresh start connection time
			_connectionQueue.push(p);
			_connectionCount++;
		}

		//start a thread for generate
		thread produce(std::bind(&ConnectionPoll::produceConnectionTask, this));
		produce.detach();

		//start a scanner thread
		thread scanner(std::bind(&ConnectionPoll::scannerConnectionTask, this));
		scanner.detach();
	}

	//Thread function,generate new connection
	void ConnectionPoll::produceConnectionTask()
	{
		while (1)
		{
			unique_lock<mutex> lock(_queueMutex);	//condition_variable wite for condition in  unique_lock<mutex>
			{
				cv.wait(lock);//queue not null,generatee thread wite, lack will realse 
			}

			if (_connectionCount < _maxSize)
			{
				Connection* p = new Connection();
				if (!(p->connect(_ip, _port, _username, _password, _dbname)))
				{
					LOG("failed to create thread");
				}
				p->refreshAliveTime();		//refresh time
				_connectionQueue.push(p);
				_connectionCount++;
			}

			//tell consume thread , connection can be used
			cv.notify_all();

		}	//relase mutex lack auto
	}

	//interface to get a connection for user
	shared_ptr<Connection> ConnectionPoll::getConnection()
	{
		unique_lock<mutex> lock(_queueMutex);	//currect thread get connection from queue should add lock

		while (_connectionQueue.empty())
		{
			if (cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(_connectionTimeOut)))	//weak up time out
			{
				if (_connectionQueue.empty())
				{
					LOG("time out get connection");
					return nullptr;
				}
			}		//
		}

		/*
		shared_ptr de constract connection will be closed
		need define realse resourse function, give back connection to queue
		*/
		shared_ptr<Connection> p(_connectionQueue.front(),
			[&](Connection* p) {
				unique_lock<mutex> lock(_queueMutex);	//operate connect queue must use lock, for thread safe
				p->refreshAliveTime();		//refresh time
				_connectionQueue.push(p);
			}
		);
		_connectionQueue.pop();
		if (_connectionQueue.empty())
		{
			cv.notify_all();	//connect queue empty,inform generatee
		}

		return p;
	}

	//scanner connection time 
	void ConnectionPoll::scannerConnectionTask()
	{
		while (1)
		{
			this_thread::sleep_for(chrono::seconds(_maxIdleTime));
			//scann queue, realse connection.
			unique_lock<mutex> lock(_queueMutex);
			while (_connectionCount > _initSize)
			{
				Connection* p = _connectionQueue.front();
				if (p->getAliveTime() >= _maxIdleTime * 1000)
				{
					_connectionQueue.pop();
					_connectionCount--;
					delete p;	//~Connection()
				}
				else
				{
					break;
				}
			}
		}
	}