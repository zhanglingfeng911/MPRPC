#pragma once

#include<mprpcconfig.h>
//mprpc框架的初始化类 只有一个 设计成单例模式
class MprpcApplication
{
private:
    static MprpcConfig m_config;
    MprpcApplication(){}
    MprpcApplication(const MprpcApplication&)=delete;
    MprpcApplication(const MprpcApplication&&)=delete;
public:
    static MprpcApplication& getInstance();
   static MprpcConfig &Getconfig();
    static void Init(int argc,char **argv);
};


