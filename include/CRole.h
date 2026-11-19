#ifndef CROle_H
#define CROle_H

#include <string>

class CRole
{
private:
    int roleId;

public:
    std::string title;
    int accessMask;
    std::string description;

    CRole();
    CRole(int id, const std::string &roleTitle, int mask, const std::string &desc);

    void create(int id, const std::string &roleTitle, int mask, const std::string &desc);
    bool hasPermission(int requiredMask) const;
    int getRoleId() const;
};

#endif