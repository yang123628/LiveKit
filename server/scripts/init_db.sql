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

CREATE TABLE IF NOT EXISTS gifts (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    icon TEXT NOT NULL
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
    like_count INTEGER DEFAULT 0,
    created_at TEXT DEFAULT (datetime('now', 'localtime')),
    ended_at TEXT,
    FOREIGN KEY (anchor_id) REFERENCES users(id)
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

INSERT OR IGNORE INTO gifts (id, name, icon) VALUES (1, '小花', 'flower.png');
INSERT OR IGNORE INTO gifts (id, name, icon) VALUES (2, '鼓掌', 'clap.png');
INSERT OR IGNORE INTO gifts (id, name, icon) VALUES (3, '比心', 'heart.png');
INSERT OR IGNORE INTO gifts (id, name, icon) VALUES (4, '火箭', 'rocket.png');
INSERT OR IGNORE INTO gifts (id, name, icon) VALUES (5, '皇冠', 'crown.png');
INSERT OR IGNORE INTO gifts (id, name, icon) VALUES (6, '烟花', 'firework.png');

CREATE INDEX IF NOT EXISTS idx_rooms_status ON rooms(status);
CREATE INDEX IF NOT EXISTS idx_rooms_anchor ON rooms(anchor_id);
CREATE INDEX IF NOT EXISTS idx_tokens_token ON tokens(token);
CREATE INDEX IF NOT EXISTS idx_tokens_user ON tokens(user_id);
CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);
