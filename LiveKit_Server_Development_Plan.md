# LiveKit 服务端开发计划

## 1. 项目结构

```
LiveKit/
├── server/
│   ├── CMakeLists.txt
│   ├── src/
│   │   ├── main.cpp                        # 入口，启动服务器
│   │   ├── core/
│   │   │   ├── Server.h/cpp                # 服务器主类，组合各模块
│   │   │   ├── EventLoop.h/cpp             # epoll 事件循环
│   │   │   ├── EpollWrapper.h/cpp          # epoll API 封装
│   │   │   ├── ThreadPool.h/cpp            # 线程池
│   │   │   ├── TaskQueue.h/cpp             # 任务队列（线程安全）
│   │   │   ├── Timer.h/cpp                 # 定时器（基于 epoll timerfd）
│   │   │   └── Logger.h/cpp                # 日志系统
│   │   ├── network/
│   │   │   ├── Connection.h/cpp            # TCP 连接封装
│   │   │   ├── Buffer.h/cpp                # 读写缓冲区
│   │   │   ├── HttpRequest.h/cpp           # HTTP 请求解析
│   │   │   ├── HttpResponse.h/cpp          # HTTP 响应构建
│   │   │   ├── HttpServer.h/cpp            # HTTP 服务器
│   │   │   ├── WebSocketHandler.h/cpp      # WebSocket 协议处理
│   │   │   ├── WebSocketFrame.h/cpp        # WebSocket 帧解析/构建
│   │   │   ├── Router.h/cpp                # HTTP 路由表
│   │   │   └── StaticFileHandler.h/cpp     # 静态文件服务
│   │   ├── business/
│   │   │   ├── UserService.h/cpp           # 用户业务逻辑
│   │   │   ├── RoomService.h/cpp           # 直播间业务逻辑
│   │   │   ├── GiftService.h/cpp           # 礼物业务逻辑
│   │   │   ├── ReplayService.h/cpp         # 回放业务逻辑
│   │   │   └── RoomManager.h/cpp           # 房间连接管理（WebSocket房间）
│   │   ├── database/
│   │   │   ├── Database.h/cpp              # SQLite 数据库封装
│   │   │   ├── UserDao.h/cpp               # 用户数据访问
│   │   │   ├── RoomDao.h/cpp               # 直播间数据访问
│   │   │   ├── GiftDao.h/cpp               # 礼物数据访问
│   │   │   └── ReplayDao.h/cpp             # 回放数据访问
│   │   ├── recording/
│   │   │   ├── RecordingManager.h/cpp      # 录制管理器
│   │   │   └── FFmpegRecorder.h/cpp        # FFmpeg 录制进程管理
│   │   └── utils/
│   │       ├── Crypto.h/cpp                # SHA-256 哈希
│   │       ├── TokenGenerator.h/cpp        # Token 生成与验证
│   │       ├── JsonHelper.h/cpp            # JSON 辅助工具
│   │       ├── FileUtils.h/cpp             # 文件操作工具
│   │       └── Config.h/cpp                # 配置文件读取
│   ├── config/
│   │   └── server.conf                     # 服务器配置文件
│   ├── data/
│   │   └── livekit.db                      # SQLite 数据库文件（运行时生成）
│   ├── static/
│   │   ├── avatars/                        # 预设头像文件
│   │   ├── covers/                         # 直播封面截图（运行时生成）
│   │   └── recordings/                     # 录制文件（运行时生成）
│   ├── scripts/
│   │   └── init_db.sql                     # 数据库初始化 SQL
│   └── third_party/
│       ├── sqlite3/                        # SQLite3 源码（amalgamation）
│       └── nlohmann/                       # JSON 库（header-only）
└── client/                                 # 客户端（另一个 trae 窗口开发）
```

---

## 2. 迭代开发计划

### v0.1 - 项目骨架与网络框架

**目标**：搭建 CMake 项目，实现 epoll 事件循环、线程池、基础 HTTP 服务器

