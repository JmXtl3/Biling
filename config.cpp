#include "biling.h"

// 全局配置对象定义
ServerConfig g_config;

// 去除字符串首尾空格
std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

// 读取配置文件
bool loadConfig(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开配置文件: " << filename << std::endl;
        return false;
    }
    
    std::string line;
    std::string currentSection;
    std::map<std::string, std::map<std::string, std::string>> configData;
    
    while (std::getline(file, line)) {
        line = trim(line);
        
        // 跳过空行和注释
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        // 检查是否是节标题
        if (line[0] == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.length() - 2);
            continue;
        }
        
        // 解析键值对
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = trim(line.substr(0, pos));
            std::string value = trim(line.substr(pos + 1));
            configData[currentSection][key] = value;
        }
    }
    
    file.close();
    
    // 解析MySQL配置
    g_config.mysql_host = configData["mysql"]["host"];
    g_config.mysql_port = std::stoi(configData["mysql"]["port"]);
    g_config.mysql_user = configData["mysql"]["user"];
    g_config.mysql_password = configData["mysql"]["password"];
    g_config.mysql_db = configData["mysql"]["db"];
    
    // 解析服务器配置
    g_config.bind_ip = configData["server"]["bind_ip"];
    g_config.server_port = std::stoi(configData["server"]["port"]);
    g_config.app_title = configData["server"]["app_title"];
    
    std::string auto_reg = configData["server"]["auto_register"];
    std::transform(auto_reg.begin(), auto_reg.end(), auto_reg.begin(), ::tolower);
    g_config.auto_register = (auto_reg == "true" || auto_reg == "1");
    
    // std::cout << "配置文件加载成功!" << std::endl;
    // std::cout << "MySQL: " << g_config.mysql_host << ":" << g_config.mysql_port 
    //           << "/" << g_config.mysql_db << std::endl;
    // std::cout << "服务器: " << g_config.bind_ip << ":" << g_config.server_port << std::endl;
    
    return true;
}