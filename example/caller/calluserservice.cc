#include<iostream>
#include "mprpcapplication.h"
#include"user.pb.h"
#include"mprpcchannel.h"

int main(int argc,char **argv)
{

    //整个程序启动以后 要使用mprpc框架享受rpc服务调用  一定要先调用框架的初始化函数
    MprpcApplication::Init(argc,argv);
    
    //演示调用远程发布的方法Login
    
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());

    //rpc方法的请求参数
    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");
    //rpc方法的调用
    fixbug::LoginResponse response;
    //发起rpc方法的调用 同步rpc的调用过程->MprpcChannel::callMethod
    stub.Login(nullptr,&request,&response,nullptr);//Rpcchannel->Rpcchannel::callmethod 做request方法调用参数的序列化和网络发送
    //一次rpc调用完成 读response响应的结果
    if (response.result().errcode()==0)
    {
        std::cout<<"rpc login response :"<<response.success()<<std::endl; 
    }
    else
    {
        std::cout<<"rpc login reponse error: "<<response.result().errmsg()<<std::endl;
    }
   
    

    
    return 0;
}