**开发内容**：

1. **CMake 项目搭建**
   - 创建 CMakeLists.txt
   - 配置 C++14 标准
   - 引入 SQLite3 amalgamation 源码
   - 引入 nlohmann/json header-only
   - 建立目录结构

2. **Logger 日志系统**
   - 多级别日志（DEBUG/INFO/WARN/ERROR）
   - 输出到控制台 + 文件
   - 线程安全
   - 日志格式：[时间] [级别] [文件:行号] 消息

3. **EpollWrapper epoll 封装**
   - epoll_create / epoll_ctl / epoll_wait 封装
   - 支持 EPOLLIN / EPOLLOUT / EPOLLET
   - RAII 管理 epoll fd

4. **EventLoop 事件循环**
   - 基于 epoll 的事件循环
   - 支持 accept 新连接
   - 支持读写事件分发
   - 支持 timerfd 定时器

5. **Buffer 读写缓冲区**
   - 自动扩容的字节缓冲区
   - 支持读取行（HTTP 请求解析需要）
   - 线程安全

6. **Connection 连接封装**
   - 封装 socket fd
   - 管理读写缓冲区
   - 回调机制（onMessage, onClose, onError）

7. **ThreadPool 线程池**
   - 固定大小线程池
   - TaskQueue 任务队列（互斥锁 + 条件变量）
   - submit() 提交任务，返回 future

8. **HttpServer HTTP 服务器**
   - 基于 EventLoop + ThreadPool
   - 监听指定端口
   - accept 新连接并注册到 epoll
   - 解析 HTTP 请求行和头部
   - 分发到工作线程处理

9. **Router 路由表**
   - 注册路由处理函数
   - 支持 GET / POST 方法
   - 路径匹配

10. **HttpResponse 响应构建**
    - 状态行 + 头部 + 正文
    - JSON 响应快捷方法

11. **Config 配置读取**
    - 读取 server.conf 配置文件
    - 支持键值对格式
    - 配置项：port, thread_count, db_path, recording_path 等

**验收标准**：
- [x] CMake 编译通过
- [x] 服务器启动监听端口
- [x] curl 访问返回 404 或默认响应
- [x] 日志正常输出到控制台和文件

**GitHub 提交信息**：
```
feat: 搭建服务端项目骨架，实现 epoll 事件循环、线程池和 HTTP 服务器
```

---

### v0.2 - 用户注册与登录

**目标**：实现用户系统，包括数据库、密码加密、Token 管理

**开发内容**：

1. **Database 数据库封装**
   - SQLite3 C API 封装
   - 连接管理
   - 执行 SQL 语句
   - 查询结果回调
   - 预处理语句支持（防 SQL 注入）
   - 数据库初始化（执行 init_db.sql）

2. **init_db.sql 数据库初始化**
   - 创建 users 表
   - 创建 tokens 表
   - 创建 gifts 表（插入预设礼物数据）

3. **UserDao 用户数据访问**
   - createUser(username, password_hash, salt, avatar_id)
   - findUserByUsername(username)
   - findUserById(id)
   - updateLastLogin(id)

4. **Crypto 密码加密**
   - SHA-256 哈希实现
   - 随机盐值生成
   - 密码 + 盐值哈希

5. **TokenGenerator Token 管理**
   - 生成 token（用户ID + 时间戳 + 随机数 → SHA-256）
   - 验证 token 有效性
   - token 过期检查（24小时）

6. **UserService 用户业务逻辑**
   - registerUser(username, password, avatar_id)
     - 校验用户名唯一性
     - 生成盐值和密码哈希
     - 创建用户记录
     - 生成 token
   - loginUser(username, password)
     - 查找用户
     - 验证密码
     - 生成 token
     - 更新最后登录时间
   - verifyToken(token)
     - 查找 token 记录
     - 检查过期时间

