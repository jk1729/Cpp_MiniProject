#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <ctime>
#include <algorithm>
#include <emscripten.h>

using namespace std;

// ===== Seat =====
class Seat {
    bool booked;
public:
    Seat() : booked(false) {}
    bool isAvailable() const { return !booked; }
    void book() { booked = true; }
    void cancel() { booked = false; }
};

// ===== Show =====
class Show {
    string movie, time;
    vector<vector<Seat>> seats;

public:
    Show(string m = "", string t = "") : movie(m), time(t) {
        seats.resize(5, vector<Seat>(10));
    }

    string getMovie() { return movie; }
    string getTime() { return time; }

    void display() {
        cout << "\n🎬 " << movie << " | 🕒 " << time << "\n\n";

        cout << "    ";
        for (int i = 1; i <= 10; i++) cout << i << " ";
        cout << "\n";

        for (int r = 0; r < 5; r++) {
            cout << char('A' + r) << " | ";
            for (int c = 0; c < 10; c++) {
                cout << (seats[r][c].isAvailable() ? ". " : "X ");
            }
            cout << "\n";
        }
    }

    bool isAvailable(int r, int c) {
        return seats[r][c].isAvailable();
    }

    void book(int r, int c) {
        seats[r][c].book();
    }

    void cancel(int r, int c) {
        seats[r][c].cancel();
    }

    int available() {
        int count = 0;
        for (auto &row : seats)
            for (auto &s : row)
                if (s.isAvailable()) count++;
        return count;
    }

    double occupancy() {
        return (50 - available()) * 100.0 / 50;
    }
};

// ===== Booking =====
class Booking {
    string id, name, movie, time, date;
    vector<string> seats;
    double amount;

public:
    Booking() {}

    Booking(string i, string n, string m, string t,
            vector<string> s, double a, string d)
        : id(i), name(n), movie(m), time(t),
          seats(s), amount(a), date(d) {}

    string getID() { return id; }
    vector<string> getSeats() { return seats; }
    string getMovie() { return movie; }
    string getTime() { return time; }

    void display() {
        cout << "\n========== 🎟 RECEIPT ==========\n";
        cout << "Booking ID: " << id << "\n";
        cout << "Name: " << name << "\n";
        cout << "Movie: " << movie << " | " << time << "\n";
        cout << "Seats: ";
        for (auto &s : seats) cout << s << " ";
        cout << "\nAmount: ₹" << amount;
        cout << "\nDate: " << date;
        cout << "\n================================\n";
    }
};

// ===== Theatre =====
class TheatreSystem {
    vector<Show> shows;
    map<string, Booking> bookings;
    int counter = 0;

    int selectedShow = -1;
    string currentName;
    vector<string> selectedSeats;
    bool readyToConfirm = false; // 🔥 NEW

    string getTimeNow() {
        time_t now = time(0);
        string t = ctime(&now);
        t.pop_back();
        return t;
    }

    string generateID() {
        return "BK" + to_string(++counter);
    }

    bool parseSeat(string input, int &r, int &c) {
        if (input.length() < 2) return false;

        char row = toupper(input[0]);
        if (row < 'A' || row >= 'A' + 5) return false;

        r = row - 'A';

        try { c = stoi(input.substr(1)) - 1; }
        catch (...) { return false; }

        return (c >= 0 && c < 10);
    }

public:
    TheatreSystem() {
        shows.push_back(Show("Oppenheimer", "9:00 AM"));
        shows.push_back(Show("Oppenheimer", "1:00 PM"));
        shows.push_back(Show("Oppenheimer", "5:00 PM"));
        shows.push_back(Show("Interstellar", "10:00 AM"));
        shows.push_back(Show("Interstellar", "6:00 PM"));
        shows.push_back(Show("The Batman", "3:00 PM"));
        shows.push_back(Show("Avengers: Endgame", "9:00 PM"));
        shows.push_back(Show("Joker", "4:00 PM"));
    }

    void listShows() {
        cout << "\n🎬 AVAILABLE SHOWS:\n\n";
        for (int i = 0; i < shows.size(); i++) {
            cout << i + 1 << ". "
                 << shows[i].getMovie()
                 << " | 🕒 " << shows[i].getTime() << "\n";
        }
    }

    void showSeats(int index) {
        if (index < 1 || index > shows.size()) {
            cout << "\n❌ Invalid show!\n";
            return;
        }
        shows[index - 1].display();
    }

    void startBooking(int index) {
        if (index < 1 || index > shows.size()) {
            cout << "\n❌ Invalid show!\n";
            return;
        }

        selectedShow = index - 1;
        selectedSeats.clear();
        readyToConfirm = false;

        cout << "\n🎯 Selected Show:\n";
        shows[selectedShow].display();

        cout << "\n👉 Enter your name:\n";
    }

