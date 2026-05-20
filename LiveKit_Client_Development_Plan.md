# LiveKit 客户端开发计划

## 1. 项目结构

```
LiveKit/
├── client/
│   ├── CMakeLists.txt
│   ├── src/
│   │   ├── main.cpp
│   │   ├── app/
│   │   │   ├── Application.h/cpp          # 应用主类，管理全局状态
│   │   │   └── AppConfig.h/cpp            # 配置管理（服务器地址等）
│   │   ├── network/
│   │   │   ├── HttpClient.h/cpp           # HTTP 请求封装
│   │   │   ├── WebSocketClient.h/cpp      # WebSocket 客户端封装
│   │   │   └── ApiResponse.h/cpp          # API 响应解析
│   │   ├── core/
│   │   │   ├── VideoPlayer.h/cpp          # 视频播放器（FFmpeg+SDL+OpenGL）
│   │   │   ├── VideoPusher.h/cpp          # 视频推流器（FFmpeg编码+RTMP）
│   │   │   ├── CameraCapture.h/cpp        # 摄像头采集（OpenCV）
│   │   │   ├── DesktopCapture.h/cpp       # 桌面采集（QScreen）
│   │   │   ├── AudioCapture.h/cpp         # 音频采集（QAudioInput）
│   │   │   ├── AudioPlayer.h/cpp          # 音频播放（SDL2）
│   │   │   ├── PacketQueue.h/cpp          # 音视频包队列
│   │   │   └── PicInPic.h/cpp             # 画中画合成
│   │   ├── ui/
│   │   │   ├── MainWindow.h/cpp           # 主窗口（底部导航）
│   │   │   ├── LoginPage.h/cpp            # 登录页
│   │   │   ├── RegisterPage.h/cpp         # 注册页
│   │   │   ├── LiveHallPage.h/cpp         # 直播大厅
│   │   │   ├── LiveRoomPage.h/cpp         # 直播间（观众视角）
│   │   │   ├── AnchorRoomPage.h/cpp       # 直播间（主播视角）
│   │   │   ├── StartLivePage.h/cpp        # 开播设置页
│   │   │   ├── ReplayPlayerPage.h/cpp     # 回放播放页
│   │   │   ├── ProfilePage.h/cpp          # 个人中心
│   │   │   ├── SettingsPage.h/cpp         # 设置页
│   │   │   ├── DanmakuWidget.h/cpp        # 弹幕组件
│   │   │   ├── GiftPanel.h/cpp            # 礼物面板
│   │   │   ├── LikeButton.h/cpp           # 点赞按钮+动画
│   │   │   ├── GiftAnimation.h/cpp        # 礼物动画
│   │   │   ├── RoomCard.h/cpp             # 直播间卡片
│   │   │   ├── OpenGLWidget.h/cpp         # OpenGL 渲染组件
│   │   │   └── PicInPicWidget.h/cpp       # 画中画悬浮窗
│   │   ├── theme/
│   │   │   ├── ThemeManager.h/cpp         # 主题管理器
│   │   │   ├── DarkTheme.h/cpp            # 深色主题定义
│   │   │   └── LightTheme.h/cpp           # 浅色主题定义
│   │   └── model/
│   │       ├── UserInfo.h/cpp             # 用户信息模型
│   │       ├── RoomInfo.h/cpp             # 直播间信息模型
│   │       ├── ReplayInfo.h/cpp           # 回放信息模型
│   │       └── GiftInfo.h/cpp             # 礼物信息模型
│   ├── resources/
│   │   ├── icons/                         # 图标资源
│   │   ├── avatars/                       # 预设头像
│   │   ├── gifts/                         # 礼物图标
│   │   ├── qss/                           # QSS 样式表
│   │   │   ├── dark.qss
│   │   │   └── light.qss
│   │   └── resources.qrc                  # Qt 资源文件
│   └── third_party/
│       ├── ffmpeg-4.2.2/                  # FFmpeg 库
│       ├── SDL2-2.0.10/                   # SDL2 库
│       └── opencv-4.2.0/                  # OpenCV 库
└── server/                                # 服务端（另一个 trae 窗口开发）
```

---

## 2. 迭代开发计划

### v0.1 - 项目骨架与基础框架

**目标**：搭建 CMake 项目结构，实现主窗口框架和主题系统骨架

