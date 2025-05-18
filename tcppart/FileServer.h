#ifndef __FILESERVER_H__
#define __FILESERVER_H__

#include "ThreadPool.h"
#include "TcpServer.h"
#include <sw/redis++/redis++.h>
#include <mysql/mysql.h>
#include <utility>
#include <string>
#include "CusInfo.h"
using namespace sw::redis;
using namespace std;
class FileTask
{
public:
    FileTask(const TcpConnectionPtr &con);
    void process();
    void userValidate(bool valid);
    string savefile(fileheader *fh,unsigned long long userid);
    void downloadfile(fileheader *fh, string filename,unsigned long long userid);
    vector<pair<int,pair<string,unsigned long long>>> getfilelist(unsigned long long uid,unsigned long long dirid, string filename);
    bool isfileexits(string filename,unsigned long long userid);
    ~FileTask();
private:
    TcpConnectionPtr _con;
    Redis _redis;
    MYSQL *_mysql;
};

class FileServer
{
public:
    FileServer(size_t threadNum, size_t queSize
               , const string &ip, 
               unsigned short port);
    ~FileServer();

    void start();
    void stop();

    void onNewConnection(const TcpConnectionPtr &con);
    void onMessage(const TcpConnectionPtr &con);
    void onClose(const TcpConnectionPtr &con);

private:
    ThreadPool _pool;
    TcpServer _server;
};

#endif
