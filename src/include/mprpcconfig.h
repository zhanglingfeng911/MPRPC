#pragma once 
#include<unordered_map>
#include<string>

//rpcserver_ip rpcserver_port zookeeper_ip zookeeper_port
//框架读取配置文件类
class MprpcConfig
{
private:

    std::unordered_map<std::string,std::string>m_configMap;

public:


    //负责查询配置信息
    std::string Load(const std::string &key);

    //负责解析加载配置文件
   void LoadConfigFile(const char * config_file);
    //去掉字符串前后的空格
   void Trim(std::string &src_buff);
};