**开发内容**：

1. **CMake 项目搭建**
   - 创建 CMakeLists.txt，配置 Qt5 / FFmpeg / SDL2 / OpenCV 依赖
   - 建立目录结构
   - 配置 Debug/Release 构建

2. **Application 主类**
   - 单例模式管理应用生命周期
   - 管理全局状态（当前用户、服务器地址等）

3. **MainWindow 主窗口**
   - 底部导航栏：直播大厅 / 我要开播 / 个人中心
   - QStackedWidget 切换页面
   - 窗口图标和标题

4. **ThemeManager 主题管理器**
   - QSS 样式表加载机制
   - 深色/浅色主题切换接口
   - 主题偏好本地保存（QSettings）

5. **AppConfig 配置管理**
   - 服务器地址配置
   - QSettings 持久化

**验收标准**：
- [x] CMake 能成功编译运行
- [x] 主窗口显示底部导航，可切换空白页面
- [x] 深色/浅色主题可切换

**GitHub 提交信息**：
```
feat: 项目初始化，搭建 CMake 骨架、主窗口框架和主题系统
```

---

### v0.2 - 用户注册与登录

**目标**：实现完整的注册登录流程

**开发内容**：

1. **HttpClient 网络模块**
   - 基于 QNetworkAccessManager 封装
   - 支持 GET/POST 请求
   - JSON 请求体和响应体解析
   - 请求超时和错误处理

2. **LoginPage 登录页**
   - 用户名输入框
   - 密码输入框
   - 登录按钮
   - "没有账号？去注册" 链接
   - 登录中 loading 状态
   - 错误提示

3. **RegisterPage 注册页**
   - 用户名输入框
   - 密码输入框
   - 确认密码输入框
   - 头像选择（网格展示预设头像，点击选中高亮）
   - 注册按钮
   - "已有账号？去登录" 链接

4. **UserInfo 用户模型**
   - 用户 ID、用户名、头像 ID
   - Token 管理（本地保存、自动登录）

5. **ApiResponse 响应解析**
   - 统一的 API 响应格式解析
   - 错误码处理

**验收标准**：
- [x] 注册新用户成功，自动登录进入大厅
- [x] 登录已有用户成功，进入大厅
- [x] 登录失败显示错误提示
- [x] 关闭应用后重新打开自动登录

**GitHub 提交信息**：
```
feat: 实现用户注册登录功能，包含 HTTP 请求模块和 Token 管理
```

---

### v0.3 - 直播大厅

**目标**：实现直播大厅页面，展示直播间列表

**开发内容**：

1. **LiveHallPage 直播大厅**
   - 顶部：Logo + 分类筛选标签栏（全部/游戏/聊天/音乐/其他）
   - 主体：QScrollArea + QGridLayout 网格卡片
   - 下拉刷新
   - 空状态提示

2. **RoomCard 直播间卡片**
   - 封面截图（占位图或实际截图）
   - 主播头像（圆形裁剪）
   - 主播名称
   - 直播标题
   - 观众人数
   - 分类标签
   - hover 效果（阴影 + 放大）
   - 点击信号

3. **RoomInfo 直播间模型**
   - 解析服务端返回的房间列表数据

4. **定时刷新**
   - QTimer 每 30 秒自动刷新列表

**验收标准**：
- [x] 大厅页面展示直播间网格卡片
- [x] 点击分类标签筛选对应类型
- [x] 点击卡片跳转（暂时跳转到空白页）

**GitHub 提交信息**：
```
feat: 实现直播大厅页面，网格卡片布局展示直播间列表
```

---

### v0.4 - 直播推流与播放

**目标**：实现核心的直播推流和 RTMP 播放功能

**开发内容**：

1. **StartLivePage 开播设置页**
   - 直播标题输入
   - 分类选择下拉框
   - 模式选择：摄像头直播 / 桌面直播 / 桌面+画中画
   - "开始直播"按钮

2. **CameraCapture 摄像头采集**
   - 基于 OpenCV VideoCapture
   - 独立线程采集
   - 帧数据信号发射

3. **DesktopCapture 桌面采集**
   - 基于 QScreen::grabWindow
   - 独立线程采集
   - 帧数据信号发射

4. **AudioCapture 音频采集**
   - 基于 QAudioInput
   - PCM 采集 + swr_convert 转 FLTP
   - 音频帧数据信号发射

