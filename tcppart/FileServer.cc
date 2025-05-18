#include "FileServer.h"
#include "TcpConnection.h"
#include <iostream>
#include <unordered_map>
#include <string>
#include <functional>
#include <sw/redis++/redis++.h>
#include <unistd.h>
#include <mysql/mysql.h>
#include <openssl/md5.h>
#include <utility>
#include <memory>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <SimpleAmqpClient/SimpleAmqpClient.h>


using namespace sw::redis;
using namespace std;
using namespace AmqpClient;

class UserValidate
{
public:
    static pair<bool,unsigned long long> validate(string uuid,Redis &redis)
    {
        unordered_map<string,string> result;
        redis.hgetall(uuid,inserter(result,result.begin()));
        if(result.size())
        { 
            return {true,stoull(result["userid"])};
        }
        return {false,-1};
    }
};

FileTask::FileTask(const TcpConnectionPtr &con)
:_con(con)
,_redis("redis://127.0.0.1:6379")
,_mysql(mysql_init(nullptr))
{
    if(_mysql == nullptr)
    {
        cout << "mysql init error" << endl;
        return;
    }
}
FileTask::~FileTask()
{
    if(_mysql)
    {
        mysql_close(_mysql);
    }
}

void FileTask::userValidate(bool valid)
{
    UserValidation uv;
    uv.valid = valid;
    _con->send((char *)&uv, sizeof(uv));
}  

void FileTask::process()
{
    // 文件处理操作
    char buff[sizeof(datauuid)] = {0};
    _con->receive(buff,sizeof(datauuid));
    datauuid *d = (datauuid *)buff;
    char *uuid = d->uuid; 
    auto res=UserValidate::validate(string(uuid),_redis);
    bool valid=res.first;
    auto userid=res.second;
    userValidate(valid);   // send2 client
    if(!valid){
        return;
    }
    const int filenamelen= d->namelen;
    char filename[filenamelen+1] = {0};
    _con->receive(filename,filenamelen);  // 文件名
    char fheaderinfo[sizeof(fileheader)] = {0};
    _con->receive(fheaderinfo,sizeof(fileheader));
    fileheader *fh = (fileheader *)fheaderinfo;
    if(fh->upload){
        string hashfilepath=savefile(fh,userid);
        // 写入数据库
        string sql= "insert into filetree (userid,filepath,index,hashfilepath,dirid,filesize) values ( "
        +to_string(userid) + "," + "'" +string(filename)+ 
        "',"+ to_string(fh->index) +",'" +  hashfilepath + "'," + to_string(fh->dirid) + "," + to_string(fh->filelen) + ");";
        mysql_query(_mysql,sql.c_str());
        _redis.sadd(to_string(userid),hashfilepath);  // set user
        // 设置key 24h有效
        _redis.setex(to_string(userid),4*60*60,hashfilepath);
    }else{
        downloadfile(fh,string(filename),userid);
    }
}

vector<pair<int,pair<string,unsigned long long>>> FileTask::getfilelist(unsigned long long uid,unsigned long long dirid, string filename){
    string querysql= "select index,hashfilepath,filesize from filetree where userid=" + to_string(uid) +" and dirid=" 
    + to_string(dirid) + " and filepath= '" + filename + "';";
    mysql_query(_mysql,querysql.c_str());  
    MYSQL_RES *res=mysql_store_result(_mysql);
    MYSQL_ROW row;  
    vector<pair<int,pair<string,unsigned long long>>> filelist;
    while((row=mysql_fetch_row(res))!=NULL)
    {
        int index=atoi(row[0]);
        string filepath=row[1];
        unsigned long long filesize= atoll(row[2]);
        pair<int,pair<string,unsigned long long>> item;
        item.first=index;
        item.second.first=filepath;
        item.second.second=filesize;
        filelist.push_back(item);
    }
    return filelist;
}

