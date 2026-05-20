# LiveKit 服务端需求文档

## 1. 项目概述

### 1.1 项目名称
LiveKit Server - 直播平台后端服务

### 1.2 项目定位
为 LiveKit 客户端提供用户管理、直播间管理、实时消息推送、直播录制等后端服务的纯 C++ 服务端程序。

### 1.3 运行环境
- 操作系统：Ubuntu (虚拟机)
- 网络环境：局域网，Windows 客户端通过虚拟机 IP 访问
- 协同服务：Nginx-RTMP（流媒体转发）

---

## 2. 技术栈

| 维度 | 技术选型 | 说明 |
|------|---------|------|
| 语言 | C++11/14 | 使用现代 C++ 特性 |
| 构建系统 | CMake | 跨平台构建 |
| 网络框架 | 自研 | 基于 epoll 的 HTTP/WebSocket 服务器 |
| 数据库 | SQLite 3 | 轻量级嵌入式数据库 |
| JSON 解析 | nlohmann/json | 现代 C++ JSON 库 |
| 密码加密 | SHA-256 | 用户密码哈希存储 |
| 直播录制 | FFmpeg (命令行) | 录制 RTMP 流为 MP4 文件 |
| 流媒体 | Nginx-RTMP | RTMP 推流/拉流转发 |
| 日志 | 自研 | 基于文件的多级别日志系统 |
| 线程模型 | 线程池 | Reactor 模式 + 线程池 |

---

## 3. 架构设计

### 3.1 整体架构

```
┌─────────────────────────────────────────────────┐
│                  LiveKit Server                  │
│                                                  │
│  ┌──────────┐  ┌──────────┐  ┌───────────────┐ │
│  │ HTTP     │  │WebSocket │  │ Recording     │ │
│  │ Handler  │  │ Manager  │  │ Manager       │ │
│  └────┬─────┘  └────┬─────┘  └───────┬───────┘ │
│       │              │                │          │
│  ┌────┴──────────────┴────────────────┴───────┐ │
│  │              Business Logic Layer           │ │
│  │  UserService | RoomService | GiftService    │ │
│  └────────────────────┬───────────────────────┘ │
│                       │                          │
│  ┌────────────────────┴───────────────────────┐ │
│  │              Data Access Layer              │ │
│  │           SQLite Database                   │ │
│  └────────────────────────────────────────────┘ │
│                                                  │
│  ┌────────────────────────────────────────────┐ │
│  │           Network Layer (epoll)             │ │
│  │     ThreadPool | EventLoop | Connection     │ │
│  └────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────┘
         │                    │
    ┌────┴─────┐        ┌────┴─────┐
    │ Nginx    │        │ SQLite   │
    │ RTMP     │        │ DB File  │
    └──────────┘        └──────────┘
```

### 3.2 网络层设计

#### 3.2.1 Reactor 模式
- 基于 Linux epoll 实现事件循环
- 主线程负责 accept 新连接
- 工作线程池处理读写事件
- 支持同时处理 HTTP 和 WebSocket 连接

#### 3.2.2 HTTP 服务器
- 支持 GET / POST 请求解析
- 支持 JSON 请求体和响应体
- 支持 CORS 跨域（局域网场景可选）
- 路由表注册机制

#### 3.2.3 WebSocket 服务器
- 支持 WebSocket 握手升级
- 支持文本帧收发
- 支持房间概念（每个直播间一个房间）
- 消息广播机制（同一房间内广播）

### 3.3 线程模型
- 1个主线程：epoll 事件循环 + accept
- N个工作线程：处理 HTTP 请求和 WebSocket 消息
- 1个录制线程：管理 FFmpeg 录制进程
- 使用线程池任务队列分发

---

## 4. 功能需求

### 4.1 用户系统

#### 4.1.1 用户注册
- 接收客户端注册请求（username, password）
- 校验用户名唯一性
- 密码使用 SHA-256 加盐哈希后存储
- 注册成功生成 token（用户ID+时间戳+签名）
- 返回 token 给客户端

#### 4.1.2 用户登录
- 接收客户端登录请求（username, password）
- 校验用户名密码
- 登录成功生成 token
- 返回 token 和用户信息

#### 4.1.3 Token 验证
- 每个需要鉴权的请求携带 token
- 服务端验证 token 有效性
- token 有效期 24 小时

#### 4.1.4 头像管理
- 服务端内置 8-12 个预设头像文件
- 用户注册时选择头像 ID
- 头像通过 HTTP 静态文件服务提供访问

### 4.2 直播间管理

#### 4.2.1 创建直播间
- 主播发起创建请求（title, category, mode）
- 生成唯一 room_id 和 stream_key
- 返回推流地址 rtmp://{server_ip}/live/{stream_key}
- 在数据库中记录直播间信息
- 通知 Nginx-RTMP 准备接收推流

#### 4.2.2 直播间列表
- 返回所有正在直播的房间列表
- 每个房间包含：room_id, title, anchor_name, anchor_avatar, viewer_count, category, cover_url
- 支持按分类筛选

