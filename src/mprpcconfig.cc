#include "mprpcconfig.h"
#include <iostream>
#include <string>
//负责查询配置信息
std::string MprpcConfig::Load(const std::string &key)
{
    auto it = m_configMap.find(key);
    if (it == m_configMap.end())
    {
        return "";
    }

    return it->second;
}

//负责解析加载配置文件
void MprpcConfig::LoadConfigFile(const char *config_file)
{
    FILE *pf = fopen(config_file, "r");
    if (pf == nullptr)
    {
        std::cout << config_file << "is not exit!" << std::endl;
        exit(EXIT_FAILURE);
    }

    while (!feof(pf))
    {
        //1.注释 2.正确的配置项 3.去掉开头多余的空格
        char buff[512] = {0};
        fgets(buff, 512, pf);
        //去掉字符串前面多余的空格

        std::string read_buff(buff);
        Trim(read_buff);

        //判断#号的注释
        if (read_buff[0] == '#' || read_buff.empty())
        {
            continue;
        }

        //解析配置项
        int idx = read_buff.find('=');
        if (idx == -1)
        {
            //配置项不合法
            continue;
        }

        std::string key;
        std::string value;
        key = read_buff.substr(0, idx);
        //去掉key前后的空格
        Trim(key);
        int endidx=read_buff.find('\n',idx);
        value = read_buff.substr(idx + 1, endidx-1 - idx);//去掉后面的换行符
        //去掉value前后的空格
        Trim(value);
        m_configMap.insert({key, value});
    }
    fclose(pf);
}

void MprpcConfig::Trim(std::string &src_buff)
{
    int idx = src_buff.find_first_not_of(' ');
    if (idx != -1)
    {
        //说明字符串前面有空格
        src_buff = src_buff.substr(idx, src_buff.size() - idx);
    }
    //去掉字符串后面多余的空格
    idx = src_buff.find_last_not_of(' ');
    if (idx != -1)
    {
        //说明字符串后面有空格
        src_buff = src_buff.substr(0, idx + 1);
    }
}