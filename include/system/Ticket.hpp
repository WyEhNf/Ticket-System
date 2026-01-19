// #ifdef _TICKET_HPP_
// #define _TICKET_HPP_
#pragma once
#include "Train.hpp"
#include "vector.hpp"

using namespace std;
namespace sjtu {
class Ticket {
   private:
    Train* train_ptr;
    String trainID;
    String from_station, to_station;
    int date;
    friend class User;
    friend class UserSystem;
    friend class TicketSystem;
    friend class System;

   public:
    Ticket() = default;
    Ticket(Train* train_ptr, String trainID, String from_station,
           String to_station, int date)
        : train_ptr(train_ptr),
          trainID(trainID),
          from_station(from_station),
          to_station(to_station),
          date(date) {
    }
    Ticket(const Ticket& other)
        : train_ptr(other.train_ptr),
          trainID(other.trainID),
          from_station(other.from_station),
          to_station(other.to_station),
          date(other.date) {
    }
    ~Ticket() = default;
    Ticket& operator=(const Ticket& other) {
        if (this == &other) return *this;
        train_ptr = other.train_ptr, trainID = other.trainID,
        from_station = other.from_station, to_station = other.to_station,
        date = other.date;
        return *this;
    }
    bool operator==(const Ticket& other) const {
        return trainID == other.trainID;
    }
    bool operator!=(const Ticket& other) const {
        return !(*this == other);
    }
    bool operator<(const Ticket& other) const {
        return trainID < other.trainID;
    }
    bool operator<=(const Ticket& other) const {
        return trainID <= other.trainID;
    }
    bool operator>(const Ticket& other) const {
        return !(*this <= other);
    }
    bool operator>=(const Ticket& other) const {
        return !(*this < other);
    }
    int getPrice() const {
        int from_index = -1, to_index = -1;
        for (int i = 0; i < train_ptr->stationNum; i++) {
            if (train_ptr->stations[i] == from_station) from_index = i;
            if (train_ptr->stations[i] == to_station) to_index = i;
        }
        int total_price = 0;
        for (int i = from_index; i < to_index; i++) {
            total_price += train_ptr->prices[i];
        }
        return total_price;
    }

    int getTime() const {
        int from_index = -1, to_index = -1;
        for (int i = 0; i < train_ptr->stationNum; i++) {
            if (train_ptr->stations[i] == from_station) from_index = i;
            if (train_ptr->stations[i] == to_station) to_index = i;
        }
        int time = train_ptr->startTime;
        for (int i = 1; i <= to_index; i++) {
            time += train_ptr->travelTimes[i - 1];
            if (i <= from_index) time += train_ptr->stopoverTimes[i - 1];
        }
        return time;
    }

    void printTicket(String from_station, String to_station) {
        cout << trainID << ' ' << from_station << ' '
             << train_ptr->getTime(from_station, date) << " -> " << to_station
             << ' ' << train_ptr->getTime(to_station, date) << ' ' << getPrice()
             << ' ' << train_ptr->get_seat_res(from_station, to_station, date)
             << endl;
    }

    String getID() const {
        return trainID;
    }
};
class order {
   public:
    Ticket ticket;
    int num;
    string status="";
    String UserID;
    int pos=-1;
    order() : ticket(), num(0) {
    }
    order(const Ticket& ticket, int num, String UserID)
        : ticket(ticket), num(num), UserID(UserID) {
    }
    ~order() = default;
    bool operator==(const order& other) const {
        return ticket == other.ticket;
    }
};

}  // namespace sjtu