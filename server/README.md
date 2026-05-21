# LiveKit Server

LiveKit 直播平台后端服务，使用纯 C++ 开发，基于 epoll 实现高并发 HTTP/WebSocket 服务器。

## 技术栈

- C++14 / CMake
- epoll (ET 模式) + 线程池 (Reactor 模式)
- SQLite3 (WAL 模式)
- nlohmann/json
- OpenSSL (SHA-256)
- FFmpeg (录制/截图)

## 编译

```bash
cd server
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

## 运行

```bash
cd server
./scripts/start.sh
# 或直接运行
./build/LiveKitServer config/server.conf
```

服务器默认监听端口 9090。

## 配置

配置文件：`config/server.conf`

```ini
[server]
port=9090
thread_count=4
host=127.0.0.1

[database]
path=./data/livekit.db

[recording]
path=./static/recordings
ffmpeg_path=/usr/bin/ffmpeg

[nginx]
rtmp_host=127.0.0.1
rtmp_port=1935
server_ip=127.0.0.1

[log]
level=info
path=./logs/livekit.log
```

## HTTP API

### 通用响应格式

```json
{"code": 0, "msg": "success", "data": {}}
```

### 用户系统

| 接口 | 方法 | 说明 |
|------|------|------|
| `/api/register` | POST | 用户注册 |
| `/api/login` | POST | 用户登录 |
| `/api/avatars` | GET | 获取头像列表 |

### 直播间

| 接口 | 方法 | 说明 |
|------|------|------|
| `/api/live/create` | POST | 创建直播间 |
| `/api/live/end` | POST | 结束直播 |
| `/api/live/rooms` | GET | 直播间列表 |
| `/api/live/room/{room_id}` | GET | 直播间详情 |
| `/api/live/join` | POST | 加入直播间 |
| `/api/live/leave` | POST | 离开直播间 |

### 礼物与回放

| 接口 | 方法 | 说明 |
|------|------|------|
| `/api/gifts` | GET | 礼物列表 |
| `/api/live/replays` | GET | 回放列表 |

### 静态文件

| 路径 | 说明 |
|------|------|
| `/avatars/{id}.png` | 头像文件 |
| `/gifts/{name}.png` | 礼物图标 |
| `/covers/{room_id}.jpg` | 直播封面 |
| `/recordings/{filename}.mp4` | 录制视频 (支持 Range 请求) |

## WebSocket

连接地址：`ws://{host}:9090/ws?token={token}&room_id={room_id}`

### 客户端发送

| type | 字段 | 说明 |
|------|------|------|
| danmaku | content | 弹幕 |
| gift | gift_id | 送礼物 |
| like | - | 点赞 |

### 服务端推送

| type | 说明 |
|------|------|
| danmaku | 弹幕广播 |
| gift | 礼物广播 |
| like | 点赞计数 |
| viewer_join | 观众加入 |
| viewer_leave | 观众离开 |
| viewer_count | 观众人数 |
| live_end | 直播结束 |

## 测试示例

```bash
# 注册
curl -X POST http://127.0.0.1:9090/api/register \
  -H "Content-Type: application/json" \
  -d '{"username":"test","password":"123456","avatar_id":1}'

# 登录
curl -X POST http://127.0.0.1:9090/api/login \
  -H "Content-Type: application/json" \
  -d '{"username":"test","password":"123456"}'

# 创建直播间
curl -X POST http://127.0.0.1:9090/api/live/create \
  -H "Content-Type: application/json" \
  -d '{"token":"YOUR_TOKEN","title":"测试直播","category":"game","mode":"camera"}'

# 获取直播间列表
curl http://127.0.0.1:9090/api/live/rooms

# 结束直播
curl -X POST http://127.0.0.1:9090/api/live/end \
  -H "Content-Type: application/json" \
  -d '{"token":"YOUR_TOKEN","room_id":1}'

# 获取回放列表
curl http://127.0.0.1:9090/api/live/replays

# Range 请求 (视频拖拽)
curl -H "Range: bytes=0-1023" http://127.0.0.1:9090/recordings/test.mp4
```

## 目录结构

```
server/
├── CMakeLists.txt
├── config/server.conf
├── scripts/
│   ├── init_db.sql
│   └── start.sh
├── src/
│   ├── main.cpp
│   ├── core/          (Logger, EventLoop, ThreadPool, EpollWrapper, TaskQueue)
│   ├── network/       (HttpServer, HttpRequest, HttpResponse, Router, Connection, Buffer, WebSocket*, StaticFileHandler)
│   ├── business/      (UserService, RoomService, RoomManager, GiftService, ReplayService)
│   ├── database/      (Database, UserDao, RoomDao, GiftDao, ReplayDao)
│   ├── recording/     (FFmpegRecorder, RecordingManager)
│   └── utils/         (Config, Crypto, TokenGenerator, ErrorCode)
├── static/
│   ├── avatars/
│   ├── covers/
│   ├── gifts/
│   └── recordings/
├── data/              (SQLite DB, 运行时生成)
├── logs/              (日志文件, 按天分割)
└── third_party/       (sqlite3, nlohmann/json)
```