#### 4.2.3 结束直播
- 主播发起结束请求
- 停止 FFmpeg 录制进程
- 将直播记录标记为已结束
- 生成回放记录
- 通知房间内所有观众直播已结束

#### 4.2.4 直播间信息
- 返回指定直播间的详细信息
- 包含在线观众列表

### 4.3 实时消息系统

#### 4.3.1 WebSocket 连接管理
- 客户端通过 ws://{server_ip}:8080/ws?token={token}&room_id={room_id 连接
- 验证 token 有效性
- 将连接加入对应房间
- 维护房间 → 连接列表的映射

#### 4.3.2 弹幕消息
- 接收观众发送的弹幕
- 在房间内广播弹幕消息
- 弹幕消息包含：username, content, timestamp

#### 4.3.3 礼物消息
- 接收观众发送的礼物
- 在房间内广播礼物消息
- 礼物消息包含：username, gift_id, gift_name, timestamp
- 记录礼物到数据库

#### 4.3.4 点赞消息
- 接收观众点赞
- 在房间内广播点赞计数
- 点赞消息包含：count, timestamp

#### 4.3.5 观众进出通知
- 观众进入直播间时广播 viewer_join 消息
- 观众离开直播间时广播 viewer_leave 消息
- 定期广播 viewer_count 消息

### 4.4 直播录制

#### 4.4.1 录制启动
- 直播开始后，服务端启动 FFmpeg 进程录制 RTMP 流
- 命令：ffmpeg -i rtmp://localhost/live/{stream_key} -c copy -f mp4 {output_path}
- 录制文件存储路径：./recordings/{room_id}_{timestamp}.mp4

#### 4.4.2 录制停止
- 直播结束时停止 FFmpeg 录制进程
- 生成回放记录到数据库
- 回放文件通过 HTTP 静态文件服务提供访问

#### 4.4.3 回放管理
- 回放列表 API：返回所有可回放的直播记录
- 回放信息：replay_id, title, anchor_name, duration, cover_url, play_url
- play_url 格式：http://{server_ip}:8080/recordings/{filename}

### 4.5 静态文件服务
- 提供头像文件访问：/avatars/{avatar_id}.png
- 提供录制回放文件访问：/recordings/{filename}
- 提供直播间封面截图访问：/covers/{room_id}.jpg

---

## 5. 数据库设计

### 5.1 users 表
| 字段 | 类型 | 说明 |
|------|------|------|
| id | INTEGER PRIMARY KEY | 用户 ID |
| username | TEXT UNIQUE | 用户名 |
| password_hash | TEXT | SHA-256 哈希密码 |
| salt | TEXT | 密码盐值 |
| avatar_id | INTEGER | 预设头像 ID |
| created_at | TEXT | 注册时间 |
| last_login | TEXT | 最后登录时间 |

### 5.2 rooms 表
| 字段 | 类型 | 说明 |
|------|------|------|
| id | INTEGER PRIMARY KEY | 房间 ID |
| anchor_id | INTEGER | 主播用户 ID |
| title | TEXT | 直播标题 |
| category | TEXT | 直播分类 |
| mode | TEXT | 直播模式（camera/desk/desk_pip） |
| stream_key | TEXT UNIQUE | 推流密钥 |
| status | TEXT | 状态（live/ended） |
| viewer_count | INTEGER | 观众人数 |
| created_at | TEXT | 创建时间 |
| ended_at | TEXT | 结束时间 |

### 5.3 gifts 表
| 字段 | 类型 | 说明 |
|------|------|------|
| id | INTEGER PRIMARY KEY | 礼物 ID |
| name | TEXT | 礼物名称 |
| icon | TEXT | 礼物图标文件名 |

### 5.4 gift_records 表
| 字段 | 类型 | 说明 |
|------|------|------|
| id | INTEGER PRIMARY KEY | 记录 ID |
| room_id | INTEGER | 房间 ID |
| sender_id | INTEGER | 发送者 ID |
| gift_id | INTEGER | 礼物 ID |
| created_at | TEXT | 发送时间 |

### 5.5 replays 表
| 字段 | 类型 | 说明 |
|------|------|------|
| id | INTEGER PRIMARY KEY | 回放 ID |
| room_id | INTEGER | 原房间 ID |
| title | TEXT | 直播标题 |
| anchor_id | INTEGER | 主播 ID |
| duration | INTEGER | 时长（秒） |
| file_path | TEXT | 录制文件路径 |
| cover_path | TEXT | 封面图路径 |
| created_at | TEXT | 创建时间 |

### 5.6 tokens 表
| 字段 | 类型 | 说明 |
|------|------|------|
| id | INTEGER PRIMARY KEY | Token ID |
| user_id | INTEGER | 用户 ID |
| token | TEXT UNIQUE | Token 字符串 |
| created_at | TEXT | 创建时间 |
| expires_at | TEXT | 过期时间 |

---

## 6. HTTP API 接口定义

### 6.1 通用响应格式
```json
{
  "code": 0,
  "msg": "success",
  "data": {}
}
```

### 6.2 接口列表

