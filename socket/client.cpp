#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
using namespace std;

int main() {
    //套接字,IPV4 TCP
    //防御性编程：可能出错的地方就一定要判断
    //go:如果返回error的函数不判断err是不是空，是无法通过lint的
    int clientSocket =socket(AF_INET,SOCK_STREAM,0);
    if (clientSocket == -1){
        cout<<"socket err"<<endl;
        return -1;
    }
    
    //服务端地址信息
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("10.0.8.6");
    serverAddr.sin_port = htons(8888);
    
    //连接服务器
    int ret = connect(clientSocket,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
    if(ret != 0 ){
        cout<<"connect err"<<endl;
        return -1;
    }
    
    //发送一条数据,比较可控
    char buffer[] = "abcd";
    send(clientSocket,buffer,sizeof(buffer),0);
    
    char msg[1024];
    recv(clientSocket,msg,sizeof(msg),0);
    cout<<"server response"<<endl;
    cout<<msg<<endl;
    
    //主动关闭可以释放内存
    //池化技术(预分配)：内存池，线程池,连接池
    close(clientSocket);
    return 0;
}
