#include "mprpcchannel.h"
#include <string>
#include "rpcheader.pb.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <error.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "mprpcapplication.h"
#include <iostream>
#include <unistd.h>
#include"zookeeperutil.h"

/*消息格式
header_size+service_name\method_naem+args_size+args*/
//重写rpcchannel的callmethod方法
//所有通过stub代理对象调用的rpc方法 都走到这里 统一做rpc方法调用的数据序列化和网络发送
//方法是由calluservice.cc调用的
void MprpcChannel::CallMethod(const MethodDescriptor *method,
                              RpcController *controller, const Message *request,
                              Message *response, Closure *done)
{
    const google::protobuf::ServiceDescriptor *sd = method->service();
    std::string service_name = sd->name();
    std::string method_name = method->name();

    //获取参数的序列化字符串长度 args_size
    int args_size = 0;
    std::string args_str;
    //requeset里面包含方法的参数 （request是未序列化的！） Login:方法 参数为name和pwd
    if (request->SerializeToString(&args_str)) //将调用方法的参数数据序列化，并将序列化好的数据放入args_str字符串里
    {
        args_size = args_str.size();
    }
    else
    {
        std::cout << "serialize request error!" << std::endl;
        return;
    }

    //定义rpc请求header
    mprpc::RpcHeader rpcheader;
    rpcheader.set_service_name(service_name);
    rpcheader.set_method_name(method_name);
    rpcheader.set_args_size(args_size);

    uint32_t header_size = 0;
    std::string rpc_header_str;
    if (rpcheader.SerializeToString(&rpc_header_str))
    {
        header_size = rpc_header_str.size();
    }
    else
    {
        std::cout << "serialize rpcheader error!" << std::endl;
        return;
    }
    //定义rpc的请求header
    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char *)&header_size, 4));
    send_rpc_str += rpc_header_str;
    send_rpc_str += args_str;

    // 打印调试信息
    std::cout << "============================================" << std::endl;
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "============================================" << std::endl;

    //使用tcp变成 完成rpc的远程调用
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        std::cout << "create socket error! erron:" << error << std::endl;
        exit(EXIT_FAILURE);
    }
    //因为用户不知道提供某个服务的服务主机的ip和port所以用户只能根据serice_name和method_name取zkserver
    //查询ip:port
    //读取rpcserver配置信息
    //InetAddress(StringArg ip, uint16_t port, bool ipv6 = false);
    // std::string ip = MprpcApplication::getInstance().Getconfig().Load("rpcserverip");
    // uint16_t port = atoi(MprpcApplication::getInstance().Getconfig().Load("rpcserverport").c_str());
    
    ZkClient zkClil;
    zkClil.Start();
    // /Uservice/Login 根据这个路径去zookeeper查询

    std::string method_path="/"+service_name+"/"+method_name;

    std::string host_data=zkClil.GetData(method_path.c_str());
    if (host_data=="")
    {
        //  controller->SetFailed(method_path + " is not exist!");
        // return;
        //to do:改成controller
        std::cout<<method_path<<"is not exit!"<<std::endl;
        return;
    }
    int idx=host_data.find(":");
    if (idx==-1)
    {
         std::cout<<method_path<<" address is invalid!"<<std::endl;
        return;
    }
    
    std::string ip = host_data.substr(0, idx);
    uint16_t port = atoi(host_data.substr(idx+1, host_data.size()-idx).c_str()); 
    //填写client需要连接的server信息ip+port
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    server_addr.sin_port = htons(port);
    //连接rpc服务节点 clietn和server连接 同时自动绑定客户端ip和port
    if (connect(clientfd, (sockaddr *)&server_addr, sizeof(sockaddr_in)) == -1)
    {
        std::cout << "connect error! error:" << errno << std::endl;
        exit(EXIT_FAILURE);

        // std::cerr<<"connect server error"<<std::endl;
        close(clientfd);
        // exit(-1);
    }
    //发送rpc请求
    int len = send(clientfd, send_rpc_str.c_str(), send_rpc_str.size() + 1, 0);
    if (len == -1)
    {
        std::cout << "send send_rpc_str error! erron:" << errno << std::endl;
        close(clientfd);
        return;
        // exit(EXIT_FAILURE);
        // std::cerr<<"send send_rpc_str error:"<<send_rpc_str<<std::endl;
    }
    //接收rpc请求的响应
    char rec_buff[1024] = {0};
    len = recv(clientfd, rec_buff, 1024, 0);
    if (-1 == len)
    {
        std::cout << "recv error! erron:" << errno << std::endl;
        close(clientfd);
        return;
    }
    // std::string response_str(rec_buff);
    // std::string response_str(rec_buff, 0, len);//bug ：recev_buff遇到\0后面的字符就接受不到了
    //把调用rpc接收到的数据反序列到response
    // if (!response->ParseFromString(response_str))
    // {
    //     std::cout << "parse error! response_str:" << response_str << std::endl;
    //     close(clientfd);
    //     return;
    // }
    //把调用rpc接收到的数据反序列到response
    if (!response->ParseFromArray(rec_buff,len))
    {
        std::cout << "parse error! response_str:" << rec_buff << std::endl;
        close(clientfd);
        return;
    }
}