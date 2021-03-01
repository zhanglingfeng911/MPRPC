#include "rpcprovider.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include"logger.h"
#include"zookeeperutil.h"

using namespace std;
using namespace placeholders;

/*
service_name :service描述
*/
//框架提供给外部使用的 可以发布rpc方法的函数接口
void RpcProvider::NotifyService(google::protobuf::Service *service)
{

    ServiceInfo service_Info;
    //获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    //获取服务的名字
    std::string service_name = pserviceDesc->name();
    // std::cout << "service_name:" << service_name << std::endl;
    LOG_INFO("Service name:%s",service_name.c_str());
    //获取服务对象方法的数量
    int methodCnt = pserviceDesc->method_count();
    for (int i = 0; i < methodCnt; i++)
    {
        //获取了服务对象指定下标服务方法的描述
        const MethodDescriptor *pmethodDesc = pserviceDesc->method(i);
        string method_name = pmethodDesc->name();
        // std::cout << "method_name:" << method_name << std::endl;
        service_Info.m_methodMap.insert({method_name, pmethodDesc});
        LOG_INFO("method_name:%s",method_name.c_str());

    }
    service_Info.m_service = service;
    m_serviceMap.insert({service_name, service_Info});
}

//启动Rpc服务节点 开始提供rpc远程网络调用服务
void RpcProvider::Run()
{

    //InetAddress(StringArg ip, uint16_t port, bool ipv6 = false);
    std::string ip = MprpcApplication::getInstance().Getconfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::getInstance().Getconfig().Load("rpcserverport").c_str());
    InetAddress address(ip, port);

    //创建
    TcpServer server(&m_eventloop, address, "RpcProvider");

    //绑定连接回调和消息读写回调 分离了网络代码和业务代码

    // typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, _1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, _1, _2, _3));

    //设置Muduo库的线程数量
    server.setThreadNum(4);

    //自动会发送ping消息 维护和zkserver的心跳连接  session timeout:30s 
    //zkclient 网络I/O线程 会在1/3timeout时间发送ping消息
    //把当前rpc节点上要发布的服务全部注册到zkserver上面 让rpc client可以从zk上发现服务
    ZkClient zkCli;
    zkCli.Start();
    //service_name：永久性节点 method_name：临时性节点
    for (auto &sp:m_serviceMap)
    {
        //service_name eg:UserServiceRPc
        std::string Service_path="/"+sp.first;//sp.first：service_name为服务名字
        zkCli.create(Service_path.c_str(),nullptr,0);
        for (auto &mp:sp.second.m_methodMap)
        {
            //service_name/method_name  eg:/UserviceRpc/Login  数据是当前rpc服务节点的ip和port
            std::string method_path=Service_path+"/"+mp.first;//mp.firsr：method_name
            char method_path_data[128]={0};
            sprintf(method_path_data,"%s:%d",ip.c_str(),port);//记录rpc节点（即rpc服务器的ip port)
            //ZOO_EPHEMERAL临时性节点
            zkCli.create(method_path.c_str(),method_path_data,strlen(method_path_data),ZOO_EPHEMERAL);
        }
        
    }
    
    //rpc服务器准备启动 打印信息
    std::cout << "RpcProvider start servet at ip:" << ip << " port:" << port << std::endl;
    //启动网络服务
    server.start();
    //启动了Epoll_wait 以阻塞的方式等待远程的连接
    m_eventloop.loop();
}
//新的socket连接回调
void RpcProvider::OnConnection(const TcpConnectionPtr &conn)
{
    //如果客户端 断开连接
    if (!conn->connected())
    {
        conn->shutdown();
    }
}

/*
在框架内部 RpcProvider和RpcConsumer协商好的protobuf的数据类型
service_name method_name /args参数
在.proto文件 定义proto的message类型进行数据的序列化和反序列化

header_size(4字节 服务名字+方法名字+args_size) +header_str+（args_size)args_str

*/
//已建立用户的读写事件回调
void RpcProvider::OnMessage(const TcpConnectionPtr &conn,
                            Buffer *buffer,
                            Timestamp timestamp)
{
    //网络上接收远程rpc调用请求字符名 方法名字/参数
    std::string recv_buf = buffer->retrieveAllAsString();

    //从字符流中读取前4个字节的内容(这个内容为header_size的大小 包括了service_name method_naem args_size)
    uint32_t header_size = 0;
    recv_buf.copy((char *)&header_size, 4, 0);

    //rpcheader_str是开头4个字节以后的字符串
    //根据header_size读取数据头的原始字符流 反序列化数据得到rpc请求的详细信息
    std::string rpc_header_str = recv_buf.substr(4, header_size);

    //mprpc是namespace （rpcheader.proto文件中的0
    mprpc::RpcHeader rpcheader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if (rpcheader.ParseFromString(rpc_header_str))
    {
        //数据头反序列化成功
        service_name = rpcheader.service_name();
        method_name = rpcheader.method_name();
        args_size = rpcheader.args_size();
    }
    else
    {
        //数据头反序列化失败
        std::cout << "rpc_header_str"
                  << " parse_error!" << std::endl;
        return;
    }

    //获取rpc方法参数的字符流数据
    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    // 打印调试信息
    std::cout << "============================================" << std::endl;
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "============================================" << std::endl;

    //获取service对象和method对象
    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end())
    {
        std::cout << service_name << "is not exist!" << std::endl;
        return;
    }

    auto mit = it->second.m_methodMap.find(method_name);//it->second为serviceInfo
    if (mit == it->second.m_methodMap.end())
    {
        std::cout << service_name << ":" << method_name << "is not exist!" << std::endl;
        return;
    }

    Service *service = it->second.m_service;                        //获取服务对象 new userservice()
    const google::protobuf::MethodDescriptor *method = mit->second; //获取方法对象 login

    //生成rpc方法调用的请求request和响应response
    Message *request = service->GetRequestPrototype(method).New(); //获取service和method
    if (!request->ParseFromString(args_str))//request反序列化
    {
        std::cout << "request parse error,content:" << args_str << std::endl;
        return;
    }

    Message *response = service->GetResponsePrototype(method).New();

    //给下面callmethod方法调用 绑定一个clousre的回调函数
    google::protobuf::Closure *done = NewCallback<RpcProvider, const TcpConnectionPtr &, Message *>(this, &RpcProvider::SendResponse, conn, response);
    //在框架上根据远端rpc请求 调用当前rpc节点上的方法
    //virtual void Login(::google::protobuf::RpcController *controller,
    /*   const ::fixbug::LoginRequest *request,
                       ::fixbug::LoginResponse *response,
                       ::google::protobuf::Closure *done) */
    service->CallMethod(method, nullptr, request, response, done);
    //在CallMethod（以userservice中的login方法举例 其中会调用done->run()方法，run又会
    //回调sendresponse方法 （因为156行设置了done-》run的回调函数为SendResponse
}

//Clousre的回调操作 用于序列化rpc的响应和网络发送
void RpcProvider::SendResponse(const TcpConnectionPtr &conn, Message *response)
{
    std::string response_str;
    if (response->SerializeToString(&response_str))//response进行序列化
    {
        //序列化成功后 通过网络把rpc方法的结果发送给调用方
        conn->send(response_str);
    }
    else
    {
        std::cout << "serialize response-str error!" << std::endl;
    }
    conn->shutdown(); //模拟http的短链接服务 rpcprovider主动断开连接
}