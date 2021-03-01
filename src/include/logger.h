#pragma once
#include"lockqueue.h"
#include<string>

// 定义宏 LOG_INFO("xxx %d %s", 20, "xxxx")
#define LOG_INFO(logmsgformat, ...) \
    do \
    {  \
        Logger &logger = Logger::getInstance(); \
        logger.setLogLevel(INFO); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c); \
    } while(0) \

#define LOG_ERR(logmsgformat, ...) \
    do \
    {  \
        Logger &logger = Logger::getInstance(); \
        logger.setLogLevel(ERROR); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c); \
    } while(0) \


enum logLevel
{
    INFO,  //普通的日志信息
    ERROR, //错误信息
};

//mprpc框架的日志系统
class Logger
{
public:
    //获取日志类的单例
    static Logger& getInstance();
    //设置日志级别
    void setLogLevel(logLevel level);
    //写日志
    void Log(std::string msg);

private:
    int m_loglevel;//记录日志级别
    LockQueue<std::string>m_lockQue;//日志缓冲队列
    Logger(const Logger&)=delete;
    Logger(Logger&&)=delete;
    Logger();
};


