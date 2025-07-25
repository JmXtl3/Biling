#include "biling.h"

// 全局MySQL连接句柄定义
MYSQL *g_mysql_connection = nullptr;

// 新增：校验账号是否存在，不存在则新增
bool CheckOrCreateAccount(const std::string &accountText, const std::string &passwordText)
{
    if (!g_mysql_connection)
    {
        writeLog("MySQL连接未建立，无法校验或新增账号");
        return false;
    }
    // 查询账号是否存在
    std::string query = "SELECT COUNT(*) FROM account WHERE name = '" + accountText + "'";
    if (mysql_query(g_mysql_connection, query.c_str()) != 0)
    {
        writeLog("SQL查询失败: " + std::string(mysql_error(g_mysql_connection)));
        writeLog("SQL语句: " + query);
        return false;
    }
    MYSQL_RES *result = mysql_store_result(g_mysql_connection);
    if (!result)
    {
        writeLog("获取查询结果失败: " + std::string(mysql_error(g_mysql_connection)));
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(result);
    bool exists = (row && row[0] && std::atoi(row[0]) > 0);
    mysql_free_result(result);

    // 如果不存在则插入新账号
    if (!exists && g_config.auto_register)
    {
        std::string pwdLower = passwordText;
        std::transform(pwdLower.begin(), pwdLower.end(), pwdLower.begin(), ::tolower);
        std::string insertQuery = "INSERT INTO account (name, password, email, question) VALUES ('" +
                                  accountText + "', '" + pwdLower + "', '1@qq.com', '" + pwdLower + "')";
        if (mysql_query(g_mysql_connection, insertQuery.c_str()) != 0)
        {
            writeLog("新增账号失败: " + std::string(mysql_error(g_mysql_connection)));
            writeLog("SQL语句: " + insertQuery);
            return false;
        }
        writeLog("账号[" + accountText + "]不存在，已自动新增");
    }
    return true;
}
void BilingLeave(std::string accountText , int accountstate )
{
    if (!g_mysql_connection)
    {
        writeLog("MySQL连接未建立，无法进行登录验证");
        return;
    }
    // 更新登录状态
    std::string updateQuery = "UPDATE account SET tel = '" + std::to_string(accountstate) + "' WHERE name = '" + accountText + "'";
    if (mysql_query(g_mysql_connection, updateQuery.c_str()) != 0)
    {
        writeLog("tel字段更新失败: " + std::string(mysql_error(g_mysql_connection)));
        writeLog("SQL语句: " + updateQuery);
    }

}
int BilingLogin(std::string accountText, std::string passwordText)
{
    CheckOrCreateAccount(accountText, passwordText);
    if (!g_mysql_connection)
    {
        writeLog("MySQL连接未建立，无法进行登录验证");
        return 0;
    }
    // 构建SQL查询语句
    std::string query = "SELECT * FROM account WHERE name = '" + accountText +
                        "' AND password = '" + passwordText + "'";
    // 执行查询
    if (mysql_query(g_mysql_connection, query.c_str()) != 0)
    {
        writeLog("SQL查询失败: " + std::string(mysql_error(g_mysql_connection)));
        writeLog("SQL语句: " + query);
        return 0;
    }
    // 获取查询结果
    MYSQL_RES *result = mysql_store_result(g_mysql_connection);
    if (!result)
    {
        writeLog("获取查询结果失败: " + std::string(mysql_error(g_mysql_connection)));
        return 0;
    }
    // 检查是否有查询结果
    if (mysql_num_rows(result) == 0)
    {
        writeLog("登录失败: 未找到账号或密码错误");
        mysql_free_result(result);
        return 0;
    }
    // 获取第一行数据
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row)
    {
        writeLog("登录失败: 获取账号信息失败");
        mysql_free_result(result);
        return 0;
    }
    // 检查账号状态
    int accountstate = 0;
    if (row[7])
    {
        accountstate = std::atoi(row[7]);
    }
    else
    {
        // 字段为空，按需处理
        writeLog("警告: 账号状态字段为空，默认设为0");
        accountstate = 0; // 或者其它默认值
    }
    if (accountstate == 1)
    {
        writeLog("登录失败: 账号在线中");
        BilingLeave(accountText, 0); // 更新账号状态为离线
        mysql_free_result(result);
        return 1;
    }
    // 登录成功
    writeLog("账号 " + accountText + " 登录成功");
    mysql_free_result(result);

    
    return 2;
}
// 获取账号点数
int BilingGetHumanPoint(std::string accountText)
{
    if (!g_mysql_connection)
    {
        writeLog("MySQL连接未建立，无法查询账号点数");
        return 0;
    }
    // 构建SQL查询语句
    std::string query = "SELECT point FROM account WHERE name = '" + accountText + "'";
    // 执行查询
    if (mysql_query(g_mysql_connection, query.c_str()) != 0)
    {
        writeLog("SQL查询失败: " + std::string(mysql_error(g_mysql_connection)));
        writeLog("SQL语句: " + query);
        return 0;
    }
    // 获取查询结果
    MYSQL_RES *result = mysql_store_result(g_mysql_connection);
    if (!result)
    {
        writeLog("获取查询结果失败: " + std::string(mysql_error(g_mysql_connection)));
        return 0;
    }
    // 检查是否有查询结果
    if (mysql_num_rows(result) == 0)
    {
        writeLog("未找到账号: " + accountText);
        mysql_free_result(result);
        return 0;
    }

    // 获取第一行数据
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row || !row[0])
    {
        writeLog("账号 " + accountText + " 的point字段为空");
        mysql_free_result(result);
        return 0;
    }
    // 获取point字段的值
    int pointValue = 0;
        if (row[0])
    {
        pointValue = std::atoi(row[0]);
    }
    else
    {
        // 字段为空，按需处理
        writeLog("警告: 点数字段为空，默认设为0");
        pointValue = 0; // 或者其它默认值
    }
    writeLog("账号 " + accountText + " 查询点数剩余: " + row[0]);
    return pointValue;
}

