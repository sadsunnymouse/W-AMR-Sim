#include "CRole.h"

CRole::CRole()
    : roleId(0), title(""), accessMask(0), description("") {}

CRole::CRole(int id, const std::string &roleTitle, int mask, const std::string &desc)
    : roleId(id), title(roleTitle), accessMask(mask), description(desc) {}

void CRole::create(int id, const std::string &roleTitle, int mask, const std::string &desc)
{
    roleId = id;
    title = roleTitle;
    accessMask = mask;
    description = desc;
}

bool CRole::hasPermission(int requiredMask) const
{
    if (requiredMask == 0)
    {
        return true;
    }
    return (accessMask & requiredMask) == requiredMask;
}

int CRole::getRoleId() const
{
    return roleId;
}