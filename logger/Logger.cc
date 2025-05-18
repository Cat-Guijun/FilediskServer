/**
 *  loggeer server
 *  log4cpp : 1. layout  2. appender 3 .catagory 4.writer
 */


#include <string>
#include <SimpleAmqpClient/SimpleAmqpClient.h>
#include <SimpleAmqpClient/Channel.h>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>

using namespace std;
using namespace AmqpClient;
using namespace log4cpp;

class Logger{
public:
    Logger(string queueName)
    : _queueName(queueName),_isRunning(false)
    {
        _channel=Channel::Create();
        ptrlayout=new PatternLayout();
        ptrlayout->setConversionPattern("%d{yyyy-MM-dd HH:mm:ss} %p %c %m%n");
        _fileappender=new FileAppender("fileappender","filelog.txt");
        _fileappender->setLayout(ptrlayout);
        Category::getInstance("Filelog");
    }
    ~Logger(){
        if(_fileappender) delete _fileappender;
        if(ptrlayout) delete ptrlayout;
    }
    void start(){
        Category& logcategory=Category::getInstance("Filelog");
        logcategory.addAppender(_fileappender);
        while(true){
            // 
            Envelope::ptr_t mesg=_channel->BasicConsumeMessage(_queueName);
            if(mesg){
                logcategory.info(mesg->Message()->Body());
            }
        }
    }
    void stop(){
        _isRunning==false;
    }

private:
    Channel::ptr_t _channel;
    const string _queueName;
    bool _isRunning;
    PatternLayout *ptrlayout;
    FileAppender *_fileappender;
};