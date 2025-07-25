#include "biling.h"


// 全局变量，用于存储事件回调函数
static EventCallback eventCallback = nullptr;

// 设置事件回调函数
void setEventCallback(EventCallback callback) {
    eventCallback = callback;
}

// 事件处理函数：接受客户端连接并处理数据
void handleEvents(int server_fd) {
    while (true) {
        struct sockaddr_in clientAddress;
        socklen_t clientAddrLen = sizeof(clientAddress);
        int clientSocket = accept(server_fd, (struct sockaddr *)&clientAddress, &clientAddrLen);
        if (clientSocket < 0) {
            perror("accept failed");
            continue;
        }

        // 缓存数据
        std::vector<uint8_t> packetBuffer;
        const std::vector<uint8_t> PACKET_HEADER = {85, 170}; // 封包头
        const std::vector<uint8_t> PACKET_TAIL = {170, 85};  // 封包尾

        while (true) {
            uint8_t byte = 0;
            int bytesRead = read(clientSocket, &byte, 1);
            if (bytesRead <= 0) {
                std::cerr << "客户端断开连接或读取错误" << std::endl;
                break;
            }

            packetBuffer.push_back(byte);

            // 查找封包头
            if (packetBuffer.size() >= 2 &&
                packetBuffer[0] == PACKET_HEADER[0] &&
                packetBuffer[1] == PACKET_HEADER[1]) {
                //packetBuffer.erase(packetBuffer.begin(), packetBuffer.begin() + 2); // 移除封包头
            }

            // 查找封包尾
            if (packetBuffer.size() >= 2 &&
                packetBuffer[packetBuffer.size() - 2] == PACKET_TAIL[0] &&
                packetBuffer[packetBuffer.size() - 1] == PACKET_TAIL[1]) {
                //packetBuffer.resize(packetBuffer.size() - 2); // 移除封包尾

                // 触发事件回调
                if (eventCallback) {
                    eventCallback(clientSocket, packetBuffer);
                }

                packetBuffer.clear(); // 清空缓冲区，准备处理下一个封包
            }
        }

        close(clientSocket); // 关闭客户端连接
    }
}

int connectSocket(const std::string &address, int port) {
    int server_fd;
    struct sockaddr_in serverAddress;
    int opt = 1;

    // 创建套接字
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return -1;
    }

    // 设置套接字选项
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server_fd);
        return -1;
    }

    // 配置地址和端口
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(address.c_str());
    serverAddress.sin_port = htons(port);

    // 绑定套接字到地址和端口
    if (bind(server_fd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("bind failed");
        close(server_fd);
        return -1;
    }

    // 开始监听
    if (listen(server_fd, 10) < 0) {
        perror("listen failed");
        close(server_fd);
        return -1;
    }

    //std::cout << "服务器正在监听 " << address << ":" << port << "..." << std::endl;
    writeLog("服务器正在监听 " + address + ":" + std::to_string(port));
    return server_fd;
}