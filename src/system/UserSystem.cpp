#include "UserSystem.hpp"
#include "TicketSystem.hpp"

using namespace std;
namespace sjtu {

bool UserSystem::add_user(const User& new_user) {
    auto res = user_tree.find(new_user.UserName);
    if (res.size() == 0) return false;
    user_tree.insert(new_user.UserName, new_user);
    return true;
}
bool UserSystem::delete_user(String user_id) {
    auto res = user_tree.find(user_id);
    if (res.size() == 0) return false;
    user_tree.erase(res[0].index, res[0].value);
    return true;
}
bool UserSystem::login(String user_id, String password) {
    auto res = user_tree.find(user_id);
    if (res.size() == 0) return false;
    User u = res[0].value;
    if (u.PassWord != password) return false;
    if (u.logged_in) return false;
    u.logged_in = true;
    user_tree.erase(res[0].index, res[0].value);
    user_tree.insert(u.UserName, u);
    return true;
}
bool UserSystem::logout(String user_id) {
    auto res = user_tree.find(user_id);
    if (res.size() == 0) return false;
    User u = res[0].value;
    if (!u.logged_in) return false;
    u.logged_in = false;
    user_tree.erase(res[0].index, res[0].value);
    user_tree.insert(u.UserName, u);
    return true;
}
User UserSystem::find_user(String user_id) {
    auto res = user_tree.find(user_id);
    if (res.size() == 0) return User();
    User u = res[0].value;
    return u;
}
User UserSystem::modify_user(String user_id, const User& new_user) {
    auto res = user_tree.find(user_id);
    if (res.size() == 0) return User();
    user_tree.erase(res[0].index, res[0].value);
    user_tree.insert(new_user.UserName, new_user);
    return new_user;
}
bool UserSystem::query_ordered_tickets(const String& user_id) {
    auto res = user_tree.find(user_id);
    if (res.size() == 0) return false;
    User u = res[0].value;
    cout<<u.bought_tickets.size()<<endl;
    for (size_t i = 0; i < u.bought_tickets.size(); i++) {
        order t = u.bought_tickets[i];
        cout<<"["<<t.status<<"] ";
        t.ticket.printTicket(t.ticket.from_station, t.ticket.to_station);
      /*
      TODO: print order information
      */
        
    }
    return true;
}
void UserSystem::add_ticket(String user_id, const Ticket& ticket,int num,string status) {
    auto res = user_tree.find(user_id);
    if (res.size() == 0) return;
    User u = res[0].value;
    u.bought_tickets.push_back(order{ticket,num,user_id});
    u.bought_tickets[u.bought_tickets.size()-1].pos = u.bought_tickets.size()-1;
    user_tree.erase(res[0].index, res[0].value);
    user_tree.insert(u.UserName, u);
}
order UserSystem::refund_ticket(String user_id, int pos) {
    auto res = user_tree.find(user_id);
    if (res.size() == 0) return order();
    User u = res[0].value;
    if (pos < 0 || pos >= (int)u.bought_tickets.size()) return order();
    order target = u.bought_tickets[pos];
    // u.bought_tickets.erase(u.bought_tickets.begin() + pos);
    u.bought_tickets[pos].status="refunded";
    user_tree.erase(res[0].index, res[0].value);
    user_tree.insert(u.UserName, u);
    return target;
}
void UserSystem::modify_oder(order &o,string new_sta){
    auto res = user_tree.find(o.UserID);
    if (res.size() == 0) return;
    User u = res[0].value;
    u.bought_tickets[o.pos].status=new_sta;
    user_tree.erase(res[0].index, res[0].value);
    user_tree.insert(u.UserName, u);
}
}  // namespace sjtu