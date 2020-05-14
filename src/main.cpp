#include "data_base_manager.h"

int main(){
    struct Default
    {
        int id;
        char description [249];
        char city [30];
        char state [2];
        char weather [35];

        void show(){
            std:: cout << "id: " << id << std::endl;
            std:: cout << "desc: " << description << std::endl;
            std:: cout << "city: " << city << std::endl;
            std:: cout << "state: " << state << std::endl;
            std:: cout << "weather: " << weather << std::endl;
            std:: cout << "-------------------------------" << std::endl;        }
    };

    bd2::DataBase<Default, int> db = bd2::DataBase<Default, int>();
    db.loadFromExternalFile("data_short.bin");
    Default d;
    db.readRecord(d, 3);
    d.show();
}