5. **PicInPic 画中画合成**
   - 摄像头画面缩放到 320x240
   - 叠加到桌面画面右下角
   - RGB → YUV420P 转换

6. **VideoPusher 视频推流器**
   - FFmpeg H.264 编码（superfast/zerolatency）
   - FFmpeg AAC 编码
   - FLV 封装
   - RTMP 推流
   - 视频帧队列 + 音频帧队列
   - 稳帧机制

7. **VideoPlayer 视频播放器**
   - FFmpeg 解码（使用新版 API avcodec_send_packet/receive_frame）
   - SDL2 音频播放
   - QOpenGLWidget 视频渲染
   - 音视频同步（音频时钟为基准）
   - PacketQueue 缓冲队列

8. **AnchorRoomPage 主播直播间**
   - 视频预览区域
   - 在线人数显示
   - 停止直播按钮

9. **LiveRoomPage 观众直播间**
   - 视频播放区域
   - 音量控制
   - 全屏切换
   - 返回按钮

10. **PicInPicWidget 画中画悬浮窗**
    - 置顶显示摄像头画面
    - 可拖拽移动
    - 可关闭/重新打开

**验收标准**：
- [x] 摄像头直播模式：采集摄像头画面 + 音频，推流到 RTMP
- [x] 桌面直播模式：采集桌面画面 + 音频，推流到 RTMP
- [x] 桌面+画中画模式：桌面画面 + 右下角摄像头 + 音频，推流到 RTMP
- [x] 观众端输入 RTMP 地址可播放直播流
- [x] 音视频同步正常
- [x] 画中画窗口可拖拽

**GitHub 提交信息**：
```
feat: 实现直播推流（摄像头/桌面/画中画）和 RTMP 播放功能
```

---

### v0.5 - 弹幕系统

**目标**：实现实时弹幕收发

**开发内容**：

1. **WebSocketClient 模块**
   - 基于 QWebSocket
   - 连接/断开/重连机制
   - 消息发送和接收
   - JSON 消息解析

2. **DanmakuWidget 弹幕组件**
   - 固定位置弹幕显示区域
   - 消息从下往上弹出
   - 半透明背景 + 白色文字
   - 最多显示 50 条，超出自动移除旧消息
   - 自动滚动到底部

3. **弹幕输入框**
   - 底部输入框 + 发送按钮
   - Enter 键发送
   - 发送后清空输入框

4. **弹幕消息集成**
   - 观众端：发送弹幕 + 显示弹幕
   - 主播端：显示弹幕

**验收标准**：
- [x] WebSocket 连接成功
- [x] 发送弹幕，其他用户实时看到
- [x] 弹幕固定位置从下往上弹出
- [x] 弹幕自动滚动

**GitHub 提交信息**：
```
feat: 实现弹幕系统，WebSocket 实时消息推送和固定弹幕显示
```

---

### v0.6 - 礼物与点赞系统

**目标**：实现礼物赠送和点赞功能

**开发内容**：

1. **GiftPanel 礼物面板**
   - 点击礼物图标展开面板
   - 网格展示 6-8 种礼物（图标 + 名称）
   - 点击礼物发送
   - 面板外点击关闭

2. **GiftAnimation 礼物动画**
   - QPropertyAnimation 实现
   - 礼物图标从右向左飞入
   - 显示送礼人名称
   - 3秒后自动消失

3. **LikeButton 点赞按钮**
   - 红心图标按钮
   - 点击触发飘心动画
   - 连击支持（500ms 内连续点击计数）

4. **飘心动画**
   - QPropertyAnimation
   - 红心从底部向上飘出
   - 随机左右偏移
   - 渐隐效果
   - 同时最多 10 个心

5. **点赞计数显示**
   - 实时显示当前直播间点赞总数

6. **GiftInfo 礼物模型**
   - 礼物 ID、名称、图标路径

**验收标准**：
- [x] 礼物面板展开/关闭正常
- [x] 点击礼物发送，其他用户看到礼物动画
- [x] 点赞按钮点击后飘心动画
- [x] 点赞计数实时更新

**GitHub 提交信息**：
```
feat: 实现礼物赠送动画和点赞飘心效果
```

---

### v0.7 - 直播录制与回放

