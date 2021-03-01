#pragma once
#include <queue>
#include <thread>
#include <mutex>              //pthred_mutex_t
#include <condition_variable> //pthread_condition_t 做线程通信

//异步写日志的日志队列
template <typename T>
class LockQueue
{
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condvariable;

public:
    //多个worker线程都会写日志queue
    void Push(const T &data)
    {
        std::unique_lock<std::mutex> lock(m_mutex);//是否需要改成Lock_guard
        m_queue.push(data);
        m_condvariable.notify_one();//通知写线程把日志写到本地磁盘上(只有1个线程)
    }
    //一个线程读日志queue,写日志文件
    T Pop()
    {
        std::unique_lock<std::mutex>lock(m_mutex);
        while(m_queue.empty())
        {
            //日志队列为空 线程进入wait状态
            m_condvariable.wait(lock);

        }
        T ret=m_queue.front();
        m_queue.pop();
        // m_condvariable.notify_all();//作用:通知所有阻塞在条件变量上的线程（即把日志写到消息队列的线程）
        //但这里没有线程阻塞i在Push方法上 因为是队列 没有容量限制 更无满不满
        return ret;
        
    }
};