    void setName(string name) {
        currentName = name;

        cout << "\n👉 Now enter seats (Example: A1, B2)\n";
        cout << "👉 Type 'done' when finished\n";
    }

    void addSeat(string seat) {

        // ===== CONFIRM =====
        if (seat == "confirm") {
            if (!readyToConfirm) {
                cout << "\n❌ Finish seat selection first (type 'done')\n";
                return;
            }
            confirmBooking();
            return;
        }

        // ===== CANCEL =====
        if (seat == "cancel") {
            cout << "\n❌ Booking cancelled!\n";
            selectedSeats.clear();
            readyToConfirm = false;
            return;
        }

        // ===== DONE =====
        if (seat == "done") {
            if (selectedSeats.empty()) {
                cout << "\n❌ No seats selected!\n";
                return;
            }

            readyToConfirm = true;

            cout << "\n========== 📋 BOOKING SUMMARY ==========\n";
            cout << "Name: " << currentName << "\n";
            cout << "Movie: " << shows[selectedShow].getMovie() << "\n";
            cout << "Time: " << shows[selectedShow].getTime() << "\n";

            cout << "Seats: ";
            for (auto &s : selectedSeats) cout << s << " ";

            cout << "\nTotal: ₹" << selectedSeats.size() * 250 << "\n";
            cout << "========================================\n";

            cout << "\n👉 Type 'confirm' to book OR 'cancel'\n";
            return;
        }

        // ===== NORMAL SEAT =====
        int r, c;
        if (!parseSeat(seat, r, c)) {
            cout << "\n❌ Invalid seat! (Example: A1)\n";
            return;
        }

        if (!shows[selectedShow].isAvailable(r, c)) {
            cout << "\n❌ Seat already booked!\n";
            return;
        }

        if (find(selectedSeats.begin(), selectedSeats.end(), seat) != selectedSeats.end()) {
            cout << "\n❌ Seat already selected!\n";
            return;
        }

        selectedSeats.push_back(seat);

        cout << "✅ Added: " << seat << "\n";
        cout << "👉 Add more or type 'done'\n";

        shows[selectedShow].display();
    }

    void confirmBooking() {
        Show &sh = shows[selectedShow];

        sort(selectedSeats.begin(), selectedSeats.end());

        for (auto &s : selectedSeats) {
            int r, c;
            parseSeat(s, r, c);
            sh.book(r, c);
        }

        string id = generateID();

        Booking b(id, currentName, sh.getMovie(), sh.getTime(),
                  selectedSeats, selectedSeats.size() * 250, getTimeNow());

        bookings[id] = b;

        b.display();

        // RESET
        selectedSeats.clear();
        readyToConfirm = false;
    }

    void searchBooking(string id) {
        if (bookings.count(id))
            bookings[id].display();
        else
            cout << "\n❌ Booking not found!\n";
    }

    void cancelBooking(string id) {
        if (!bookings.count(id)) {
            cout << "\n❌ Booking not found!\n";
            return;
        }

        Booking b = bookings[id];

        for (auto &sh : shows) {
            if (sh.getMovie() == b.getMovie() &&
                sh.getTime() == b.getTime()) {

                for (auto &s : b.getSeats()) {
                    int r, c;
                    parseSeat(s, r, c);
                    sh.cancel(r, c);
                }
            }
        }

        bookings.erase(id);
        cout << "\n✅ Booking Cancelled!\n";
    }

    void report() {
        cout << "\n📊 OCCUPANCY REPORT:\n\n";
        for (auto &sh : shows) {
            cout << sh.getMovie() << " | " << sh.getTime()
                 << " | Available: " << sh.available()
                 << " | Occupancy: " << sh.occupancy() << "%\n";
        }
    }

    void viewAll() {
        if (bookings.empty()) {
            cout << "\n📭 No bookings found!\n";
            return;
        }

        for (auto &p : bookings)
            p.second.display();
    }
};

// ===== GLOBAL =====
TheatreSystem theatre;

// ===== WASM EXPORT =====
extern "C" {

EMSCRIPTEN_KEEPALIVE void listShows(){ theatre.listShows(); }
EMSCRIPTEN_KEEPALIVE void showSeats(int i){ theatre.showSeats(i); }
EMSCRIPTEN_KEEPALIVE void startBooking(int i){ theatre.startBooking(i); }
EMSCRIPTEN_KEEPALIVE void setName(const char* n){ theatre.setName(n); }
EMSCRIPTEN_KEEPALIVE void addSeat(const char* s){ theatre.addSeat(s); }
EMSCRIPTEN_KEEPALIVE void searchBooking(const char* id){ theatre.searchBooking(id); }
EMSCRIPTEN_KEEPALIVE void cancelBooking(const char* id){ theatre.cancelBooking(id); }
EMSCRIPTEN_KEEPALIVE void report(){ theatre.report(); }
EMSCRIPTEN_KEEPALIVE void viewAll(){ theatre.viewAll(); }

}