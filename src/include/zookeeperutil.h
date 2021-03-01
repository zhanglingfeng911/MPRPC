#pragma once
#include<semaphore.h>
#include<zookeeper/zookeeper.h>
#include<string>

//封装的zk客户端类
class ZkClient
{
private:

    //zk的客户端句柄
    zhandle_t *m_zhandle;

public:
    ZkClient(/* args */);
    ~ZkClient();
    //zkclient启动连接zkserver
    void Start();
    //在zkserver上 根据指定的path创建node节点 state默认为0 是永久性节点
    void create(const char *path,const char *data,int datalen,int state=0);
    //根据参数指定的znode节点路径 获取znode节点的值
    std::string GetData(const char *path);
};


