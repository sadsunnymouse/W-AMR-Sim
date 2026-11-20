CREATE TABLE IF NOT EXISTS roles (
    role_id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL UNIQUE,
    access_mask INTEGER NOT NULL DEFAULT 0,
    description TEXT NOT NULL DEFAULT ''
);

CREATE TABLE IF NOT EXISTS users (
    user_id INTEGER PRIMARY KEY AUTOINCREMENT,
    full_name TEXT NOT NULL,
    login TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL,
    is_blocked INTEGER NOT NULL DEFAULT 0,
    role_id INTEGER NOT NULL,
    FOREIGN KEY (role_id) REFERENCES roles(role_id)
);

CREATE TABLE IF NOT EXISTS warehouse_maps (
    map_id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    version INTEGER NOT NULL DEFAULT 1,
    width INTEGER NOT NULL,
    height INTEGER NOT NULL,
    status TEXT NOT NULL DEFAULT 'Черновик',
    author_id INTEGER NOT NULL DEFAULT 0
);

CREATE TABLE IF NOT EXISTS warehouse_map_cells (
    map_id INTEGER NOT NULL,
    x INTEGER NOT NULL,
    y INTEGER NOT NULL,
    object_type TEXT NOT NULL,
    is_walkable INTEGER NOT NULL DEFAULT 1,
    PRIMARY KEY (map_id, x, y),
    FOREIGN KEY (map_id) REFERENCES warehouse_maps(map_id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS simulation_sessions (
    session_id INTEGER PRIMARY KEY AUTOINCREMENT,
    map_id INTEGER NOT NULL,
    operator_id INTEGER NOT NULL,
    started_at TEXT NOT NULL,
    status TEXT NOT NULL,
    total_ticks INTEGER NOT NULL DEFAULT 0,
    completed_tasks INTEGER NOT NULL DEFAULT 0,
    total_tasks INTEGER NOT NULL DEFAULT 0,
    FOREIGN KEY (map_id) REFERENCES warehouse_maps(map_id) ON DELETE CASCADE,
    FOREIGN KEY (operator_id) REFERENCES users(user_id) ON DELETE RESTRICT
);

CREATE TABLE IF NOT EXISTS simulation_reports (
    report_id INTEGER PRIMARY KEY AUTOINCREMENT,
    session_id INTEGER NOT NULL,
    created_at TEXT NOT NULL,
    total_time_sec REAL NOT NULL DEFAULT 0.0,
    total_ticks INTEGER NOT NULL DEFAULT 0,
    total_distance REAL NOT NULL DEFAULT 0.0,
    battery_consumed REAL NOT NULL DEFAULT 0.0,
    recharge_count INTEGER NOT NULL DEFAULT 0,
    idle_ticks INTEGER NOT NULL DEFAULT 0,
    report_file_path TEXT NOT NULL DEFAULT '',
    FOREIGN KEY (session_id) REFERENCES simulation_sessions(session_id) ON DELETE CASCADE
);

INSERT OR IGNORE INTO roles (role_id, title, access_mask, description) VALUES
    (1, 'Администратор системы', 1, 'Управление учетными записями и ролями'),
    (2, 'Руководитель склада', 2, 'Конструирование и утверждение топологии склада'),
    (3, 'Оператор симуляции', 4, 'Параметризация, запуск и мониторинг симуляции');
