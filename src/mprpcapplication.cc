#include "mprpcapplication.h"
#include <iostream>
#include <unistd.h>
#include <string>

MprpcConfig MprpcApplication::m_config;

MprpcConfig &MprpcApplication::Getconfig()
{
    return m_config;
}

void showArgsHelp()
{
    std::cout << "format command -i <configfile> " << std::endl;
}
MprpcApplication &MprpcApplication::getInstance()
{
    static MprpcApplication app;
    return app;
}
void MprpcApplication::Init(int argc, char **argv)
{
    if (argc < 2)
    {
        showArgsHelp();
        exit(EXIT_FAILURE);
    }
    int c = 0;
    //单个字符后接一个＂　：＂，　表示该选项后必须跟一个参数值．参数紧跟在选项后或者以空格隔开.该参数的指针赋值给optarg
    std::string config_file;
    while ((c = getopt(argc, argv, "i:")) != -1)
    {
        switch (c)
        {
        case 'i':
            config_file = optarg;
            break;
        case '?':
            // std::cout<<"invalid args"<<std::endl;
            showArgsHelp();
            exit(EXIT_FAILURE);
        case ':':
            // std::cout<<"need <configfile>"<<std::endl;
            showArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }

    //开始加载配置文件 rpcserver_ip= rpcserver_port= zookeeper_ip= zookeeper_port=
    m_config.LoadConfigFile(config_file.c_str());

    // std::cout<<"rpcserverip:"<<m_config.Load("rpcserverip")<<std::endl;
    // std::cout<<"rpcserverport:"<<m_config.Load("rpcserverport")<<std::endl;
    // std::cout<<"zookeeperip:"<<m_config.Load("zookeeperip")<<std::endl;
    // std::cout<<"zookeeperport:"<<m_config.Load("zookeeperport")<<std::endl;

    // rpcserverip=127.0.0.1
    // #rpc节点的port端口号
    // rpcserverport=8000
    // zookeeperip=127.0.0.1
    // zookeeperport=5000
}