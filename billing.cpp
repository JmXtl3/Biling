#include "biling.h"

void printByteArray(const std::vector<uint8_t> &byteArray)
{
    std::cout << "{";
    for (size_t i = 0; i < byteArray.size(); ++i)
    {
        std::cout << static_cast<int>(byteArray[i]); // 将 uint8_t 转换为 int 输出
        if (i < byteArray.size() - 1)
        {
            std::cout << ",";
        }
    }
    std::cout << "}" << std::endl;
}

std::string passwordAlgorithm(const std::vector<uint8_t> &packet)
{
    std::ostringstream hexStream;
    for (uint8_t byte : packet)
    {
        hexStream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte & 0xFF);
    }
    return hexStream.str();
}

// 账号登录验证 162
std::vector<uint8_t> Account_InLogin(const std::vector<uint8_t> &message)
{
    // 构建响应包
    std::vector<uint8_t> responsePacket = {85, 170, 0};

    int AccountLength = message[11]; // 从消息中获取账号长度字段
    // 从封包的第12个字节开始，取账号长度相应的字节
    std::string accountText(message.begin() + 12, message.begin() + 12 + AccountLength);
    // 从封包中提取密码字段
    std::vector<uint8_t> passwordBytes(message.begin() + (message.size() - 18), message.begin() + (message.size() - 18) + 16);
    std::string passwordText = passwordAlgorithm(passwordBytes);
    //验证账号密码是否正确
    int vlue = BilingLogin(accountText, passwordText);
    if (vlue == 0 ||vlue == 1 )
    {
        responsePacket.push_back(AccountLength + 5); // 长度字段，+5是因为后面还有5个字节
        responsePacket.push_back(162);                // 消息类型
        // 取从第4个字节开始，到第(4 + AccountLength)个字节的数据
        responsePacket.insert(responsePacket.end(), message.begin() + 5, message.begin() + 7);
        responsePacket.push_back(AccountLength);  
        // 添加账号的数据
        responsePacket.insert(responsePacket.end(), message.begin() + 12, message.begin() + 12 + AccountLength); // 从第5个字节开始取2个
        if (vlue == 0)
        {
            // 添加额外的固定字节
            responsePacket.insert(responsePacket.end(), {2, 170, 85});
        }else
        {
            // 添加额外的固定字节
            responsePacket.insert(responsePacket.end(), {4, 170, 85});
        }
        // 返回构建好的响应包
        return responsePacket;
    }
    responsePacket.push_back(AccountLength + 25); // 长度字段，+5是因为后面还有5个字节
    responsePacket.push_back(162);                // 消息类型
    // 从消息中提取特定字节并添加到响应包
    responsePacket.insert(responsePacket.end(), message.begin() + 5, message.begin() + 7); // 从第5个字节开始取2个

    // 账号长度
    responsePacket.push_back(AccountLength);

    // 添加账号的数据
    responsePacket.insert(responsePacket.end(), message.begin() + 12, message.begin() + 12 + AccountLength); // 从第5个字节开始取2个

    // 添加额外的固定字节
    responsePacket.insert(responsePacket.end(), {1, 113, 132, 240, 91, 127, 252, 78, 0, 0, 0, 0, 78, 78, 78, 78, 78, 78, 78, 0, 0, 170, 85});

    // 返回构建好的响应包
    return responsePacket;
}

// 角色进入游戏验证 163
std::vector<uint8_t> Account_InGame(const std::vector<uint8_t> &message)
{
    // 构建响应包
    std::vector<uint8_t> responsePacket = {85, 170, 0};
    int AccountLength = message[7]; // 从消息中获取账号长度字段
    //已知账号长度在第8个字节 且发送来的数据 这个字节 是15
    responsePacket.push_back(AccountLength + 21); // 消息类型
    // 添加账号的数据
    // 取从第4个字节开始，到第(4 + AccountLength)个字节的数据
    responsePacket.insert(responsePacket.end(), message.begin() + 4, message.begin() + 8 + AccountLength);
    // 添加额外的固定字节
    responsePacket.insert(responsePacket.end(), {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 170, 85});
    // 获取账号数据
    std::string accountText(message.begin() + 8, message.begin() + 8 + AccountLength);
    // 获取角色数据
    int NametLength = message[8 + AccountLength]; // 从消息中获取角色名称长度字段
    int NameStartIndex = 9 + AccountLength; // 角色名称的起始索引
    std::string nameText(message.begin() + NameStartIndex , message.begin() + NameStartIndex + NametLength);
    // 打印账号和角色数据
    writeLog("账号[" + accountText + "]进入游戏，角色名称: " + ansiToUtf8(nameText));
    //更新账号状态
    BilingLeave(accountText, 1); // 更新账号状态为在线
    // 返回构建好的响应包
    return responsePacket;
}