#### POST /api/register
请求：
```json
{
  "username": "testuser",
  "password": "123456",
  "avatar_id": 1
}
```
响应：
```json
{
  "code": 0,
  "msg": "注册成功",
  "data": {
    "token": "abc123...",
    "user_info": {
      "id": 1,
      "username": "testuser",
      "avatar_id": 1
    }
  }
}
```

#### POST /api/login
请求：
```json
{
  "username": "testuser",
  "password": "123456"
}
```
响应：
```json
{
  "code": 0,
  "msg": "登录成功",
  "data": {
    "token": "abc123...",
    "user_info": {
      "id": 1,
      "username": "testuser",
      "avatar_id": 1
    }
  }
}
```

#### GET /api/avatars
响应：
```json
{
  "code": 0,
  "data": {
    "avatars": [
      {"id": 1, "url": "http://192.168.x.x:8080/avatars/1.png"},
      {"id": 2, "url": "http://192.168.x.x:8080/avatars/2.png"}
    ]
  }
}
```

#### GET /api/live/rooms?category=all
响应：
```json
{
  "code": 0,
  "data": {
    "rooms": [
      {
        "room_id": 1,
        "title": "一起玩游戏",
        "anchor_name": "player1",
        "anchor_avatar_id": 3,
        "viewer_count": 50,
        "category": "game",
        "cover_url": "http://192.168.x.x:8080/covers/1.jpg"
      }
    ]
  }
}
```

#### POST /api/live/create
请求：
```json
{
  "token": "abc123...",
  "title": "我的直播间",
  "category": "chat",
  "mode": "desk_pip"
}
```
响应：
```json
{
  "code": 0,
  "data": {
    "room_id": 1,
    "stream_key": "abc123def",
    "push_url": "rtmp://192.168.x.x/live/abc123def"
  }
}
```

#### POST /api/live/end
请求：
```json
{
  "token": "abc123...",
  "room_id": 1
}
```

#### GET /api/live/room/{room_id}
响应：
```json
{
  "code": 0,
  "data": {
    "room_info": {
      "room_id": 1,
      "title": "我的直播间",
      "anchor_name": "player1",
      "anchor_avatar_id": 3,
      "viewer_count": 50,
      "category": "chat",
      "status": "live"
    },
    "viewer_list": [
      {"id": 2, "username": "viewer1", "avatar_id": 1},
      {"id": 3, "username": "viewer2", "avatar_id": 5}
    ]
  }
}
```

#### GET /api/live/replays
响应：
```json
{
  "code": 0,
  "data": {
    "replays": [
      {
        "replay_id": 1,
        "title": "我的直播间",
        "anchor_name": "player1",
        "duration": 3600,
        "cover_url": "http://192.168.x.x:8080/covers/1.jpg",
        "play_url": "http://192.168.x.x:8080/recordings/1_1716192000.mp4"
      }
    ]
  }
}
```

---

## 7. WebSocket 消息协议

### 7.1 连接
- 地址：ws://{server_ip}:8080/ws?token={token}&room_id={room_id}
- 握手时验证 token，无效则拒绝连接
- 同一用户同一房间只允许一个连接

### 7.2 客户端发送消息类型

| type | 字段 | 说明 |
|------|------|------|
| danmaku | content | 发送弹幕 |
| gift | gift_id | 送礼物 |
| like | (无额外字段) | 点赞 |

### 7.3 服务端推送消息类型

| type | 字段 | 说明 |
|------|------|------|
| danmaku | username, content, timestamp | 弹幕广播 |
| gift | username, gift_id, gift_name, timestamp | 礼物广播 |
| like | count, timestamp | 点赞计数广播 |
| viewer_count | count | 观众人数更新 |
| viewer_join | username | 观众加入通知 |
| viewer_leave | username | 观众离开通知 |
| live_end | reason | 直播结束通知 |

---

## 8. Nginx-RTMP 配置要求

```nginx
rtmp {
    server {
        listen 1935;
        application live {
            live on;
            record off;
            # 允许所有来源推流
            allow publish all;
            allow play all;
        }
    }
}

http {
    server {
        listen 8080;
        # 代理 LiveKit Server 的 HTTP/WebSocket
        location / {
            proxy_pass http://127.0.0.1:9090;
            proxy_http_version 1.1;
            proxy_set_header Upgrade $http_upgrade;
            proxy_set_header Connection "upgrade";
        }
    }
}
```

---

## 9. 非功能需求

### 9.1 性能要求
- 支持同时 50+ 个 WebSocket 连接
- HTTP 请求响应时间 < 100ms
- WebSocket 消息广播延迟 < 200ms

### 9.2 可靠性要求
- FFmpeg 录制进程异常退出时自动检测并记录
- 数据库操作使用事务保证一致性
- 服务端异常退出时优雅关闭所有连接

### 9.3 代码质量
- 使用现代 C++ 特性（智能指针、RAII、lambda 等）
- Reactor 模式 + 线程池展示网络编程能力
- 清晰的分层架构（网络层/业务层/数据层）
- 完善的日志系统（DEBUG/INFO/WARN/ERROR）