void FileTask::downloadfile(fileheader *fh, string filename,unsigned long long userid)
{
    // 直接从文件中读取
    auto filelist=getfilelist(userid,fh->dirid,filename);

    for(auto& fileinfo : filelist){
        int index=fileinfo.first;
        string filepath=fileinfo.second.first;
        unsigned long long filesize=fileinfo.second.second;
        int sfd= open(filename.c_str(),O_RDWR|O_CREAT|O_APPEND,0666);  // 下载就慢一点
        if(sfd < 0)
        {
            cout << "open file error" << endl;
            return;
        }
        // 文件大小
        char *data=(char*)mmap(nullptr,filesize,PROT_READ|PROT_WRITE,MAP_SHARED,sfd,0);
        fileheader downfh;
        downfh.index=index;   
        downfh.filelen=filesize;
        downfh.dirid=fh->dirid;
        downfh.upload=false;
        _con->send((char *)&downfh,sizeof(fileheader));
        _con->send(data,filesize);
        munmap(data,filesize);
        close(sfd);
    }
}

bool FileTask::isfileexits(string filename,unsigned long long userid)
{
    // 应该查表
    bool res=_redis.sismember(to_string(userid),filename);
    if(res) return true;
    string sql= "select count(*) from filetree where hashfilepath= \""+filename + "\";";
    mysql_query(_mysql,sql.c_str());
    MYSQL_RES *fileres=mysql_store_result(_mysql);
    MYSQL_ROW row=mysql_fetch_row(fileres);
    if(stoi(row[0])>0)
    {
        return true;
    }
    return false;
}


string FileTask::savefile(fileheader *fh,unsigned long long userid)
{
    auto filelen= fh->filelen;
    //char *filebuff = new char[filelen];
    string filename="/home/filedisk/";
    for(int i=0;i<16;i++)
    {
        char tmp[3]={0};
        sprintf(tmp,"%02x",fh->md5[i]);
        filename+=tmp;
    }
    // is_exits 存在则返回
    bool isexits=isfileexits(filename,userid);
    // if (isexits)
    // {
    //     // 直接返回
    //     _con->send("file exits",strlen("file exits"));
    //     return filename;
    // }
    fileexists fe;
    fe.exists = isexits;   //
    _con->send((char *)&fe, sizeof(fileexists));
    if(isexits)
    {
        return filename;
    }
    unique_ptr<char> filebuffptr(new char[filelen]);

    while(true){
        _con->receive(filebuffptr.get(),filelen);  
        // checkmd5  
        // 计算md5
        unsigned char md5[16]={0};
        MD5_CTX ctx;
        MD5_Init(&ctx);
        MD5_Update(&ctx, filebuffptr.get(), filelen);
        MD5_Final(md5, &ctx);
        // 比较
        if(memcmp(md5,fh->md5,16) == 0)
        {
            break;
        }else{
            fileerror fe;
            fe.error = true;
            _con->send((char *)&fe, sizeof(fileerror));
        }
    }
    int sfd= open(filename.c_str(),O_RDWR|O_CREAT|O_APPEND,0666);
    write(sfd,filebuffptr.get(),filelen);
    close(sfd);
    return filename;
}

FileServer::FileServer(size_t threadNum, size_t queSize
                       , const string &ip
                       , unsigned short port)
: _pool(threadNum, queSize)
, _server(ip, port)
{

}

FileServer::~FileServer()
{

}

void FileServer::start()
{   
    _pool.start();
    using namespace  std::placeholders;
    _server.setAllCallback(std::bind(&FileServer::onNewConnection, this, _1)
                           , std::bind(&FileServer::onMessage, this, _1)
                           , std::bind(&FileServer::onClose, this, _1));
    _server.start();
}

void FileServer::stop()
{
    _pool.stop();
    _server.stop();
}

void FileServer::onNewConnection(const TcpConnectionPtr &con)
{
    //  后面做个log操作
    Channel::ptr_t channel= Channel::Create();
    BasicMessage::ptr_t message=BasicMessage::Create(con->toString()+" has connected!");
    channel->BasicPublish("copyfile","cpfile",message);
}

void FileServer::onMessage(const TcpConnectionPtr &con)
{
    FileTask task(con);
    _pool.addTask(std::bind(&FileTask::process, &task));
}

void FileServer::onClose(const TcpConnectionPtr &con)
{
    Channel::ptr_t channel= Channel::Create();
    BasicMessage::ptr_t message=BasicMessage::Create(con->toString()+" has closed!");
    channel->BasicPublish("copyfile","cpfile",message);
    // cout << con->toString() << " has closed!" << endl;
}



int main(){
    FileServer server(10,100,"127.0.0.1",8888);
    server.start();
}