bool BilingSetHumanPoint(std::string accountText, int point, int Newpoint)
{
    if (!g_mysql_connection)
    {
        writeLog("MySQL连接未建立，无法设置账号点数");
        return 0;
    }
    // 构建SQL更新语句
    std::string query = "UPDATE account SET point = " + std::to_string(Newpoint) + " WHERE name = '" + accountText + "'";
    // 执行更新
    if (mysql_query(g_mysql_connection, query.c_str()) != 0)
    {
        writeLog("SQL更新失败: " + std::string(mysql_error(g_mysql_connection)));
        writeLog("SQL语句: " + query);
        return 0;
    }
    writeLog("账号 " + accountText + " 兑换点数:" + std::to_string(point) + "  剩余点数: " + std::to_string(Newpoint));
    return 1;
}
// 连接MySQL数据库
bool connectMySQL()
{
    // 初始化MySQL连接
    g_mysql_connection = mysql_init(nullptr);
    if (!g_mysql_connection)
    {
        writeLog("MySQL初始化失败");
        return false;
    }

    // 连接到MySQL服务器
    if (!mysql_real_connect(g_mysql_connection,
                            g_config.mysql_host.c_str(),
                            g_config.mysql_user.c_str(),
                            g_config.mysql_password.c_str(),
                            g_config.mysql_db.c_str(),
                            g_config.mysql_port,
                            nullptr, 0))
    {
        writeLog("MySQL连接失败: " + std::string(mysql_error(g_mysql_connection)));
        mysql_close(g_mysql_connection);
        g_mysql_connection = nullptr;
        return false;
    }

    // 设置字符集为UTF-8
    // if (mysql_set_character_set(g_mysql_connection, "utf8") != 0) {
    //    writeLog("设置MySQL字符集失败: " + std::string(mysql_error(g_mysql_connection)));
    //}

    writeLog("MySQL连接成功: " + g_config.mysql_host + ":" +
             std::to_string(g_config.mysql_port));

    return true;
}

// 断开MySQL连接
void disconnectMySQL()
{
    if (g_mysql_connection)
    {
        mysql_close(g_mysql_connection);
        g_mysql_connection = nullptr;
        writeLog("MySQL连接已关闭");
    }
}

// 检查连接状态
bool isConnected()
{
    return g_mysql_connection && mysql_ping(g_mysql_connection) == 0;
}
