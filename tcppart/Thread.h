#ifndef __THREAD_H__
#define __THREAD_H__

#include <pthread.h>
#include <functional>

using std::function;

class Thread
{
    using ThreadCallback = function<void()>;
public:
    /* Thread(const ThreadCallback &cb); */
    Thread(ThreadCallback &&cb);
    ~Thread();

    //线程的启动
    void start();
    //线程的停止
    void stop();

private:
    //线程入口函数
    static void *threadFunc(void *arg);
private:
    pthread_t _thid;//线程id
    bool _isRunning;//标识线程是否运行的标志
    ThreadCallback _cb;//
};


#endif
