#include "CDatabaseManager.h"
#include <fstream>
#include <sstream>

CDatabaseManager::CDatabaseManager() : db(nullptr), dbPath(""), lastError("") {}

CDatabaseManager::~CDatabaseManager()
{
    close();
}

bool CDatabaseManager::open(const std::string &path)
{
    close();
    dbPath = path;
    int rc = sqlite3_open(path.c_str(), &db);
    if (rc != SQLITE_OK)
    {
        lastError = db ? sqlite3_errmsg(db) : "Не удалось открыть базу данных SQLite";
        if (db)
        {
            sqlite3_close(db);
            db = nullptr;
        }
        return false;
    }
    executeSql("PRAGMA foreign_keys = ON;");
    return true;
}

void CDatabaseManager::close()
{
    if (db != nullptr)
    {
        sqlite3_close(db);
        db = nullptr;
    }
}

bool CDatabaseManager::isOpen() const
{
    return db != nullptr;
}

std::string CDatabaseManager::getLastError() const
{
    return lastError;
}

bool CDatabaseManager::executeSql(const std::string &sql)
{
    if (db == nullptr)
    {
        lastError = "База данных не открыта";
        return false;
    }
    char *errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        lastError = errMsg ? errMsg : sqlite3_errmsg(db);
        if (errMsg)
        {
            sqlite3_free(errMsg);
        }
        return false;
    }
    return true;
}

bool CDatabaseManager::applySchemaFile(const std::string &schemaFilePath)
{
    if (db == nullptr)
    {
        lastError = "База данных не открыта";
        return false;
    }

    std::ifstream file(schemaFilePath);
    if (!file.is_open())
    {
        lastError = "Не удалось открыть файл схемы: " + schemaFilePath;
        return false;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return executeSql(buffer.str());
}

std::vector<CRole> CDatabaseManager::loadRoles()
{
    std::vector<CRole> result;
    if (db == nullptr)
    {
        return result;
    }

    const char *sql = "SELECT role_id, title, access_mask, description FROM roles ORDER BY role_id;";
    sqlite3_stmt *stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        lastError = sqlite3_errmsg(db);
        return result;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char *rawTitle = sqlite3_column_text(stmt, 1);
        int mask = sqlite3_column_int(stmt, 2);
        const unsigned char *rawDesc = sqlite3_column_text(stmt, 3);

        std::string title = rawTitle ? reinterpret_cast<const char *>(rawTitle) : "";
        std::string desc = rawDesc ? reinterpret_cast<const char *>(rawDesc) : "";
        result.emplace_back(id, title, mask, desc);
    }

    sqlite3_finalize(stmt);
    return result;
}

std::vector<CUser> CDatabaseManager::loadUsers()
{
    std::vector<CUser> result;
    if (db == nullptr)
    {
        return result;
    }

    const char *sql =
        "SELECT user_id, full_name, login, password_hash, is_blocked, role_id "
        "FROM users ORDER BY user_id;";
    sqlite3_stmt *stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        lastError = sqlite3_errmsg(db);
        return result;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char *rawName = sqlite3_column_text(stmt, 1);
        const unsigned char *rawLogin = sqlite3_column_text(stmt, 2);
        const unsigned char *rawHash = sqlite3_column_text(stmt, 3);
        bool blocked = sqlite3_column_int(stmt, 4) != 0;
        int roleId = sqlite3_column_int(stmt, 5);

        std::string name = rawName ? reinterpret_cast<const char *>(rawName) : "";
        std::string login = rawLogin ? reinterpret_cast<const char *>(rawLogin) : "";
        std::string hash = rawHash ? reinterpret_cast<const char *>(rawHash) : "";

        result.push_back(CUser::fromStoredHash(id, name, login, hash, blocked, roleId));
    }

    sqlite3_finalize(stmt);
    return result;
}

bool CDatabaseManager::saveUsers(const std::vector<CUser> &users)
{
    if (db == nullptr)
    {
        lastError = "База данных не открыта";
        return false;
    }

    if (!executeSql("BEGIN TRANSACTION;"))
    {
        return false;
    }

    const char *upsertSql =
        "INSERT INTO users (user_id, full_name, login, password_hash, is_blocked, role_id) "
        "VALUES (?1, ?2, ?3, ?4, ?5, ?6) "
        "ON CONFLICT(user_id) DO UPDATE SET "
        "  full_name = excluded.full_name, "
        "  login = excluded.login, "
        "  password_hash = excluded.password_hash, "
        "  is_blocked = excluded.is_blocked, "
        "  role_id = excluded.role_id;";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, upsertSql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        lastError = sqlite3_errmsg(db);
        executeSql("ROLLBACK;");
        return false;
    }

    for (const auto &u : users)
    {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);

        std::string loginStr = u.getLogin();
        std::string passHashStr = u.getPasswordHash();
        sqlite3_bind_int(stmt, 1, u.getUserId());
        sqlite3_bind_text(stmt, 2, u.fullName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, loginStr.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, passHashStr.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 5, u.getIsBlocked() ? 1 : 0);
        sqlite3_bind_int(stmt, 6, u.roleId);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            lastError = sqlite3_errmsg(db);
            sqlite3_finalize(stmt);
            executeSql("ROLLBACK;");
            return false;
        }
    }

    sqlite3_finalize(stmt);
    return executeSql("COMMIT;");
}