// 角色离开游戏验证 164
std::vector<uint8_t> Account_InLeave(const std::vector<uint8_t> &message)
{
    // 构建响应包
    std::vector<uint8_t> responsePacket = {85, 170, 0};
    int AccountLength = message[7]; // 从消息中获取账号长度字段
    responsePacket.push_back(AccountLength + 5); // 消息类型
    // 取从第4个字节开始，到第(4 + AccountLength)个字节的数据
    responsePacket.insert(responsePacket.end(), message.begin() + 4, message.begin() + 8 + AccountLength);
    responsePacket.insert(responsePacket.end(), {1, 170, 85});
    // 获取账号数据
    std::string accountText(message.begin() + 8, message.begin() + 8 + AccountLength);
    // 打印账号数据
    writeLog("账号[" + accountText + "]离开游戏");
    //更新账号状态
    BilingLeave(accountText, 0);    //更新账号状态为离线
    // 返回构建好的响应包
    return responsePacket;
}

// 166封包数据
std::vector<uint8_t> Account_InMessage166(const std::vector<uint8_t> &message)
{
    // 构建响应包
    std::vector<uint8_t> responsePacket = {85, 170, 0};
    int AccountLength = message[7]; // 从消息中获取账号长度字段
    responsePacket.push_back(AccountLength + 17); // 消息类型
    // 取从第4个字节开始，到第(4 + AccountLength)个字节的数据
    responsePacket.insert(responsePacket.end(), message.begin() + 4, message.begin() + 8 + AccountLength);
    // 添加额外的固定字节
    responsePacket.insert(responsePacket.end(), {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 170, 85});
    return responsePacket;
}

// 点数查询数据
std::vector<uint8_t> Account_InInquire(const std::vector<uint8_t> &message)
{
    // 构建响应包
    std::vector<uint8_t> responsePacket = {85, 170, 0};
    int AccountLength = message[7]; // 从消息中获取账号长度字段
    responsePacket.push_back(AccountLength + 8); // 消息类型
    // 取从第4个字节开始，到第(4 + AccountLength)个字节的数据
    responsePacket.insert(responsePacket.end(), message.begin() + 4, message.begin() + 8 + AccountLength);
    // 获取账号数据
    std::string accountText(message.begin() + 8, message.begin() + 8 + AccountLength);
    //取出账号点数
    int point = BilingGetHumanPoint(accountText);
    // 打印账号数据
    writeLog("账号[" + accountText + "]查询点数，点数: " + std::to_string(point));
    // 将点数转换为字节数组
    std::vector<uint8_t> pointbytes = {
            static_cast<uint8_t>((point >> 24) & 0xFF),
            static_cast<uint8_t>((point >> 16) & 0xFF),
            static_cast<uint8_t>((point >> 8) & 0xFF),
            static_cast<uint8_t>(point & 0xFF)};
    responsePacket.insert(responsePacket.end(), pointbytes.begin(), pointbytes.end());
    // 添加额外的固定字节
    responsePacket.insert(responsePacket.end(), { 170, 85});
    return responsePacket;
}
// 点数扣除数据
std::vector<uint8_t> Account_InDeduct(const std::vector<uint8_t> &message)
{
    // 构建响应包
    std::vector<uint8_t> responsePacket = {85, 170, 0};
    int AccountLength = message[7]; // 从消息中获取账号长度字段
    responsePacket.push_back(AccountLength + 26); // 消息类型
    // 取从第4个字节开始，到第(4 + AccountLength)个字节的数据
    responsePacket.insert(responsePacket.end(), message.begin() + 4, message.begin() + 8 + AccountLength);
    responsePacket.insert(responsePacket.end(), message.end() - 37, message.end() - 15);

    std::string accountText(message.begin() + 8, message.begin() + 8 + AccountLength);
    std::vector<uint8_t> AccountPoint(message.end() -6, message.end() - 2);
    uint32_t value = (AccountPoint[0] << 24) | (AccountPoint[1] << 16) | (AccountPoint[2] << 8) | AccountPoint[3];

    //取出账号点数
    int point = BilingGetHumanPoint(accountText);
    if (point >= (int)value)
    {
        // 扣除点数
        int point_value = point-value;
        BilingSetHumanPoint(accountText, value, point_value);
        std::vector<uint8_t> bytes = {
            static_cast<uint8_t>((point_value >> 24) & 0xFF),
            static_cast<uint8_t>((point_value >> 16) & 0xFF),
            static_cast<uint8_t>((point_value >> 8) & 0xFF),
            static_cast<uint8_t>(point_value & 0xFF)};
        responsePacket.insert(responsePacket.end(), bytes.begin(), bytes.end());
        responsePacket.insert(responsePacket.end(), {0,1});
        responsePacket.insert(responsePacket.end(), message.begin() + 23, message.begin() + 27);
        responsePacket.insert(responsePacket.end(), AccountPoint.begin(), AccountPoint.end());
        responsePacket.insert(responsePacket.end(), { 170, 85});
        writeLog("账号[" + accountText + "]兑换点数成功，兑换点数: " + std::to_string(value));
        responsePacket.push_back(1); // 成功标志
    }
    else
    {
        writeLog("账号[" + accountText + "]扣除点数失败，点数不足");
        responsePacket.insert(responsePacket.end(), {2, 170, 85});
    }
    
    return responsePacket;
}

