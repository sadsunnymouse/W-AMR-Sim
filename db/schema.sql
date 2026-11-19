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

INSERT OR IGNORE INTO roles (role_id, title, access_mask, description) VALUES
    (1, 'Администратор системы', 1, 'Управление учетными записями и ролями'),
    (2, 'Руководитель склада', 2, 'Конструирование и утверждение топологии склада'),
    (3, 'Оператор симуляции', 4, 'Параметризация, запуск и мониторинг симуляции');