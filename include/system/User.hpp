#pragma once
#include "../container/String.hpp"
#include "Ticket.hpp"
using namespace std;
namespace sjtu {
class User {
   private:
    int privilege;
    String UserName, PassWord, MailAdr;
    String name;
    vector<order> bought_tickets;
    bool logged_in = false;
    friend class UserSystem;
    friend class System;
    friend class Ticket;

   public:
    User() : privilege(-1), name(),UserName(), PassWord(), MailAdr() {
    }
    User(int privilege, String UserName, String PassWord, String name,String MailAdr)
        : privilege(privilege),
          PassWord(PassWord),
          UserName(UserName),
          MailAdr(MailAdr),
          name(name){
    }
    User(const User& other)
        : privilege(other.privilege),
          UserName(other.UserName),
          PassWord(other.PassWord),
          name(other.name),
          MailAdr(other.MailAdr),
          bought_tickets(other.bought_tickets),
          logged_in(other.logged_in){
    }
    ~User() {
    }
    User& operator=(const User& other) {
        if (this == &other) return *this;
        privilege = other.privilege;
        UserName = other.UserName, PassWord = other.PassWord;
        MailAdr = other.MailAdr;
        bought_tickets = other.bought_tickets;
        logged_in = other.logged_in;
        name = other.name;
        return *this;
    }
    bool operator==(const User& other) const{
        return UserName == other.UserName;
    }
    bool operator!=(const User& other) const{
        return UserName != other.UserName;
    }
    bool operator<(const User& other) const{
        return UserName < other.UserName;
    }
    bool operator<=(const User& other) const{
        return UserName <= other.UserName;
    }
    bool operator>(const User& other) const{
        return !(*this <= other);
    }
    bool operator>=(const User& other) const{
        return !(*this < other);
    }
};

}  // namespace sjtu