// 228封包数据
std::vector<uint8_t> Account_InMessage228(const std::vector<uint8_t> &message)
{
    // 构建响应包
    std::vector<uint8_t> responsePacket = {85, 170, 0};
    int AccountLength = message[8]; // 从消息中获取账号长度字段
    responsePacket.push_back(AccountLength + 18); // 消息类型
    // 取从第4个字节开始，到第7个字节的数据
    responsePacket.insert(responsePacket.end(), message.begin() + 4, message.begin() + 7);
    responsePacket.push_back(message[7]);     // 消息类型
    responsePacket.push_back(message[7] + 1); // 消息类型
    responsePacket.insert(responsePacket.end(), message.begin() + 8, message.begin() + 9 + AccountLength);
    // 添加额外的固定字节
    responsePacket.insert(responsePacket.end(), {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 170, 85});
    return responsePacket;
}
// 修改回调函数以处理字节数组
void onMessageReceived(int clientSocket, const std::vector<uint8_t> &message)
{
    // 处理收到的字节数组
    // std::cout << "接受到的包 " << clientSocket << ": ";
    // printByteArray(message);
    if (message.size() >= 4 && message[4] == 160)
    {
        // 构建响应包
        std::vector<uint8_t> responsePacket = {85, 170, 0, 5, 160};
        responsePacket.insert(responsePacket.end(), message.begin() + 5, message.begin() + 7); // 从第5个字节开始取2个
        responsePacket.push_back(message[6]);                                                  // 从第6个字节开始取1个
        responsePacket.insert(responsePacket.end(), {0, 170, 85});
        // std::cout << "拼接好的包160 " << clientSocket << ": ";
        // printByteArray(responsePacket);
        // 发送响应包
        send(clientSocket, responsePacket.data(), responsePacket.size(), 0);
    }
    else if (message.size() >= 4 && message[4] == 161)
    {
        // 构建响应包
        std::vector<uint8_t> responsePacket = {85, 170, 0, 5, 161};
        responsePacket.insert(responsePacket.end(), message.begin() + 5, message.begin() + 7); // 从第5个字节开始取2个
        responsePacket.insert(responsePacket.end(), {1, 0, 170, 85});
        // std::cout << "拼接好的包161 " << clientSocket << ": ";
        // printByteArray(responsePacket);
        // 发送响应包
        send(clientSocket, responsePacket.data(), responsePacket.size(), 0);
    }
    else if (message.size() >= 4 && message[4] == 162)
    {
        // 调用 Account_InGame 函数处理封包
        std::vector<uint8_t> processedPacket = Account_InLogin(message);
        // std::cout << "处理后的包162 " << clientSocket << ": ";
        // printByteArray(processedPacket);
        // 发送处理后的封包
        send(clientSocket, processedPacket.data(), processedPacket.size(), 0);
    }
    else if (message.size() >= 4 && message[4] == 163)
    {
        // 调用 Account_InGame 函数处理封包
        std::vector<uint8_t> processedPacket = Account_InGame(message);
        // std::cout << "处理后的包163 " << clientSocket << ": ";
        // printByteArray(processedPacket);
        // 发送处理后的封包
        send(clientSocket, processedPacket.data(), processedPacket.size(), 0);
    }
    else if (message.size() >= 4 && message[4] == 164)
    {
        // 调用 Account_InLeave 函数处理封包
        std::vector<uint8_t> processedPacket = Account_InLeave(message);
        // std::cout << "处理后的包164 " << clientSocket << ": ";
        // printByteArray(processedPacket);
        // 发送处理后的封包
        send(clientSocket, processedPacket.data(), processedPacket.size(), 0);
    }
    else if (message.size() >= 4 && message[4] == 166)
    {
        // 调用 Account_InMessage166 函数处理封包
        std::vector<uint8_t> processedPacket = Account_InMessage166(message);
        // std::cout << "处理后的包166 " << clientSocket << ": ";
        // printByteArray(processedPacket);
        // 发送处理后的封包
        send(clientSocket, processedPacket.data(), processedPacket.size(), 0);
    }
    else if (message.size() >= 4 && message[4] == 228)
    {
        // 调用 Account_InMessage228 函数处理封包
        std::vector<uint8_t> processedPacket = Account_InMessage228(message);
        // std::cout << "处理后的包228 " << clientSocket << ": ";
        // printByteArray(processedPacket);
        // 发送处理后的封包
        send(clientSocket, processedPacket.data(), processedPacket.size(), 0);
    }
    else if (message.size() >= 4 && message[4] == 226)
    {
        // 调用 Account_InInquire 函数处理封包
        std::vector<uint8_t> processedPacket = Account_InInquire(message);
        // std::cout << "处理后的包229 " << clientSocket << ": ";
        // printByteArray(processedPacket);
        // 发送处理后的封包
        send(clientSocket, processedPacket.data(), processedPacket.size(), 0);
    }
    else if (message.size() >= 4 && message[4] == 225)
    {
        // 调用 Account_InDeduct 函数处理封包
        std::vector<uint8_t> processedPacket = Account_InDeduct(message);
        // std::cout << "处理后的包230 " << clientSocket << ": ";
        // printByteArray(processedPacket);
        // 发送处理
        send(clientSocket, processedPacket.data(), processedPacket.size(), 0);
    }
}