7. **HTTP API 路由注册**
   - POST /api/register → UserService::registerUser
   - POST /api/login → UserService::loginUser
   - GET /api/avatars → 返回预设头像列表

8. **HttpRequest 完善**
   - 解析请求体（JSON）
   - 解析查询参数

**验收标准**：
- [x] curl POST /api/register 注册成功
- [x] curl POST /api/login 登录成功返回 token
- [x] 重复用户名注册失败
- [x] 错误密码登录失败
- [x] 数据库文件正确创建

**GitHub 提交信息**：
```
feat: 实现用户注册登录 API，SQLite 数据库和 Token 鉴权
```

---

### v0.3 - 直播间管理

**目标**：实现直播间的创建、列表查询、结束等功能

**开发内容**：

1. **RoomDao 直播间数据访问**
   - createRoom(anchor_id, title, category, mode, stream_key)
   - findRoomById(room_id)
   - findRoomsByStatus(status)
   - findRoomsByCategory(category, status)
   - updateRoomStatus(room_id, status)
   - updateViewerCount(room_id, count)
   - findRoomsByAnchorId(anchor_id)

2. **RoomService 直播间业务逻辑**
   - createRoom(token, title, category, mode)
     - 验证 token
     - 生成唯一 stream_key（随机字符串）
     - 创建房间记录
     - 返回 room_id, stream_key, push_url
   - getRoomList(category)
     - 查询所有正在直播的房间
     - 按分类筛选
     - 组装房间信息（含主播信息）
   - getRoomInfo(room_id)
     - 查询房间详情
     - 查询在线观众列表
   - endRoom(token, room_id)
     - 验证 token 和主播身份
     - 更新房间状态为 ended
     - 通知房间内观众（后续 WebSocket 实现）
   - updateViewerCount(room_id, count)
     - 更新在线观众人数

3. **HTTP API 路由注册**
   - POST /api/live/create → RoomService::createRoom
   - GET /api/live/rooms → RoomService::getRoomList
   - GET /api/live/room/{room_id} → RoomService::getRoomInfo
   - POST /api/live/end → RoomService::endRoom

4. **stream_key 生成**
   - 随机 16 字符字符串（字母+数字）
   - 确保唯一性

5. **Nginx-RTMP 配置**
   - 配置 RTMP 应用 live
   - 允许推流和拉流
   - 配置 HTTP-FLV 可选

**验收标准**：
- [x] curl POST /api/live/create 创建直播间成功
- [x] curl GET /api/live/rooms 返回直播间列表
- [x] curl GET /api/live/room/{id} 返回直播间详情
- [x] curl POST /api/live/end 结束直播间成功
- [x] Nginx-RTMP 可接收推流

**GitHub 提交信息**：
```
feat: 实现直播间管理 API，支持创建/查询/结束直播间
```

---

### v0.4 - WebSocket 实时通信

**目标**：实现 WebSocket 服务器，支持房间消息广播

**开发内容**：

1. **WebSocketFrame 帧解析**
   - 解析 WebSocket 帧头（FIN, opcode, mask, payload length）
   - 处理掩码解码
   - 支持文本帧
   - 支持 ping/pong 心跳
   - 支持关闭帧

2. **WebSocketHandler 协议处理**
   - WebSocket 握手升级（Sec-WebSocket-Accept 计算）
   - 帧解析和分发
   - 帧构建和发送
   - 连接生命周期管理

3. **RoomManager 房间连接管理**
   - 维护 room_id → Connection 列表的映射
   - joinRoom(room_id, connection, user_info)
   - leaveRoom(room_id, connection)
   - broadcastToRoom(room_id, message) - 排除发送者
   - getRoomViewerCount(room_id)
   - getRoomViewerList(room_id)

4. **WebSocket 连接流程**
   - 客户端连接 ws://server:8080/ws?token=xxx&room_id=xxx
   - 验证 token
   - 加入房间
   - 广播 viewer_join 消息
   - 接收消息并广播
   - 断开时广播 viewer_leave 消息

