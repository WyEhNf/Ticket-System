#pragma once
#include "TicketSystem.hpp"
#include "TrainSystem.hpp"  
#include "UserSystem.hpp"
#include "../parser.hpp"
using namespace std;
namespace sjtu {
    class System {
    private:
        TicketSystem ticket_system;
        TrainSystem train_system;
        UserSystem user_system;
        Input input;;
        int timestamp=0,user_cnt=0;

    public:
        ~System() = default;
        explicit System(const std::string& name);
        void run();
        void add_user();
        void login();
        void logout();
        void query_profile();
        void modify_profile();
        void add_train();
        void delete_train();
        void release_train();
        void query_train();
        void query_ticket();
        void query_transfer_ticket();
        void buy_ticket();
        void refund_ticket();
        void query_order();
        void clean();
    };

}