// 日志写入函数
void writeLog(const std::string& message)
{
    std::ofstream logFile("biling.log", std::ios::app); // 以追加模式打开文件
    if (logFile.is_open())
    {
        // 获取当前时间
        std::time_t now = std::time(nullptr);
        char timeStr[100];
        std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        
        // 写入时间戳和消息
        logFile << "[" << timeStr << "] " << message << std::endl;\
       // 同时输出到控制台 - 使用printf的正确方式
        printf("[%s] %s\n", timeStr, message.c_str());
        logFile.close();
    }
    else
    {
        std::cerr << "无法打开日志文件 biling.log" << std::endl;
    }
}

// 简化的ANSI(GBK)转UTF-8函数
std::string ansiToUtf8(const std::string& ansiStr) {
    iconv_t cd = iconv_open("UTF-8", "GBK");
    if (cd == (iconv_t)-1) {
        return ansiStr; // 转换失败，返回原字符串
    }
    
    size_t inLen = ansiStr.length();
    size_t outLen = inLen * 4;
    
    char* inBuf = const_cast<char*>(ansiStr.c_str());
    char* outBuf = new char[outLen];
    char* outPtr = outBuf;
    
    size_t result = iconv(cd, &inBuf, &inLen, &outPtr, &outLen);
    
    std::string utf8Str;
    if (result != (size_t)-1) {
        utf8Str = std::string(outBuf, outPtr - outBuf);
    } else {
        utf8Str = ansiStr; // 转换失败，返回原字符串
    }
    
    delete[] outBuf;
    iconv_close(cd);
    return utf8Str;
}