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
          const std::string &plainPassword,
          bool blocked,
          int role);

    static CUser fromStoredHash(int id,
                                const std::string &name,
                                const std::string &userLogin,
                                const std::string &storedHash,
                                bool blocked,
                                int role);

    bool authenticate(const std::string &inputLogin, const std::string &inputPassword) const;
    void setRole(int newRoleId);
    void setBlockStatus(bool blocked);
    void resetPassword(const std::string &newPlainPassword);

    int getUserId() const;
    std::string getLogin() const;
    std::string getPasswordHash() const;
    bool getIsBlocked() const;
};

#endif