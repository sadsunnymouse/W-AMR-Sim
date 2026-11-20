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

INSERT OR IGNORE INTO roles (role_id, title, access_mask, description) VALUES
    (1, 'Администратор системы', 1, 'Управление учетными записями и ролями'),
    (2, 'Руководитель склада', 2, 'Конструирование и утверждение топологии склада'),
    (3, 'Оператор симуляции', 4, 'Параметризация, запуск и мониторинг симуляции');
