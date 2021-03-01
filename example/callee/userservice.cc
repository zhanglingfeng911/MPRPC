#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include "logger.h"
/*
UserService原来是一个本低服务，提供了两个进程内的本地方法 Login和GetFreindLists

*/
using namespace fixbug;
class UserService : public UserServiceRpc //使用在rpc服务发布端（rpc服务提供者)
{
private:
    /* data */
public:
    //本地业务
    bool login(std::string name, std::string pwd)
    {
        std::cout << "doing local service :login" << std::endl;
        std::cout << "name:" << name << " pwd:" << pwd << std::endl;
        return true;
        // return false;
    }

    // RPC业务 重写基类UserServiceRpc的虚函数 下面这些方法是框架直接调用
    virtual void Login(::google::protobuf::RpcController *controller,
                       const ::fixbug::LoginRequest *request,
                       ::fixbug::LoginResponse *response,
                       ::google::protobuf::Closure *done)
    {

        //框架给业务上报了请求参数 loginRequest,应用获取相应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        //做本地业务
        bool login_result = login(name, pwd);

        //把响应写入 包括错误码、错误消息、返回值
        ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        // code->set_errcode(1);
        // code->set_errmsg("login error");

        response->set_success(login_result);

        //执行回调操作 执行响应对象数据的序列化和网络发送（都是由框架来完成的）
        done->Run();
    }
};

int main(int argc, char **argv)
{
    LOG_INFO("first log message!");
    LOG_ERR("%s:%s:%d",__FILE__,__FUNCTION__,__LINE__);

    //调用框架的初始化
    MprpcApplication::Init(argc, argv);

    //provider是一个rpc网络服务对象，把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new UserService());

    //启动一个rpc服务发布节点 Run以后 线程进入阻塞状态 等待远程的rpc调用请求
    provider.Run();

    return 0;
}