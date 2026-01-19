#include "../../include/system/TicketSystem.hpp"
#include <iostream>

using namespace std;
namespace sjtu {

bool TicketSystem::add_ticket(const Train& train) {
    for (int day = train.sale_begin; day <= train.sale_end; day++) {
        for (int i = 0; i < train.stationNum - 1; i++) {
            TicketKey key{day, train.stations[i]};
            for(int j=i+1;j<train.stationNum;j++)
            {
                 Ticket ticket(const_cast<Train*>(&train), train.ID,
                          train.stations[i], train.stations[j], day);
                ticket_tree.insert(key, ticket);
            }
        }
    }
    return true;
}
bool TicketSystem::Compare_with_cost(
    const BPlusTree<TicketKey, Ticket>::Key& A,
    const BPlusTree<TicketKey, Ticket>::Key& B) {
    if (A.value.getPrice() == B.value.getPrice()) {
        return A.value.trainID < B.value.trainID;
    }
    return A.value.getPrice() < B.value.getPrice();
}
bool TicketSystem::Compare_with_time(
    const BPlusTree<TicketKey, Ticket>::Key& A,
    const BPlusTree<TicketKey, Ticket>::Key& B) {
    if (A.value.getPrice() == B.value.getPrice()) {
        return A.value.trainID < B.value.trainID;
    }
    return A.value.getTime() < B.value.getTime();
}


bool TicketSystem::query_ticket(const String& from_station,
                                const String& to_station, int date,
                                CompareType cmp_type) {
    TicketKey low_key{date, from_station};
    TicketKey high_key{date, to_station};
    auto low_res = ticket_tree.find(low_key);
    if (low_res.size() == 0) return false;
    if (cmp_type == PRICE)
        low_res.sort(Compare_with_cost);
    else if (cmp_type == TIME)
        low_res.sort(Compare_with_time);
    vector<BPlusTree<TicketKey, Ticket>::Key> final_res;
    for (int i = 0; i < low_res.size(); i++) {
        Ticket t = low_res[i].value;
        if (t.to_station == to_station) {
            final_res.push_back(low_res[i]);
        }
    }
    if (final_res.empty()) return false;
    cout << final_res.size() << endl;
    for (auto& item : final_res) {
        Ticket t = item.value;
       t.printTicket(from_station, to_station);
    }
    return true;
}
bool TicketSystem::query_transfer_ticket(const String& from_station,
                                         const String& to_station, int date,
                                         CompareType cmp_type) {
    TicketKey low_key{date, from_station};
    auto low_res = ticket_tree.find(low_key);
    if (low_res.size() == 0) return false;
    vector<pair<BPlusTree<TicketKey, Ticket>::Key,
                BPlusTree<TicketKey, Ticket>::Key>>
        final_res;
    for (int i = 0; i < low_res.size(); i++) {
        Ticket t1 = low_res[i].value;
        TicketKey mid_key{date, t1.to_station};
        auto mid_res = ticket_tree.find(mid_key);
        for (int j = 0; j < mid_res.size(); j++) {
            Ticket t2 = mid_res[j].value;
            if (t2.to_station == to_station) {
                final_res.push_back({low_res[i], mid_res[j]});
            }
        }
    }
    if (final_res.empty()) return false;
    pair<BPlusTree<TicketKey, Ticket>::Key, BPlusTree<TicketKey, Ticket>::Key>
        ans = final_res[0];
    for (int i = 1; i < final_res.size(); i++) {
        Ticket t1_a = final_res[i].first.value;
        Ticket t2_a = final_res[i].second.value;
        Ticket t1_b = ans.first.value;
        Ticket t2_b = ans.second.value;
        if (cmp_type == PRICE) {
            if (t1_a.getPrice() + t2_a.getPrice() <
                t1_b.getPrice() + t2_b.getPrice()) {
                ans.first = final_res[i].first;
                ans.second = final_res[i].second;
            } else if (t1_a.getPrice() + t2_a.getPrice() ==
                           t1_b.getPrice() + t2_b.getPrice() &&
                       t1_a.getTime() + t2_a.getTime() <
                           t1_b.getTime() + t2_b.getTime()) {
                ans.first = final_res[i].first;
                ans.second = final_res[i].second;
            }
        } else if (cmp_type == TIME) {
            if (t1_a.getTime() + t2_a.getTime() <
                t1_b.getTime() + t2_b.getTime()) {
                ans.first = final_res[i].first;
                ans.second = final_res[i].second;
            } else if (t1_a.getTime() + t2_a.getTime() ==
                           t1_b.getTime() + t2_b.getTime() &&
                       t1_a.getPrice() + t2_a.getPrice() <
                           t1_b.getPrice() + t2_b.getPrice()) {
                ans.first = final_res[i].first;
                ans.second = final_res[i].second;
            }
        }
        if (t1_a.getTime() + t2_a.getTime() ==
                t1_b.getTime() + t2_b.getTime() &&
            t1_a.getPrice() + t2_a.getPrice() ==
                t1_b.getPrice() + t2_b.getPrice()) {
            if (t1_a.trainID < t1_b.trainID) {
                ans.first = final_res[i].first;
                ans.second = final_res[i].second;
            } else if (t1_a.trainID == t1_b.trainID &&
                       t2_a.trainID < t2_b.trainID) {
                ans.first = final_res[i].first;
                ans.second = final_res[i].second;
            }
        }
    }
    ans.first.value.printTicket(ans.first.value.from_station, ans.first.value.to_station);
    ans.second.value.printTicket(ans.second.value.from_station, ans.second.value.to_station);
    return true;
}
bool TicketSystem::buy_ticket(const Ticket& ticket, int num, bool if_wait,
                              order& result, const String& UserID) {
    Train tr=*ticket.train_ptr;
    int seat_res=tr.get_seat_res(ticket.from_station,ticket.to_station,
                                    ticket.date);
    if(seat_res<num)
     {
        if(!if_wait) return false;
        else
        {
            waiting_list.push_back(order{ticket,num,UserID});
            result.ticket=ticket;
            result.num=num; 
            result.UserID=UserID;
            result.status="queue";
            cout<<"queue"<<std::endl;
            return true;
        }
     }
    else {
        tr.update_seat_res(ticket.from_station, ticket.to_station, ticket.date, num);
        result.ticket=ticket;
        result.num=num;
        result.UserID=UserID;
        result.status="success";
        cout<<ticket.getPrice()*num<<endl;
        return true;
    }
}
order TicketSystem::refund_ticket(const Ticket& ticket, int num) {
    Train tr=*ticket.train_ptr;
    tr.update_seat_res(ticket.from_station, ticket.to_station, ticket.date,-num);
    // process waiting list
    order res;
    for (auto it = waiting_list.begin(); it != waiting_list.end();) {
        if(tr.get_seat_res(ticket.from_station, ticket.to_station, ticket.date) >= it->num) {
            TicketSystem::buy_ticket(it->ticket, it->num, false, res,it->UserID);
            it = waiting_list.erase(it);
        } else {
            ++it;
        }
    }
    return res;
}
}  // namespace sjtu