std::vector<CWarehouseMap> CDatabaseManager::loadMaps()
{
    std::vector<CWarehouseMap> maps;
    if (db == nullptr)
    {
        return maps;
    }

    const char *mapSql =
        "SELECT map_id, title, version, width, height, status, author_id "
        "FROM warehouse_maps ORDER BY map_id;";
    sqlite3_stmt *stmt = nullptr;

    if (sqlite3_prepare_v2(db, mapSql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        lastError = sqlite3_errmsg(db);
        return maps;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char *rawTitle = sqlite3_column_text(stmt, 1);
        int version = sqlite3_column_int(stmt, 2);
        int width = sqlite3_column_int(stmt, 3);
        int height = sqlite3_column_int(stmt, 4);
        const unsigned char *rawStatus = sqlite3_column_text(stmt, 5);
        int authorId = sqlite3_column_int(stmt, 6);

        std::string title = rawTitle ? reinterpret_cast<const char *>(rawTitle) : "Схема";
        std::string status = rawStatus ? reinterpret_cast<const char *>(rawStatus) : "Черновик";

        CWarehouseMap m;
        m.create(id, title, version, width, height, status, authorId);
        maps.push_back(m);
    }
    sqlite3_finalize(stmt);

    const char *cellSql =
        "SELECT x, y, object_type FROM warehouse_map_cells WHERE map_id = ?1;";

    for (auto &m : maps)
    {
        sqlite3_stmt *cellStmt = nullptr;
        if (sqlite3_prepare_v2(db, cellSql, -1, &cellStmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_int(cellStmt, 1, m.getMapId());
            while (sqlite3_step(cellStmt) == SQLITE_ROW)
            {
                int x = sqlite3_column_int(cellStmt, 0);
                int y = sqlite3_column_int(cellStmt, 1);
                const unsigned char *rawType = sqlite3_column_text(cellStmt, 2);
                std::string objType = rawType ? reinterpret_cast<const char *>(rawType) : "passage";

                if (y >= 0 && y < m.getHeight() && x >= 0 && x < m.getWidth())
                {
                    m.cells[y][x].setObjectType(objType);
                }
            }
            sqlite3_finalize(cellStmt);
        }
    }

    return maps;
}

int CDatabaseManager::saveMap(const CWarehouseMap &map)
{
    if (db == nullptr || map.getWidth() <= 0 || map.getHeight() <= 0)
    {
        lastError = "База данных не открыта или некорректные размеры карты";
        return -1;
    }

    if (!executeSql("BEGIN TRANSACTION;"))
    {
        return -1;
    }

    const char *insertMapSql =
        "INSERT INTO warehouse_maps (title, version, width, height, status, author_id) "
        "VALUES (?1, ?2, ?3, ?4, ?5, ?6);";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, insertMapSql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        lastError = sqlite3_errmsg(db);
        executeSql("ROLLBACK;");
        return -1;
    }

    sqlite3_bind_text(stmt, 1, map.title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, map.getVersion());
    sqlite3_bind_int(stmt, 3, map.getWidth());
    sqlite3_bind_int(stmt, 4, map.getHeight());
    std::string st = map.getStatus();
    sqlite3_bind_text(stmt, 5, st.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 6, map.getAuthorId());

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        lastError = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        executeSql("ROLLBACK;");
        return -1;
    }
    sqlite3_finalize(stmt);

    int savedMapId = static_cast<int>(sqlite3_last_insert_rowid(db));

    const char *insertCellSql =
        "INSERT OR REPLACE INTO warehouse_map_cells (map_id, x, y, object_type, is_walkable) "
        "VALUES (?1, ?2, ?3, ?4, ?5);";

    sqlite3_stmt *cellStmt = nullptr;
    if (sqlite3_prepare_v2(db, insertCellSql, -1, &cellStmt, nullptr) != SQLITE_OK)
    {
        lastError = sqlite3_errmsg(db);
        executeSql("ROLLBACK;");
        return -1;
    }

    for (int y = 0; y < map.getHeight(); ++y)
    {
        for (int x = 0; x < map.getWidth(); ++x)
        {
            sqlite3_reset(cellStmt);
            sqlite3_clear_bindings(cellStmt);

            const CCell &c = map.cells[y][x];
            sqlite3_bind_int(cellStmt, 1, savedMapId);
            sqlite3_bind_int(cellStmt, 2, x);
            sqlite3_bind_int(cellStmt, 3, y);
            sqlite3_bind_text(cellStmt, 4, c.objectType.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(cellStmt, 5, c.checkWalkable() ? 1 : 0);

            if (sqlite3_step(cellStmt) != SQLITE_DONE)
            {
                lastError = sqlite3_errmsg(db);
                sqlite3_finalize(cellStmt);
                executeSql("ROLLBACK;");
                return -1;
            }
        }
    }

    sqlite3_finalize(cellStmt);

    if (!executeSql("COMMIT;"))
    {
        return -1;
    }
    return savedMapId;
}