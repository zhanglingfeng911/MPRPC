#include "test.pb.h"
#include <iostream>
#include <string>
using namespace fixbug;

int main()
{
    //     LoginResponse rsp;
    //    ResultCode *rc= rsp.mutable_result();
    //    rc->set_errcode(1);
    //    rc->set_errmsg("登陆处理失败了");

    GetFriendListsResponse rsp;
    ResultCode *rc = rsp.mutable_result();
    //Resultcode是GetFriendListsResponse类内的对象 
    //需要通过返回一个mutalbe_result()返回一个类对象的指针 进而对其进行改变
    rc->set_errcode(0);

    User *user1 = rsp.add_friend_list();//列表类型的用法
    user1->set_name("zhang san");
    user1->set_age(20);
    user1->set_sex(User::MAN);

    User *user2 = rsp.add_friend_list();
    user2->set_name("li si");
    user2->set_age(22);
    user2->set_sex(User::MAN);

    std::cout << rsp.friend_list_size() << std::endl;//cout:2
    User user3=rsp.friend_list(1);//通过返回类型的friend_list(int index)方法 获取返回对象中的数据
    std::cout<<user3.name()<<std::endl;//打印：li si
    return 0;
}

int main1()
{
    //封装了login请求的数据
    LoginReques req;
    req.set_name("zhang san");
    req.set_pwd("123456");

    std::string send_str;
    if (req.SerializeToString(&send_str)) //将数据序列化，并将序列化好的数据放入send_str字符串里
    {
        std::cout << send_str << std::endl;
    }

    //获取序列化字符串中的数据。从send_str反序列化一个login请求对象
    LoginReques reqB;
    if (reqB.ParseFromString(send_str))
    {
        std::cout << reqB.name() << std::endl;
        std::cout << reqB.pwd() << std::endl;
    }

    return 0;
}