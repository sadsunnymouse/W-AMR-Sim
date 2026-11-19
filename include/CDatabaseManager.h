#ifndef CDATABASEMANAGER_H
#define CDATABASEMANAGER_H

#include "CRole.h"
#include "CUser.h"
#include "CWarehouseMap.h"
#include <sqlite3.h>
#include <string>
#include <vector>

class CDatabaseManager
{
private:
    sqlite3 *db;
    std::string dbPath;
    std::string lastError;

    bool executeSql(const std::string &sql);

public:
    CDatabaseManager();
    ~CDatabaseManager();

    CDatabaseManager(const CDatabaseManager &) = delete;
    CDatabaseManager &operator=(const CDatabaseManager &) = delete;

    bool open(const std::string &path);
    void close();
    bool isOpen() const;
    std::string getLastError() const;

    bool applySchemaFile(const std::string &schemaFilePath);

    std::vector<CRole> loadRoles();
    std::vector<CUser> loadUsers();
    bool saveUsers(const std::vector<CUser> &users);

    std::vector<CWarehouseMap> loadMaps();
    int saveMap(const CWarehouseMap &map);
};

#endif