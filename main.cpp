#include "biling.h"


int main()
{
   // 设置监听地址和端口
    std::string address = "127.0.0.1";
    int port = 9470;
    //读取配置
    loadConfig();
    // 初始化日志
    writeLog("biling服务器启动");
    // 初始化数据库
    if (!connectMySQL())
    {
        writeLog("数据库初始化失败");
        return -1;
    }
    // 创建并绑定套接字
    int server_fd = connectSocket(address, port);
    if (server_fd < 0)
    {
        return -1;
    }
    //启动定时线程
    std::thread(logAllClientDurations).detach();
    // 设置事件回调
    setEventCallback(onMessageReceived);

    // 开始处理事件
    handleEvents(server_fd);

    // 关闭服务器套接字
    close(server_fd);
    return 0;
}