5. **消息处理**
   - danmaku 消息：广播给房间内所有人
   - gift 消息：广播 + 记录到数据库
   - like 消息：更新点赞计数 + 广播

6. **GiftDao 礼物数据访问**
   - getGiftList()
   - createGiftRecord(room_id, sender_id, gift_id)

7. **GiftService 礼物业务逻辑**
   - getGiftList()
   - sendGift(room_id, sender_id, gift_id)

8. **HTTP API 补充**
   - GET /api/gifts → GiftService::getGiftList

**验收标准**：
- [x] WebSocket 握手成功
- [x] 发送弹幕，房间内其他用户收到
- [x] 发送礼物，房间内其他用户收到
- [x] 点赞计数实时更新
- [x] 观众进出通知正常
- [x] 异常断开连接正确清理

**GitHub 提交信息**：
```
feat: 实现 WebSocket 实时通信，支持弹幕/礼物/点赞消息广播
```

---

### v0.5 - 直播录制与回放

**目标**：实现直播流录制和回放功能

**开发内容**：

1. **FFmpegRecorder 录制进程管理**
   - 启动 FFmpeg 子进程录制 RTMP 流
   - 命令：ffmpeg -i rtmp://localhost/live/{stream_key} -c copy -f mp4 {output_path}
   - 进程监控（SIGCHLD 信号处理）
   - 录制异常检测
   - 优雅停止录制（发送 q 命令给 FFmpeg）

2. **RecordingManager 录制管理器**
   - startRecording(room_id, stream_key)
   - stopRecording(room_id)
   - isRecording(room_id)
   - 获取录制文件路径

3. **ReplayDao 回放数据访问**
   - createReplay(room_id, title, anchor_id, duration, file_path, cover_path)
   - getReplayList()
   - getReplayById(replay_id)

4. **ReplayService 回放业务逻辑**
   - 生成回放记录（直播结束时调用）
   - 获取回放列表
   - 获取回放详情

5. **HTTP API 补充**
   - GET /api/live/replays → ReplayService::getReplayList

6. **StaticFileHandler 静态文件服务**
   - 提供头像文件访问：/avatars/{id}.png
   - 提供录制文件访问：/recordings/{filename}
   - 提供封面图访问：/covers/{room_id}.jpg
   - 支持 MIME 类型识别
   - 支持 Range 请求（视频拖拽进度条需要）

7. **录制集成到直播流程**
   - 创建直播间时自动开始录制
   - 结束直播间时停止录制
   - 计算录制时长
   - 生成回放记录

8. **封面截图**
   - 直播开始时从 RTMP 流截取一帧作为封面
   - 命令：ffmpeg -i rtmp://localhost/live/{stream_key} -frames:v 1 {cover_path}
   - 封面存储到 static/covers/

**验收标准**：
- [x] 直播开始后自动录制
- [x] 直播结束后录制文件生成
- [x] 回放列表 API 返回正确数据
- [x] 静态文件服务可访问录制视频
- [x] curl 下载录制视频文件成功

**GitHub 提交信息**：
```
feat: 实现直播流录制和回放功能，FFmpeg 进程管理和静态文件服务
```

---

### v0.6 - 完善与优化

**目标**：完善业务逻辑、错误处理、性能优化

**开发内容**：

1. **Token 鉴权完善**
   - 所有需要鉴权的 API 验证 token
   - token 过期自动清理
   - 支持同一用户多设备登录

2. **错误处理完善**
   - 统一错误码定义
   - 数据库操作异常捕获
   - 网络异常处理
   - FFmpeg 进程异常处理

3. **日志完善**
   - 请求日志（访问日志格式）
   - 业务日志（关键操作记录）
   - 错误日志（异常堆栈）

4. **性能优化**
   - 数据库连接池（SQLite WAL 模式）
   - HTTP keep-alive 支持
   - 静态文件缓存头
   - WebSocket ping/pong 心跳保活

