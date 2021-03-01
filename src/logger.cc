#include "logger.h"
#include"time.h"
#include<iostream>

//获取日志类的单例
Logger &Logger::getInstance()
{
    static Logger logger;
    return logger;
}
//设置日志级别
void Logger::setLogLevel(logLevel level)
{
    m_loglevel=level;
}
//写日志,把日志信息写入外部queue缓冲区当中
void Logger::Log(std::string msg)
{
    m_lockQue.Push(msg);

}


Logger::Logger()
{
    //启动专门的写日志线程 这个线程负责把日志信息写入到本地磁盘 做磁盘I/O操作
    std::thread writeLogTask(
        [&]() {
            for (;;)
            {
                //获取当天的日期 然后取日志信息 写入相应的日志文件当中 “"a+"
                time_t now=time(nullptr);//获取1970年到现在的秒数
                tm *nowtm=localtime(&now);
                
                char filename[128];
                sprintf(filename,"%d-%d-%d-log.txt",nowtm->tm_year+1900,nowtm->tm_mon+1,nowtm->tm_mday);

                FILE *pf=fopen(filename,"a+");
                if (pf==nullptr)
                {
                    std::cout<<"logger file: "<<filename<<" opne error!"<<std::endl;
                    exit(EXIT_FAILURE);
                }
                
                std::string msg=m_lockQue.Pop();//把消息从queue缓冲区取出 (后面会把它写到磁盘上)
                char time_buf[128]={0};
                sprintf(time_buf,"%d:%d:%d=>[%s] ",
                        nowtm->tm_hour,
                        nowtm->tm_min,
                        nowtm->tm_sec,
                        (m_loglevel == INFO ? "info" : "error"));
                msg.insert(0,time_buf);//把时分秒信息 放到日志信息首部
                msg.append("\n");
                fputs(msg.c_str(),pf);
                fclose(pf);
            }
            
        });

    //设置分离线程
    writeLogTask.detach();
}