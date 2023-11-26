#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <thread>
using namespace std;

int setNonBlocking(int fd){
    //fctnl函数
    //按位设置
    //4种状态，1,2,3,4,4个int,4*4=16字节
    //bit,1个int,4字节，32位
    //0000 0111
    //第三位是1,那么代表这个状态存在
    int flags = fcntl(fd,F_GETFL,0);
    if(flags == -1){
        cout<<"fcntl get err"<<endl;
        return -1;
    }
    // flags = 0100 0011
    if(fcntl(fd,F_SETFL,flags | O_NONBLOCK) == -1){
        cout<<"fcntl set err"<<endl;
        return -1;
    }
    return 0;
}

int main(){
    //1.建立一个 socket
    // google 命名规范，驼峰命名，匈牙利命名法
    int socketFd = socket(AF_INET,SOCK_STREAM,0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;//允许来自任意地址的连接请求
    //htons:host to net short,主机序转网络序
    //大端、小段序
    //0000 0000,假设存的是1，网络序(大):1000 0000,主机序(小):0000 0001
    serverAddr.sin_port = htons(8888);
    
    //2.绑定
    int ret = bind(socketFd,(struct sockaddr *)&serverAddr,sizeof(serverAddr));
    if (ret != 0){
        cout<<"bind err"<<endl;
        return -1;
    }
    
    //3.监听,允许保存的最大监听数量排队数
    ret = listen(socketFd,10);
    if (ret != 0){
        cout<<"listen err"<<endl;
        return -1;
    }
    //初始化fd数组
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(socketFd,&readSet);

    //4.accept,等待接受连接
    while(true){
        fd_set currentSet = readSet;
        if(select(FD_SETSIZE,&currentSet,nullptr,nullptr,nullptr) == -1){
            cout<<"select error"<<endl;
            break;
        }
        //判断是否有新的请求
        if(FD_ISSET(socketFd,&currentSet)){
            int newFd = accept(socketFd,nullptr,nullptr);
            if(newFd == -1){
                cout<<"accept err"<<endl;
                continue;
            }
            else{
                cout<<"new connection accepted"<<endl;
                FD_SET(newFd,&readSet);
            }
        }
        //处理已经连接的请求
        for(int i= socketFd + 1;i<FD_SETSIZE;++i){
            if(FD_ISSET(i,&currentSet)){
                char buffer[1024];
                while(true){
                    int counts = recv(i,buffer,sizeof(buffer),0);
                    if(counts <= 0){
                        FD_CLR(i,&readSet);
                        break;
                    }
                    //echo服务器
                    send(i,buffer,counts,0);
                }
                sleep(3);
                //关闭连接
                close(i);
                cout<<"connection closed, fd:"<<i<<endl;
            }
        }
    }
    return 0;
}
