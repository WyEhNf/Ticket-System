#include "../../include/system/System.hpp"

#include <iostream>

using namespace std;
namespace sjtu {
System::System(const std::string& name)
    : ticket_system(name + "_ticket_tree.data"),
      train_system(name + "_train_tree.data"),
      user_system(name + "_user_tree.data") {
}
void System::run() {
    while (true) {
        try {
            timestamp = input.GetTimestamp();
            string command = input.GetCommand();
            if (command == "add_user") {
                add_user();
            } else if (command == "login") {
                login();
            } else if (command == "logout") {
                logout();
            } else if (command == "query_profile") {
                query_profile();
            } else if (command == "modify_profile") {
                modify_profile();
            } else if (command == "add_train") {
                add_train();
            } else if (command == "delete_train") {
                delete_train();
            } else if (command == "release_train") {
                release_train();
            } else if (command == "query_train") {
                query_train();
            } else if (command == "query_ticket") {
                query_ticket();
            } else if (command == "query_transfer") {
                query_transfer_ticket();
            } else if (command == "buy_ticket") {
                buy_ticket();
            } else if (command == "refund_ticket") {
                refund_ticket();
            } else if (command == "query_ordere") {
                query_order();
            } else if (command == "clean") {
                clean();
            } else if (command == "exit") {
                break;
            }
        } catch (...) {
            cout << "-1" << endl;
        }
    }
}
void System::add_user() {
    auto key = input.GetKey();
    String cur_name;
    bool is_first = 0;
    if (key == 'c')
        cur_name = input.GetString(), key = input.GetKey();
    else
        cur_name = String(""), is_first = 1;
    User new_user;
    while (key != '\n') {
        String str;
        if (key != 'g') str = input.GetString();
        if (key == 'u') {
            new_user.UserName = str;
        } else if (key == 'i') {
            new_user.UserName = str;
        } else if (key == 'p') {
            new_user.PassWord = str;
        } else if (key == 'n') {
            new_user.name = str;
        } else if (key == 'm') {
            new_user.MailAdr = str;
        } else if (key == 'g') {
            int pri = input.GetInteger();
            new_user.privilege = pri;
        }
    }
    if (is_first) {
        new_user.privilege = 10;
        user_system.add_user(new_user);
    } else {
        User cur_user = user_system.find_user(cur_name);
        if (cur_user.privilege <= new_user.privilege) throw -1;
        if (user_system.find_user(new_user.UserName) != User()) throw -1;
        user_system.add_user(new_user);
    }
    cout << 0 << endl;
}

void System::login() {
    char key = input.GetKey();
    String user_id = input.GetString();
    key = input.GetKey();
    String password = input.GetString();
    if (!user_system.login(user_id, password)) throw -1;
    cout << 0 << endl;
}

void System::logout() {
    char key = input.GetKey();
    String user_id = input.GetString();
    if (!user_system.logout(user_id)) throw -1;
    cout << 0 << endl;
}

void System::query_profile() {
    char key = input.GetKey();
    String user_id = input.GetString();
    User u = user_system.find_user(user_id);
    if (u == User()) throw -1;
    cout << u.UserName << ' ' << u.name << ' ' << u.MailAdr << ' '
         << u.privilege << endl;
}

void System::modify_profile() {
    char key = input.GetKey();
    String cur_username = input.GetString();
    key = input.GetKey();
    String target_username = input.GetString();
    bool qualified = true;
    User cur_user = user_system.find_user(cur_username);
    User target_user = user_system.find_user(target_username);
    if (cur_username != target_username) {
        if (cur_user.privilege <= target_user.privilege) qualified = false;
    }
    key = input.GetKey();
    User original_user = target_user;
    while (key != '\n') {
        String str;
        if (key != 'g') str = input.GetString();
        if (key == 'p') {
            target_user.PassWord = str;
        } else if (key == 'n') {
            target_user.name = str;
        } else if (key == 'm') {
            target_user.MailAdr = str;
        } else if (key == 'g') {
            int pri = input.GetInteger();
            if (cur_user.privilege <= pri)
                qualified = false;
            else
                target_user.privilege = pri;
        }
    }
    if (!qualified) throw -1;
    user_system.modify_user(original_user.UserName, target_user);
    cout << 0 << endl;
}

void System::add_train() {
    auto key = input.GetKey();
    Train new_train;
    while (key != '\n') {
        String str;
        int num;
        if (key == 'i') {
            new_train.ID = input.GetString();
        } else if (key == 'n') {
            new_train.stationNum = input.GetInteger();
        } else if (key == 'm') {
            new_train.seatNum = input.GetInteger();
        } else if (key == 's') {
            new_train.stations = input.GetStringArray();
        } else if (key == 'p') {
            new_train.prices = input.GetIntegerArray();
        } else if (key == 'x') {
            new_train.startTime = input.GetTime();
        } else if (key == 't') {
            new_train.travelTimes = input.GetIntegerArray();
        } else if (key == 'o') {
            new_train.stopoverTimes = input.GetIntegerArray();
        } else if (key == 'd') {
            new_train.sale_begin = input.GetDate();
            input.Skip();
            new_train.sale_end = input.GetDate();
        } else if (key == 'y') {
            new_train.type = input.GetChar();
        }
    }
    if (!train_system.add_train(new_train)) throw -1;
    cout << 0 << endl;
}

void System::delete_train() {
    char key = input.GetKey();
    String train_id = input.GetString();
    if (!train_system.delete_train(train_id)) throw -1;
    cout << 0 << endl;
}

void System::release_train() {
    char key = input.GetKey();
    String train_id = input.GetString();
    Train t = train_system.find_train(train_id);
    if (t == Train()) throw -1;
    if (!train_system.release_train(train_id)) throw -1;
    ticket_system.add_ticket(t);
    cout << 0 << endl;
}

void System::query_train() {
    char key = input.GetKey();
    String train_id = input.GetString();
    Train train = train_system.find_train(train_id);
    key = input.GetKey();
    int date = input.GetDate();
    if (train == Train()) throw -1;
    if (date < train.sale_begin || date > train.sale_end) throw -1;
    int time = train.startTime;
    int price = 0;
    for (int i = 0; i < train.stationNum; i++) {
        String arr_time, leave_time;
        if (i == 0)
            arr_time = "xx-xx xx:xx";
        else {
            arr_time = train.realTime(time + train.travelTimes[i - 1], date);
        }
        time += train.travelTimes[i - 1];
        if (i == train.stationNum - 1)
            leave_time = "xx-xx xx:xx";
        else {
            leave_time = train.realTime(time + train.stopoverTimes[i], date);
        }
        time += train.stopoverTimes[i];
        int res_seat = (i == train.stationNum - 1)
                           ? 0
                           : train.get_seat_res(train.stations[i],
                                                train.stations[i + 1], date);
        cout << train.stations[i] << ' ' << arr_time << " -> " << leave_time
             << ' ' << price << ' ';
        if (i != train.stationNum - 1) {
            price += train.prices[i];
            cout << res_seat << endl;
        } else
            cout << 'x' << endl;
    }
}
void System::query_ticket() {
    char key = input.GetKey();
    String from_station, to_station;
    int date;
    CompareType cmp_type = PRICE;
    while (key != '\n') {
        if (key == 's') {
            from_station = input.GetString();
        } else if (key == 't') {
            to_station = input.GetString();
        } else if (key == 'd') {
            date = input.GetDate();
        } else if (key == 'p') {
            String get_type = input.GetString();
            if (get_type == (string) "time")
                cmp_type = TIME;
            else
                cmp_type = PRICE;
        }
    }
    if (!ticket_system.query_ticket(from_station, to_station, date, cmp_type))
        throw -1;
}

void System::query_transfer_ticket() {
    char key = input.GetKey();
    String from_station, to_station;
    int date;
    CompareType cmp_type = PRICE;
    while (key != '\n') {
        if (key == 's') {
            from_station = input.GetString();
        } else if (key == 't') {
            to_station = input.GetString();
        } else if (key == 'd') {
            date = input.GetDate();
        } else if (key == 'p') {
            String get_type = input.GetString();
            if (get_type == (string) "time")
                cmp_type = TIME;
            else
                cmp_type = PRICE;
        }
    }
    if (!ticket_system.query_transfer_ticket(from_station, to_station, date,
                                             cmp_type))
        throw -1;
}

void System::buy_ticket() {
    char key = input.GetKey();
    Ticket ticket;
    int num;
    bool if_wait = false;
    String user_id;
    while (key != '\n') {
        if (key == 'u') {
            user_id = input.GetString();
        } else if (key == 'i') {
            ticket.trainID = input.GetString();
        } else if (key == 'd') {
            ticket.date = input.GetDate();
        } else if (key == 'f') {
            ticket.from_station = input.GetString();
        } else if (key == 't') {
            ticket.to_station = input.GetString();
        } else if (key == 'n') {
            num = input.GetInteger();
        } else if (key == 'q') {
            char c = input.GetChar();
            if (c == 'y') if_wait = true;
        }
    }
    if (user_system.find_user(user_id) == User()) throw -1;
    if (!train_system.find_train(ticket.trainID).is_released()) throw -1;
    order result(ticket, num, user_id);
    if (!ticket_system.buy_ticket(ticket, num, if_wait, result, user_id))
        throw -1;
    user_system.add_ticket(user_id, result.ticket, result.num, result.status);
}

void System::refund_ticket() {
    char key = input.GetKey();
    String user_id;
    Ticket ticket;
    int num = 1;
    while (key != '\n') {
        if (key == 'u') {
            user_id = input.GetString();
        } else if (key == 'n') {
            num = input.GetInteger();
        }
    }
    order refunded_order = user_system.refund_ticket(user_id, num);
    if (refunded_order == order()) throw -1;
    user_system.modify_oder(refunded_order, "refunded");
    order result =
        ticket_system.refund_ticket(refunded_order.ticket, refunded_order.num);
    if (!(result == order())) user_system.modify_oder(result, "success");
    cout << 0 << endl;
}

void System::query_order() {
    char key = input.GetKey();
    String user_id = input.GetString();
    if (!user_system.query_ordered_tickets(user_id)) throw -1;
}

void System::clean() {
    train_system.~TrainSystem();
    user_system.~UserSystem();
    ticket_system.~TicketSystem();
    cout << 0 << endl;
    /*
    TODO: clean all data files & cache
    */
}
}  // namespace sjtu