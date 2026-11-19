#include "CUser.h"
#include "SecurityUtils.h"

CUser::CUser()
    : userId(0),
      login(""),
      passwordHash(""),
      isBlocked(false),
      fullName(""),
      roleId(0) {}

CUser::CUser(int id,
             const std::string &name,
             const std::string &userLogin,
             const std::string &plainPassword,
             bool blocked,
             int role)
    : userId(id),
      login(userLogin),
      passwordHash(Security::hashPassword(plainPassword)),
      isBlocked(blocked),
      fullName(name),
      roleId(role) {}

CUser CUser::fromStoredHash(int id,
                            const std::string &name,
                            const std::string &userLogin,
                            const std::string &storedHash,
                            bool blocked,
                            int role)
{
    CUser user;
    user.userId = id;
    user.fullName = name;
    user.login = userLogin;
    user.passwordHash = storedHash;
    user.isBlocked = blocked;
    user.roleId = role;
    return user;
}

bool CUser::authenticate(const std::string &inputLogin, const std::string &inputPassword) const
{
    if (isBlocked)
    {
        return false;
    }
    if (login != inputLogin)
    {
        return false;
    }
    return Security::verifyPassword(inputPassword, passwordHash);
}

void CUser::setRole(int newRoleId)
{
    roleId = newRoleId;
}

void CUser::setBlockStatus(bool blocked)
{
    isBlocked = blocked;
}

void CUser::resetPassword(const std::string &newPlainPassword)
{
    passwordHash = Security::hashPassword(newPlainPassword);
}

int CUser::getUserId() const
{
    return userId;
}

std::string CUser::getLogin() const
{
    return login;
}

std::string CUser::getPasswordHash() const
{
    return passwordHash;
}

bool CUser::getIsBlocked() const
{
    return isBlocked;
}