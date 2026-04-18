# FilediskServer

基于 C++ 开发的高性能文件存储服务器，支持 TCP 和 HTTP 双协议访问。底层依赖 `epoll` 实现高并发 I/O 多路复用。

---

## 目录
- [项目简介](#项目简介)
- [环境依赖](#环境依赖)
- [安装步骤](#安装步骤)
- [数据库配置](#数据库配置)
- [中间件部署](#中间件部署)
- [项目结构](#项目结构)
- [主要功能与流程](#主要功能与流程)
- [HTTP RESTful API](#http-restful-api)
- [SRPC 框架说明](#srpc-框架说明)

---

## 项目简介
FilediskServer 是一个文件磁盘服务，提供文件的上传、下载、目录管理及用户认证功能。项目结合了 Workflow 网络框架和 SRPC 微服务架构，支持 RESTful 接口与 RPC 调用。

---

## 环境依赖
- **操作系统**: Linux (支持 epoll)
- **编译器**: 支持 C++11 及以上标准的编译器 (g++/clang++)
- **构建工具**: CMake (3.0+)
- **外部库**:
    - [Workflow](https://github.com/sogou/workflow)
    - [Wfrest](https://github.com/wfrest/wfrest)
    - [SRPC](https://github.com/sogou/srpc)
    - Protobuf
    - MySQL (开发库)
    - RabbitMQ & Consul (可选，用于服务发现与消息队列)

---

## 安装步骤

### 1. 克隆项目代码
```bash
git clone https://github.com/yourusername/FilediskServer.git
cd FilediskServer
```

### 2. 安装 Workflow
Workflow 是底层网络框架核心。
```bash
git clone https://kkgithub.com/sogou/workflow.git
cd workflow
mkdir build && cd build
cmake ..
make -j4
sudo make install
cd ../..
```

### 3. 安装 Wfrest
Wfrest 用于提供 HTTP RESTful 服务支持。
```bash
git clone https://kkgithub.com/wfrest/wfrest.git
cd wfrest
mkdir build && cd build
cmake ..
make -j4
sudo make install
cd ../..
```

### 4. 安装 Protobuf
确保系统已安装 Protobuf 开发库。
```bash
# Ubuntu/Debian
sudo apt-get install libprotobuf-dev protobuf-compiler

# CentOS/RHEL
sudo yum install protobuf-devel
```

### 5. 安装 SRPC
SRPC 用于微服务 RPC 调用。
```bash
git clone https://github.com/sogou/srpc.git
cd srpc
mkdir build && cd build
cmake ..
make -j4
sudo make install
cd ../..
```

### 6. 编译本项目
```bash
mkdir build && cd build
cmake ..
make -j4
```

---

## 数据库配置
项目依赖 MySQL 数据库，请预先创建数据库并执行以下表结构初始化脚本。

### 1. 用户表
用于存储用户注册信息及认证凭据。
```sql
CREATE TABLE user (
    id INT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(50) NOT NULL UNIQUE,
    salt VARCHAR(16),
    encpwd VARCHAR(64)
);
```

### 2. 文件树表
用于存储文件及目录的元数据。
- `filesize=0` 表示该记录是一个目录。
- `hashfilepath` 默认值为空字符串，`filesize` 默认值为 0。

```sql
CREATE TABLE filetree (
    id INT PRIMARY KEY AUTO_INCREMENT,
    userid INT,
    filepath VARCHAR(255),
    `index` INT,
    hashfilepath VARCHAR(255) DEFAULT "",
    dirid INT,
    filesize BIGINT DEFAULT 0
);

-- 建立索引以加速查询
CREATE INDEX fileindex ON filetree(userid, dirid);
```

---

## 中间件部署

### RabbitMQ (消息队列)
用于异步任务处理或解耦。
```bash
docker run -d \
  --name rabbitmq \
  -p 5672:5672 \
  -p 15672:15672 \
  -v $PWD/rabbitmq_data:/var/lib/rabbitmq \
  rabbitmq:management
```
- 访问地址: `http://localhost:15672` (默认账号 guest/guest)

### Consul (服务发现)
**注意**: Consul 1.16+ 版本官方镜像已迁移至 HashiCorp 仓库。
```bash
docker run -d \
  --name=consul \
  -p 8500:8500 \
  -p 8600:8600/udp \
  hashicorp/consul agent -dev -client=0.0.0.0
```
- 访问地址: `http://localhost:8500`

---

## 项目结构
```text
FilediskServer/
├── CMakeLists.txt          # 项目构建配置
├── proto/                  # Protobuf 定义文件
│   └── FileManager.proto   # RPC 接口定义
├── src/                    # 源代码目录
│   ├── main.cpp            # 程序入口
│   ├── server/             # 服务器核心逻辑
│   ├── handlers/           # HTTP/TCP 请求处理函数
│   └── dao/                # 数据库访问层
├── build/                  # 编译输出目录
└── README.md               # 本说明文档
```

---

## 主要功能与流程

### TCP 协议部分
主要用于高性能文件传输。

#### 1. 文件上传流程
1. **用户校验**: `checkuservalidation` 验证用户身份。
2. **存在性检查**: `checkfileexits` 检查文件是否已存在（秒传逻辑）。
3. **文件校验**: `checkfileok` 校验文件完整性并写入磁盘。

#### 2. 文件下载流程
- **顺序下载**: 支持按顺序流式下载文件内容。

---

## HTTP RESTful API
基于 Wfrest 实现的 HTTP 接口，主要用于元数据操作和用户管理。

### 1. 用户注册 `/register`
- **方法**: POST
- **说明**: 注册新用户，写入 `user` 表。
- **注意**: 在操作 MySQL 时，可通过 `cursor.get_cursor_status()` 检查状态（0 表示 `MYSQL_STATUS_NOT_INIT`）。

### 2. 用户登录 `/signin`
- **方法**: POST
- **请求参数**:
  ```json
  {
    "username": "string",
    "password": "string"
  }
  ```
- **响应结构**:
  ```json
  {
    "code": 0,
    "jsoninfo": "string",
    "dirid": 0,
    "uuid": "string",
    "name": "string"
  }
  ```

### 3. 查询目录 `/querydir`
- **方法**: GET/POST
- **说明**: 查询指定 `dirid` 下的文件和子目录列表。

### 4. 删除目录 `/removedir`
- **方法**: POST
- **说明**: 删除指定的目录或文件记录。

### 5. 新增目录 `/adddir`
- **方法**: POST
- **说明**: 在指定父目录下创建新目录。

---

## SRPC 框架说明
本项目支持基于 SRPC 的微服务调用。

### 代码生成
若修改了 `.proto` 文件，需重新生成 C++ 代码：
```bash
srpc_generator protobuf FileManager.proto .
```

### Proto 文件格式示例
确保 Proto 文件符合标准格式：
```protobuf
syntax = "proto3";

package FileManager;

service FileManagerService {
    // 定义 RPC 方法
}
```
