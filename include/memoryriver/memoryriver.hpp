#pragma once
#include <filesystem>
#include <fstream>
#include <string>

using namespace std;
namespace sjtu {
template <class T, int info_len = 2>
class MemoryRiver {
   private:
    fstream file;
    string file_name;

   public:
    MemoryRiver() = default;
    MemoryRiver(const string& fn) : file_name(fn) {
    }
    ~MemoryRiver() {
        if (file.is_open()) file.close();
    }

    void initialise(string fn = "") {
        if (!fn.empty()) file_name = fn;
        if (!filesystem::exists(file_name)) {
            file.open(file_name, ios::binary | ios::in | ios::out | ios::trunc);
            int zero = 0;
            for (int i = 0; i < info_len; i++)
                file.write(reinterpret_cast<char*>(&zero), sizeof(int));
            file.close();
        }
        file.open(file_name, ios::binary | ios::in | ios::out);
    }

    void get_info(int& x, int n) {
        file.seekg((n - 1) * sizeof(int));
        file.read(reinterpret_cast<char*>(&x), sizeof(int));
    }

    void write_info(int x, int n) {
        file.seekp((n - 1) * sizeof(int));
        file.write(reinterpret_cast<char*>(&x), sizeof(int));
    }

    int write(const T& t) {
        file.seekp(0, ios::end);
        int pos = file.tellp();
        file.write(reinterpret_cast<const char*>(&t), sizeof(T));
        return pos;
    }

    void read(T& t, int pos) {
        file.seekg(pos);
        file.read(reinterpret_cast<char*>(&t), sizeof(T));
    }

    void update(const T& t, int pos) {
        file.seekp(pos);
        file.write(reinterpret_cast<const char*>(&t), sizeof(T));
    }
    void clean_up() {
        file.close();
        filesystem::remove(file_name);
        initialise(file_name);
    }
};
}  // namespace sjtu