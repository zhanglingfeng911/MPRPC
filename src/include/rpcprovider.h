#pragma once
#include "google/protobuf/service.h"
#include <memory>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <functional>
#include<string>
#include<google/protobuf/descriptor.h>
#include<unordered_map>





using namespace google::protobuf;
using namespace muduo;
using namespace muduo::net;

//框架提供的专门服务发布rpc服务的网络对象类
class RpcProvider
{
private:
    // //组合了TcpServer
    // // 组合的muduo库，实现服务器功能的类对象
    // std::unique_ptr<TcpServer> m_tcpserverptr;
    EventLoop m_eventloop; // 指向事件循环对象的指针

   void OnConnection(const TcpConnectionPtr&);
   void OnMessage(const TcpConnectionPtr&,
                            Buffer*,
                            Timestamp);

    //Service服务类型信息
    struct ServiceInfo
    {
        google::protobuf::Service *m_service;//保存服务对象
        //key:方法名字 value:方法描述
        std::unordered_map<std::string,const google::protobuf::MethodDescriptor*>m_methodMap;
    };

    //存储注册成功的服务对象和其服务方法的信息
    //key:服务名字 value:服务信息
    std::unordered_map<std::string,ServiceInfo>m_serviceMap;

    //Clousre的回调操作 用于序列化rpc的响应和网络发送
    void SendResponse(const TcpConnectionPtr&,Message*);
public:


    //框架提供给外部使用的 可以发布rpc方法的函数接口
    void NotifyService(google::protobuf::Service *service);

    //启动Rpc服务节点 开始提供rpc远程网络调用服务
    void Run();
};
