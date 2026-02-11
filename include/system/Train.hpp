// #ifdef _TRAIN_HPP_
// #define _TRAIN_HPP_
#pragma once
#include "../container/String.hpp"
#include "../container/vector.hpp"
using namespace std;
namespace sjtu {
class Train {
   public:
    char type;
    String ID;
    bool released = false;
    int stationNum, seatNum;
    int startTime, sale_begin, sale_end;
    vector<String> stations;
    vector<vector<int> > seat_res;
    vector<int> prices, travelTimes, stopoverTimes;
    vector<int> date;
    friend class Ticket;
    friend class TrainSystem;
    friend class UserSystem;
    friend class TicketSystem;
    friend class System;

    Train() = default;
    Train(char type, String ID, int stationNum, int seatNum, int startTime,
          int sale_begin, int sale_end, vector<String> stations,
          vector<int> prices, vector<int> travelTimes,
          vector<int> stopoverTimes)
        : type(type),
          ID(ID),
          stationNum(stationNum),
          seatNum(seatNum),
          startTime(startTime),
          sale_begin(sale_begin),
          sale_end(sale_end),
          stations(stations),
          prices(prices),
          travelTimes(travelTimes),
          stopoverTimes(stopoverTimes) {
    }
    Train(const Train& other)
        : type(other.type),
          ID(other.ID),
          stationNum(other.stationNum),
          seatNum(other.seatNum),
          startTime(other.startTime),
          sale_begin(other.sale_begin),
          sale_end(other.sale_end),
          stations(other.stations),
          prices(other.prices),
          travelTimes(other.travelTimes),
          seat_res(other.seat_res),
          stopoverTimes(other.stopoverTimes),
          released(other.released),
          date(other.date) {
    }
    ~Train() = default;
    Train& operator=(const Train& other) {
        if (this == &other) return *this;
        type = other.type, ID = other.ID, stationNum = other.stationNum,
        seatNum = other.seatNum;
        startTime = other.startTime, sale_begin = other.sale_begin,
        sale_end = other.sale_end;
        stations = other.stations, prices = other.prices,
        travelTimes = other.travelTimes;
        stopoverTimes = other.stopoverTimes;
        seat_res = other.seat_res;
        released = other.released;
        date = other.date;
        return *this;
    }
    void initialise() {
        int time = startTime;
        int base = startTime;
        date.push_back(0);
        for (int i = 1; i < stationNum; i++) {
            time += travelTimes[i - 1];
            if (i != stationNum - 1) time += stopoverTimes[i - 1];
            date.push_back((time / 1440 - base / 1440));
        }
        // for(auto x:date) cout<<x<<' ';
        // cout<<'\n';
    }
    bool operator==(const Train& other) const {
        return ID == other.ID;
    }
    bool operator!=(const Train& other) const {
        return ID != other.ID;
    }
    bool operator<(const Train& other) const {
        return ID < other.ID;
    }
    bool operator<=(const Train& other) const {
        return ID <= other.ID;
    }
    bool operator>(const Train& other) const {
        return !(*this <= other);
    }
    bool operator>=(const Train& other) const {
        return !(*this < other);
    }
    void release() {
        released = true;
    }
    bool is_released() {
        return released;
    }
    int get_seat_res(String from_station, String to_station, int date) {
        int day_index = date - sale_begin;
        int from, to;
        for (int i = 0; i < stationNum; i++) {
            if (stations[i] == from_station) from = i;
            if (stations[i] == to_station) to = i;
        }
        // std::cerr<<from<<' '<<to<<'\n';
        // std::cerr<<day_index<<'\n';
        // std::cerr << "get_seat_res " << from << ' ' << to << ' ' << day_index
        //           << ' ' << seat_res.size() << endl;
        int min_seat = seat_res[day_index][from];
        for (int i = from; i < to; i++) {
            if (seat_res[day_index][i] < min_seat)
                min_seat = seat_res[day_index][i];
        }
        return min_seat;
    }
    void update_seat_res(String from_station, String to_station, int date,
                         int num) {
        int day_index = date - sale_begin;
        int from, to;
        for (int i = 0; i < stationNum; i++) {
            if (stations[i] == from_station) from = i;
            if (stations[i] == to_station) to = i;
        }
        for (int i = from; i < to; i++) {
            seat_res[day_index][i] -= num;
        }
    }
    string int_to_date(int date) const {
        if (date <= 30)
            return "06-" + String::FromInt(date);
        else if (date <= 61)
            return "07-" + String::FromInt(date - 30);
        else
            return "08-" + String::FromInt(date - 61);
    }
    string int_to_time(int time) const {
        int hour = time / 60;
        int minute = time % 60;
        return String::FromInt(hour) + ":" + String::FromInt(minute);
    }
    String realTime(int time, int date) const {
        int realdate = date + time / 1440;
        int realtime = time % 1440;
        return int_to_date(realdate) + " " + int_to_time(realtime);
    }
    String getTime(String station, int date,bool isArrive=1) {
        int time = startTime;
        if (stations[0] == station) return realTime(time, date);
        for (int i = 1; i < stationNum; i++) {
            // std::cerr<<"station "<<stations[i]<<endl;
            time += travelTimes[i - 1];
            if (stations[i] == station&&!isArrive) return realTime(time, date);
            if (i != stationNum - 1) time += stopoverTimes[i - 1];
             if (stations[i] == station&&isArrive) return realTime(time, date);
        }
        return String("error!");
    }
};

}  // namespace sjtu
// #endif