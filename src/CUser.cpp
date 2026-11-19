#include "CUser.h"

CUser::CUser()
    : userId(0), login(""), passwordHash(""), isBlocked(false), fullName(""), roleId(0) {}

CUser::CUser(int id,
             const std::string &name,
             const std::string &userLogin,
             const std::string &hash,
             bool blocked,
             int assignedRoleId)
    : userId(id),
      login(userLogin),
      passwordHash(hash),
      isBlocked(blocked),
      fullName(name),
      roleId(assignedRoleId) {}

void CUser::create(int id,
                   const std::string &name,
                   const std::string &userLogin,
                   const std::string &hash,
                   bool blocked,
                   int assignedRoleId)
{
    userId = id;
    fullName = name;
    login = userLogin;
    passwordHash = hash;
    isBlocked = blocked;
    roleId = assignedRoleId;
}

bool CUser::authenticate(const std::string &inputLogin, const std::string &inputPasswordHash) const
{
    if (isBlocked)
    {
        return false;
    }
    return (login == inputLogin) && (passwordHash == inputPasswordHash);
}

void CUser::setRole(int newRoleId)
{
    roleId = newRoleId;
}

void CUser::setBlockStatus(bool blocked)
{
    isBlocked = blocked;
}

void CUser::resetPassword(const std::string &newPasswordHash)
{
    passwordHash = newPasswordHash;
}

int CUser::getUserId() const
{
    return userId;
}

bool CUser::getIsBlocked() const
{
    return isBlocked;
}