5. **配置文件完善**
   - 服务器端口配置
   - 线程池大小配置
   - 数据库路径配置
   - 录制文件路径配置
   - Nginx-RTMP 地址配置
   - 日志级别配置

6. **优雅关闭**
   - SIGINT/SIGTERM 信号处理
   - 停止接受新连接
   - 等待现有请求完成
   - 关闭所有 WebSocket 连接
   - 停止所有录制进程
   - 关闭数据库

7. **数据库优化**
   - 添加索引
   - WAL 模式
   - 定期清理过期 token

**验收标准**：
- [x] 所有 API 鉴权正常
- [x] 错误响应格式统一
- [x] 日志文件正常记录
- [x] Ctrl+C 优雅关闭
- [x] 配置文件可修改运行参数

**GitHub 提交信息**：
```
feat: 完善鉴权、错误处理、日志和性能优化，支持优雅关闭
```

---

## 3. 关键技术实现指南

### 3.1 epoll 事件循环核心逻辑

```cpp
void EventLoop::loop() {
    while (!m_quit) {
        int n = epoll_wait(m_epollFd, m_events, MAX_EVENTS, -1);
        for (int i = 0; i < n; i++) {
            if (m_events[i].data.fd == m_listenFd) {
                // accept 新连接
                handleAccept();
            } else {
                // 分发到工作线程
                Connection* conn = static_cast<Connection*>(m_events[i].data.ptr);
                m_threadPool.submit([conn, events = m_events[i]]() {
                    if (events.events & EPOLLIN) conn->handleRead();
                    if (events.events & EPOLLOUT) conn->handleWrite();
                });
            }
        }
    }
}
```

### 3.2 WebSocket 握手

```cpp
bool WebSocketHandler::handshake(HttpRequest& req, HttpResponse& resp) {
    std::string key = req.getHeader("Sec-WebSocket-Key");
    std::string accept = sha1_base64(key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    resp.setStatus(101);
    resp.setHeader("Upgrade", "websocket");
    resp.setHeader("Connection", "Upgrade");
    resp.setHeader("Sec-WebSocket-Accept", accept);
    return true;
}
```

### 3.3 房间消息广播

```cpp
void RoomManager::broadcast(int roomId, const std::string& msg, int excludeFd) {
    auto it = m_rooms.find(roomId);
    if (it == m_rooms.end()) return;
    for (auto& conn : it->second.connections) {
        if (conn->fd() != excludeFd) {
            conn->sendWebSocketFrame(msg);
        }
    }
}
```

### 3.4 FFmpeg 录制

```cpp
void FFmpegRecorder::start(const std::string& streamKey, const std::string& outputPath) {
    std::string cmd = "ffmpeg -i rtmp://localhost/live/" + streamKey +
                      " -c copy -f mp4 " + outputPath;
    m_process = popen(cmd.c_str(), "w");
    // 或者使用 fork + exec 更精细控制
}
```

### 3.5 SHA-256 密码哈希

```cpp
std::string Crypto::hashPassword(const std::string& password, const std::string& salt) {
    std::string input = password + salt;
    // SHA-256 实现（可用 OpenSSL 或自实现）
    unsigned char hash[32];
    SHA256(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);
    return bytesToHex(hash, 32);
}
```

---

## 4. 服务器配置文件格式 (server.conf)

```ini
[server]
port=9090
thread_count=4

[database]
path=./data/livekit.db

[recording]
path=./static/recordings
ffmpeg_path=/usr/bin/ffmpeg

[nginx]
rtmp_host=127.0.0.1
rtmp_port=1935

[log]
level=info
path=./logs/livekit.log
```

---

## 5. 数据库初始化 SQL (init_db.sql)

