#include "Thread.h"
#include <stdio.h>
#include <iostream>

using std::cout;
using std::endl;

#if 0
Thread::Thread(const ThreadCallback &cb)
: _thid(0)
, _isRunning(false)
, _cb(cb)
{

}
#endif
Thread::Thread(ThreadCallback &&cb)
: _thid(0)
, _isRunning(false)
, _cb(std::move(cb))//注册
{

}

Thread::~Thread()
{

}

//线程的启动
void Thread::start()
{
    //shift + k
    //为了消除threadFunc的this问题，需要将其设置为static
    /* int ret = pthread_create(&_thid, nullptr, threadFunc, nullptr); */
    int ret = pthread_create(&_thid, nullptr, threadFunc, this);
    if(ret)
    {
        perror("pthread_create");
        return;
    }

    _isRunning = true;
}

//线程的停止
void Thread::stop()
{
    if(_isRunning)
    {
        int ret = pthread_join(_thid, nullptr);
        if(ret)
        {
            perror("pthread_join");
            return;
        }

        _isRunning = false;
    }
}

//线程入口函数
void *Thread::threadFunc(void *arg)
{
    //pth = arg = &mth
    Thread *pth = static_cast<Thread *>(arg);
    //Thread * this
    if(pth)
    {
        pth->_cb();//回调
    }
    else
    {
        cout << "nullptr == pth" << endl;
    }

    /* return nullptr; */
    pthread_exit(nullptr);
}

