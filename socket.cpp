#include "biling.h"


// 全局变量，用于存储事件回调函数
static EventCallback eventCallback = nullptr;

// 全局变量：保存活跃连接的起始时间
std::map<int, std::chrono::steady_clock::time_point> g_clientStartTimes;
std::mutex g_clientTimesMutex;

// 设置事件回调函数
void setEventCallback(EventCallback callback) {
    eventCallback = callback;
}

// 单独处理每个客户端的函数
void handleClient(int clientSocket, sockaddr_in clientAddress) {
    auto startTime = std::chrono::steady_clock::now();
    {
        std::lock_guard<std::mutex> lock(g_clientTimesMutex);
        g_clientStartTimes[clientSocket] = startTime;
    }

    char clientIP[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &(clientAddress.sin_addr), clientIP, INET_ADDRSTRLEN);
    writeLog(std::string("新客户端连接: ") + clientIP + ":" + std::to_string(ntohs(clientAddress.sin_port)));
    std::vector<uint8_t> packetBuffer;
    const std::vector<uint8_t> PACKET_HEADER = {85, 170};
    const std::vector<uint8_t> PACKET_TAIL = {170, 85};

    while (true) {
        uint8_t byte = 0;
        int bytesRead = read(clientSocket, &byte, 1);
        if (bytesRead <= 0) {
            std::cerr << "客户端断开连接或读取错误" << std::endl;
            break;
        }
        packetBuffer.push_back(byte);

        if (packetBuffer.size() >= 2 &&
            packetBuffer[packetBuffer.size() - 2] == PACKET_TAIL[0] &&
            packetBuffer[packetBuffer.size() - 1] == PACKET_TAIL[1]) {
            if (eventCallback) {
                eventCallback(clientSocket, packetBuffer);
            }
            packetBuffer.clear();
        }
    }
    close(clientSocket);
    std::lock_guard<std::mutex> lock(g_clientTimesMutex);
    g_clientStartTimes.erase(clientSocket);
}

// 主事件循环，负责接收连接
void handleEvents(int server_fd) {
    while (true) {
        struct sockaddr_in clientAddress;
        socklen_t clientAddrLen = sizeof(clientAddress);
        int clientSocket = accept(server_fd, (struct sockaddr *)&clientAddress, &clientAddrLen);
        if (clientSocket < 0) {
            perror("accept failed");
            continue;
        }
        // 每个连接新建一个线程处理
        std::thread clientThread(handleClient, clientSocket, clientAddress);
        clientThread.detach(); // 让线程后台运行，自动回收资源
    }
}

// 定时线程：每5分钟写一次所有活跃连接的时长
void logAllClientDurations() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::minutes(5));
        auto now = std::chrono::steady_clock::now();

        std::lock_guard<std::mutex> lock(g_clientTimesMutex);
        for (const auto& kv : g_clientStartTimes) {
            int clientSocket = kv.first;
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - kv.second).count();
            writeLog("客户端fd=" + std::to_string(clientSocket) + " 已连接时长: " + std::to_string(duration) + " 秒");
        }
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