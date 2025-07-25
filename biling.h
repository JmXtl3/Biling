#ifndef BILLING_H
#define BILLING_H
#define CONFIG_H
#define CONFIG_H
#define DATABASE_H
#define DATABASE_H

#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <functional>
#include <sstream>
#include <iomanip> // For std::setw and other manipulators
#include <fstream>
#include <ctime>
#include <mysql/mysql.h>
#include <map>
#include <mutex>
#include <algorithm>
#include <iconv.h>
#include <arpa/inet.h>
#include <thread>
#include <chrono>

// 配置结构体
struct ServerConfig {
    // MySQL配置
    std::string mysql_host;
    int mysql_port;
    std::string mysql_user;
    std::string mysql_password;
    std::string mysql_db;
    
    // 服务器配置
    std::string bind_ip;
    int server_port;
    std::string app_title;
    bool auto_register;
};

// 全局配置对象
extern ServerConfig g_config;

// 全局MySQL连接句柄
extern MYSQL* g_mysql_connection;
//连接Mysql
bool connectMySQL();
bool BilingSetHumanPoint(std::string accountText,int point, int Newpoint);  //修改账号点数
int BilingGetHumanPoint(std::string accountText); //获取账号点数
void disconnectMySQL(); // 断开MySQL连接
bool isConnected(); // 检查MySQL连接是否有效


// 函数声明
bool loadConfig(const std::string& filename = "config.ini");
// 定义事件回调类型，改为支持字节数组
using EventCallback = std::function<void(int, const std::vector<uint8_t>&)>;

void onMessageReceived(int clientSocket, const std::vector<uint8_t> &message);

// 函数声明
int connectSocket(const std::string &address, int port);
void setEventCallback(EventCallback callback);
void handleEvents(int server_fd);
void writeLog(const std::string& message);
std::string ansiToUtf8(const std::string& ansiStr);
int BilingLogin(std::string accountText, std::string passwordText);
void BilingLeave(std::string accountText,int accountstate); // 更新账号状态
void logAllClientDurations();   // 定时线程：每5分钟写一次所有活跃连接的时长
#endif // BILLING_H