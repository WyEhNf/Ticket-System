#include "TrainSystem.hpp"
using namespace std;
namespace sjtu {
bool TrainSystem::add_train(const Train& new_train) {
    auto res = train_tree.find(new_train.ID);
    if (res.size() != 0) return false;
    train_tree.insert(new_train.ID, new_train);
    return true;
}
bool TrainSystem::delete_train(const String& train_id) {
    auto res = train_tree.find(train_id);
    if (res.size() == 0) return false;
    train_tree.erase(res[0].index, res[0].value);
    return true;
}
Train TrainSystem::find_train(const String& train_id) {
    auto res = train_tree.find(train_id);
    if (res.size() == 0) return Train();
    Train t = res[0].value;
    return t;
}
bool TrainSystem::release_train(const String& train_id) {
    auto res = train_tree.find(train_id);
    if (res.size() == 0) return false;
    Train t = res[0].value;
    if (t.released) return false;
    t.released = true;
    for(int i=0;i<t.sale_end-t.sale_begin+1;i++)
    {
        vector<int> seat_row;
        for(int j=0;j<t.stationNum-1;j++)
        {
            seat_row.push_back(t.seatNum);
        }
        t.seat_res.push_back(seat_row);
    }
    train_tree.erase(res[0].index, res[0].value);
    train_tree.insert(t.ID, t);
    return true;
}
// namespace sjtu
}