```sql
CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    salt TEXT NOT NULL,
    avatar_id INTEGER DEFAULT 1,
    created_at TEXT DEFAULT (datetime('now', 'localtime')),
    last_login TEXT
);

CREATE TABLE IF NOT EXISTS tokens (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    token TEXT UNIQUE NOT NULL,
    created_at TEXT DEFAULT (datetime('now', 'localtime')),
    expires_at TEXT NOT NULL,
    FOREIGN KEY (user_id) REFERENCES users(id)
);

CREATE TABLE IF NOT EXISTS rooms (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    anchor_id INTEGER NOT NULL,
    title TEXT NOT NULL,
    category TEXT DEFAULT 'other',
    mode TEXT DEFAULT 'camera',
    stream_key TEXT UNIQUE NOT NULL,
    status TEXT DEFAULT 'live',
    viewer_count INTEGER DEFAULT 0,
    created_at TEXT DEFAULT (datetime('now', 'localtime')),
    ended_at TEXT,
    FOREIGN KEY (anchor_id) REFERENCES users(id)
);

CREATE TABLE IF NOT EXISTS gifts (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    icon TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS gift_records (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    room_id INTEGER NOT NULL,
    sender_id INTEGER NOT NULL,
    gift_id INTEGER NOT NULL,
    created_at TEXT DEFAULT (datetime('now', 'localtime')),
    FOREIGN KEY (room_id) REFERENCES rooms(id),
    FOREIGN KEY (sender_id) REFERENCES users(id),
    FOREIGN KEY (gift_id) REFERENCES gifts(id)
);

CREATE TABLE IF NOT EXISTS replays (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    room_id INTEGER NOT NULL,
    title TEXT NOT NULL,
    anchor_id INTEGER NOT NULL,
    duration INTEGER DEFAULT 0,
    file_path TEXT NOT NULL,
    cover_path TEXT,
    created_at TEXT DEFAULT (datetime('now', 'localtime')),
    FOREIGN KEY (room_id) REFERENCES rooms(id),
    FOREIGN KEY (anchor_id) REFERENCES users(id)
);

-- 预设礼物数据
INSERT INTO gifts (name, icon) VALUES ('小花', 'flower.png');
INSERT INTO gifts (name, icon) VALUES ('鼓掌', 'clap.png');
INSERT INTO gifts (name, icon) VALUES ('比心', 'heart.png');
INSERT INTO gifts (name, icon) VALUES ('火箭', 'rocket.png');
INSERT INTO gifts (name, icon) VALUES ('皇冠', 'crown.png');
INSERT INTO gifts (name, icon) VALUES ('烟花', 'firework.png');

-- 索引
CREATE INDEX IF NOT EXISTS idx_rooms_status ON rooms(status);
CREATE INDEX IF NOT EXISTS idx_rooms_anchor ON rooms(anchor_id);
CREATE INDEX IF NOT EXISTS idx_tokens_token ON tokens(token);
CREATE INDEX IF NOT EXISTS idx_tokens_user ON tokens(user_id);
```

---

## 6. 依赖库

| 库 | 版本 | 用途 | 集成方式 |
|----|------|------|---------|
| SQLite3 | 3.x | 嵌入式数据库 | amalgamation 源码编译 |
| nlohmann/json | 3.x | JSON 解析 | header-only |
| SHA-256 | - | 密码哈希 | 自实现或 OpenSSL |

---

## 7. 编译与运行

### 编译
```bash
cd server
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 运行
```bash
./LiveKitServer ../config/server.conf
```

### 测试
```bash
# 注册
curl -X POST http://192.168.x.x:9090/api/register \
  -H "Content-Type: application/json" \
  -d '{"username":"test","password":"123456","avatar_id":1}'

# 登录
curl -X POST http://192.168.x.x:9090/api/login \
  -H "Content-Type: application/json" \
  -d '{"username":"test","password":"123456"}'

# 获取直播间列表
curl http://192.168.x.x:9090/api/live/rooms

# 创建直播间
curl -X POST http://192.168.x.x:9090/api/live/create \
  -H "Content-Type: application/json" \
  -d '{"token":"xxx","title":"测试直播","category":"chat","mode":"camera"}'
```
