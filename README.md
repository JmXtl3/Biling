# TLBB-AccountServer

游戏天龙八部账号验证服务器 - C++实现版本

## 项目概述

TLBB-AccountServer 是一个高性能的游戏账号验证服务端，专门用于处理游戏内账号的注册、登录、点数查询、点数扣除、状态管理等操作。支持MySQL数据库存储，提供TCP网络服务接口，具备自动注册、在线状态检测等功能。

## 功能特性

- ✅ **数据库连接管理**: 稳定连接MySQL数据库，支持自动重连
- ✅ **网络服务**: 监听指定端口，处理客户端TCP请求
- ✅ **账号管理**: 账号注册、登录、点数查询与扣除、状态变更
- ✅ **自动注册**: 支持账号不存在时自动注册
- ✅ **在线状态检测**: 支持账号在线/封禁状态校验
- ✅ **日志记录**: 详细记录操作日志，便于审计和排查
- ✅ **多线程处理**: 支持多客户端并发请求

## 系统要求

### 操作系统
- Linux (Ubuntu 18.04+, CentOS 7+)

### 依赖库
- MySQL Client Development Libraries
- C++11 支持的编译器 (GCC 4.9+)

### 安装依赖 (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install build-essential
sudo apt-get install libmysqlclient-dev
sudo apt-get install pkg-config
```

### 安装依赖 (CentOS/RHEL)
```bash
sudo yum install gcc-c++ make
sudo yum install mysql-devel
sudo yum install pkgconfig
```

## 编译和安装

### 1. 克隆或下载源码
```bash
# 假设源码已在当前目录
ls -la
# 应该看到: billing.cpp, biling.h, config.ini, build.sh
```

### 2. 设置构建脚本权限
```bash
chmod +x build.sh
```

### 3. 编译项目
```bash
./build.sh
```

### 4. 验证编译结果
```bash
ls -la biling
# 应该看到可执行文件 biling
```

## 配置文件

编辑 `config.ini` 文件来配置服务器参数:

```ini
[mysql]
host = 192.168.2.150
port = 3306
user = root
password = rumeng
db = tlbb_db

[server]
bind_ip = 0.0.0.0
port = 9470
app_title = 111
auto_register = true
```

## 运行服务器

### 启动服务
```bash
./biling
```

## 协议说明

服务器监听配置的端口，接收客户端二进制封包，支持如下操作：
- 账号注册与自动注册
- 账号登录验证
- 点数查询与扣除
- 账号在线/封禁状态校验

## 数据库表结构示例

### account (账号表)
```sql
CREATE TABLE account (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(64) NOT NULL,
    password VARCHAR(64) NOT NULL,
    point INT DEFAULT 0,
    status INT DEFAULT 0, -- 0正常，1离线
    tel VARCHAR(32),
    ...
);
```

## 日志和监控

服务器运行时会输出详细的日志信息:
- 服务启动/停止状态
- 数据库连接状态
- 账号注册、登录、点数操作
- 错误和警告信息

## 故障排除

- 检查MySQL服务是否启动
- 验证数据库连接参数
- 确认端口没有被占用
- 检查防火墙设置

## 版本信息

- 版本: 1.0
- 开发语言: C++11
- 支持平台: Linux
- 开发者: 菁梦

## 许可证

请根据项目需求添加适当的许可证信息。
