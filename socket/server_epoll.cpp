#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <thread>
#include <sys/epoll.h>
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
    
    //4.IO多路复用-epoll
    int epFd = epoll_create1(0);
    if(epFd == -1){
        cout<<"epoll create fail"<<endl;
        return -1;
    }
    //用epoll监听网络请求
    epoll_event event;
    event.events = EPOLLIN | EPOLLET;//EPOLLIN | EPOLLOUT | EPOLLET | EPOLLLT;
    event.data.fd = socketFd;
    //添加到epFd
    if(epoll_ctl(epFd,EPOLL_CTL_ADD,socketFd,&event) == -1){
        cout<<"epoll_ctl err"<<endl;
        return -1;
    }
    
    while(true){
        epoll_event events[1024];
        int number = epoll_wait(epFd,events,1024,-1);
        for(int i=0;i<number;++i){
            if(events[i].data.fd == socketFd){
                //有新的连接
                int clientFd = accept(socketFd,nullptr,nullptr);
                if(clientFd == -1){
                    cout<<"accept err"<<endl;
                }
                else{
                    event.events = EPOLLIN;
                    event.data.fd = clientFd;
                    epoll_ctl(epFd,EPOLL_CTL_ADD,clientFd,&event);
                    cout<<"client connected,fd:"<<clientFd<<endl;
                }
            }
            else{
                //有数据可读
                if(events[i].events & EPOLLIN){
                    char buffer[1024];
                    int bytesRead = read(events[i].data.fd,buffer,sizeof(buffer));
                    if(bytesRead <= 0){
                        epoll_ctl(epFd,EPOLL_CTL_DEL,events[i].data.fd,nullptr);
                        close(events[i].data.fd);
                        cout<<"client closed, fd:"<<events[i].data.fd<<endl;
                    }
                    else{
                        write(events[i].data.fd,buffer,bytesRead);
                    }
                }
            }
        }
    }
    
    return 0;
}
