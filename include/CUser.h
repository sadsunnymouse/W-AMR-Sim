#ifndef CUSER_H
#define CUSER_H

#include <string>

class CUser
{
private:
    int userId;
    std::string login;
    std::string passwordHash;
    bool isBlocked;

public:
    std::string fullName;
    int roleId;

    CUser();
    CUser(int id,
          const std::string &name,
          const std::string &userLogin,
          const std::string &hash,
          bool blocked,
          int assignedRoleId);

    void create(int id,
                const std::string &name,
                const std::string &userLogin,
                const std::string &hash,
                bool blocked,
                int assignedRoleId);
    bool authenticate(const std::string &inputLogin, const std::string &inputPasswordHash) const;
    void setRole(int newRoleId);
    void setBlockStatus(bool blocked);
    void resetPassword(const std::string &newPasswordHash);

    int getUserId() const;
    bool getIsBlocked() const;
};

#endif