**目标**：实现直播回放功能

**开发内容**：

1. **ReplayPlayerPage 回放播放页**
   - 复用 VideoPlayer 模块
   - 进度条（QSlider）
   - 暂停/继续按钮
   - 时间显示（当前/总时长）
   - 返回按钮

2. **回放列表集成**
   - 大厅页面增加"回放"标签
   - 回放卡片显示时长标签
   - 点击回放卡片进入回放播放页

3. **ReplayInfo 回放模型**
   - 解析服务端返回的回放数据

**验收标准**：
- [x] 直播结束后大厅可看到回放卡片
- [x] 点击回放卡片可播放录制视频
- [x] 进度条可拖拽
- [x] 暂停/继续正常

**GitHub 提交信息**：
```
feat: 实现直播录制回放功能，支持进度控制和暂停续播
```

---

### v0.8 - 完善与优化

**目标**：UI 打磨、主题完善、bug 修复

**开发内容**：

1. **主题系统完善**
   - 深色主题 QSS 精调
   - 浅色主题 QSS 精调
   - 所有页面统一风格
   - 圆角、阴影、过渡动画

2. **SettingsPage 设置页**
   - 服务器地址配置
   - 推流画质选择
   - 主题切换开关
   - 关于信息

3. **ProfilePage 个人中心**
   - 头像、用户名、ID 显示
   - 直播历史记录
   - 退出登录按钮

4. **错误处理与重连**
   - 网络断开提示
   - WebSocket 自动重连
   - 推流失败重试

5. **UI 细节打磨**
   - 加载动画（QMovie 或自定义）
   - 空状态占位图
   - 按钮点击反馈
   - 页面切换过渡动画

**验收标准**：
- [x] 深色/浅色主题全局一致
- [x] 设置页功能完整
- [x] 个人中心功能完整
- [x] 网络异常有友好提示
- [x] 整体 UI 美观简约

**GitHub 提交信息**：
```
feat: 完善主题系统、设置页、个人中心，优化 UI 细节和错误处理
```

---

## 3. 关键技术实现参考

### 3.1 VideoPlayer（参考现有 MediaPlayer 项目）

现有项目使用的是 FFmpeg 旧版 API，新项目需替换：

| 旧 API | 新 API |
|--------|--------|
| av_register_all() | 删除（4.0+ 自动注册） |
| avcodec_decode_video2() | avcodec_send_packet() + avcodec_receive_frame() |
| avcodec_decode_audio4() | avcodec_send_packet() + avcodec_receive_frame() |
| av_free_packet() | av_packet_unref() |
| avpicture_get_size() | av_image_get_buffer_size() |
| avpicture_fill() | av_image_fill_arrays() |
| codec->time_base | 使用 avcodec_parameters 代替直接访问 |

### 3.2 VideoPusher（参考现有 VideoRecorder 项目）

核心流程保持一致：
1. 采集视频帧（OpenCV / QScreen）
2. RGB → YUV420P 转换（sws_scale）
3. H.264 编码（avcodec_send_frame + avcodec_receive_packet）
4. 采集音频帧（QAudioInput + swr_convert）
5. AAC 编码
6. FLV 封装 + RTMP 推流（av_interleaved_write_frame）

### 3.3 画中画合成

参考现有 PicInPic_Read：
1. 获取桌面截图 QImage
2. 获取摄像头帧 QImage
3. QPainter 将摄像头画面绘制到桌面截图右下角
4. 合成后的 QImage → YUV420P → 编码推流

### 3.4 WebSocket 消息

使用 QWebSocket：
```cpp
QWebSocket *m_webSocket = new QWebSocket();
connect(m_webSocket, &QWebSocket::textMessageReceived, [](const QString &msg) {
    QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8());
    // 处理消息
});
m_webSocket->open(QUrl("ws://192.168.x.x:8080/ws?token=xxx&room_id=1"));
```

---

## 4. 依赖库版本

| 库 | 版本 | 用途 |
|----|------|------|
| Qt | 5.x | GUI 框架 |
| FFmpeg | 4.2.2 | 音视频编解码 |
| SDL2 | 2.0.10 | 音频播放 |
| OpenCV | 4.2.0 | 摄像头采集 |
| nlohmann/json | 3.x | JSON 解析（可选，也可用 Qt JSON） |
