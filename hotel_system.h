#ifndef HOTEL_SYSTEM_H
#define HOTEL_SYSTEM_H

#include <string>
#include <vector>
#include <queue>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include "crow_all.h"

// ==================== UTILITY FUNCTIONS ====================
std::string getCurrentDateTime() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    std::stringstream ss;
    ss << std::setfill('0') 
       << std::setw(2) << ltm->tm_mday << "/"
       << std::setw(2) << (1 + ltm->tm_mon) << "/"
       << (1900 + ltm->tm_year) << " "
       << std::setw(2) << ltm->tm_hour << ":"
       << std::setw(2) << ltm->tm_min;
    return ss.str();
}

int generateID() {
    static int id = 1000;
    return id++;
}

// ==================== CORE DATA CLASSES ====================

class Room {
public:
    int roomNumber;
    std::string type;        // "Single", "Double", "Suite", "Deluxe"
    double pricePerNight;
    std::string status;      // "Available", "Occupied", "Maintenance", "Reserved"
    int floor;
    std::string features;    // "AC, TV, WiFi, Mini-bar"
    
    Room() : roomNumber(0), pricePerNight(0.0), floor(0) {}
    
    Room(int num, std::string t, double price, std::string stat, int flr, std::string feat)
        : roomNumber(num), type(t), pricePerNight(price), status(stat), floor(flr), features(feat) {}
    
    crow::json::wvalue toJSON() const {
        crow::json::wvalue json;
        json["roomNumber"] = roomNumber;
        json["type"] = type;
        json["pricePerNight"] = pricePerNight;
        json["status"] = status;
        json["floor"] = floor;
        json["features"] = features;
        return json;
    }
    
    std::string toFileString() const {
        return std::to_string(roomNumber) + "|" + type + "|" + 
               std::to_string(pricePerNight) + "|" + status + "|" + 
               std::to_string(floor) + "|" + features;
    }
    
    static Room fromFileString(const std::string& line) {
        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;
        
        while (std::getline(ss, token, '|')) {
            tokens.push_back(token);
        }
        
        if (tokens.size() >= 6) {
            return Room(std::stoi(tokens[0]), tokens[1], std::stod(tokens[2]), 
                       tokens[3], std::stoi(tokens[4]), tokens[5]);
        }
        return Room();
    }
};

class User {
public:
    std::string userId;
    std::string password;
    std::string name;
    std::string email;
    std::string phone;
    std::string role;  // "user", "admin", "staff"
    
    User() {}
    
    User(std::string id, std::string pass, std::string n, std::string e, 
         std::string p, std::string r)
        : userId(id), password(pass), name(n), email(e), phone(p), role(r) {}
    
    crow::json::wvalue toJSON() const {
        crow::json::wvalue json;
        json["userId"] = userId;
        json["name"] = name;
        json["email"] = email;
        json["phone"] = phone;
        json["role"] = role;
        return json;
    }
    
    std::string toFileString() const {
        return userId + "|" + password + "|" + name + "|" + email + "|" + phone + "|" + role;
    }
    
    static User fromFileString(const std::string& line) {
        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;
        
        while (std::getline(ss, token, '|')) {
            tokens.push_back(token);
        }
        
        if (tokens.size() >= 6) {
            return User(tokens[0], tokens[1], tokens[2], tokens[3], tokens[4], tokens[5]);
        }
        return User();
    }
};

class Booking {
public:
    int bookingId;
    std::string userId;
    int roomNumber;
    std::string checkInDate;
    std::string checkOutDate;
    int nights;
    double totalAmount;
    std::string status;  // "Pending", "Confirmed", "CheckedIn", "CheckedOut", "Cancelled"
    std::string bookingDate;
    
    Booking() : bookingId(0), roomNumber(0), nights(0), totalAmount(0.0) {}
    
    Booking(int id, std::string uid, int room, std::string cin, std::string cout, 
            int n, double amt, std::string stat, std::string bdate)
        : bookingId(id), userId(uid), roomNumber(room), checkInDate(cin), 
          checkOutDate(cout), nights(n), totalAmount(amt), status(stat), bookingDate(bdate) {}
    
    crow::json::wvalue toJSON() const {
        crow::json::wvalue json;
        json["bookingId"] = bookingId;
        json["userId"] = userId;
        json["roomNumber"] = roomNumber;
        json["checkInDate"] = checkInDate;
        json["checkOutDate"] = checkOutDate;
        json["nights"] = nights;
        json["totalAmount"] = totalAmount;
        json["status"] = status;
        json["bookingDate"] = bookingDate;
        return json;
    }
    
    std::string toFileString() const {
        return std::to_string(bookingId) + "|" + userId + "|" + std::to_string(roomNumber) + "|" +
               checkInDate + "|" + checkOutDate + "|" + std::to_string(nights) + "|" +
               std::to_string(totalAmount) + "|" + status + "|" + bookingDate;
    }
    
    static Booking fromFileString(const std::string& line) {
        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;
        
        while (std::getline(ss, token, '|')) {
            tokens.push_back(token);
        }
        
        if (tokens.size() >= 9) {
            return Booking(std::stoi(tokens[0]), tokens[1], std::stoi(tokens[2]), 
                          tokens[3], tokens[4], std::stoi(tokens[5]), 
                          std::stod(tokens[6]), tokens[7], tokens[8]);
        }
        return Booking();
    }
};

class FoodOrder {
public:
    int orderId;
    std::string userId;
    int roomNumber;
    std::vector<std::pair<std::string, int>> items; // item name, quantity
    double totalPrice;
    std::string status;  // "Pending", "Preparing", "Delivered", "Cancelled"
    std::string orderTime;
    
    FoodOrder() : orderId(0), roomNumber(0), totalPrice(0.0) {}
    
    crow::json::wvalue toJSON() const {
        crow::json::wvalue json;
        json["orderId"] = orderId;
        json["userId"] = userId;
        json["roomNumber"] = roomNumber;
        json["totalPrice"] = totalPrice;
        json["status"] = status;
        json["orderTime"] = orderTime;
        
        std::vector<crow::json::wvalue> itemsJson;
        for (const auto& item : items) {
            crow::json::wvalue itemJson;
            itemJson["name"] = item.first;
            itemJson["quantity"] = item.second;
            itemsJson.push_back(std::move(itemJson));
        }
        json["items"] = std::move(itemsJson);
        
        return json;
    }
    
    std::string toFileString() const {
        std::string itemsStr;
        for (size_t i = 0; i < items.size(); i++) {
            itemsStr += items[i].first + ":" + std::to_string(items[i].second);
            if (i < items.size() - 1) itemsStr += ",";
        }
        return std::to_string(orderId) + "|" + userId + "|" + std::to_string(roomNumber) + "|" +
               itemsStr + "|" + std::to_string(totalPrice) + "|" + status + "|" + orderTime;
    }
};

class ServiceRequest {
public:
    int requestId;
    int roomNumber;
    std::string type;      // "Cleaning", "Maintenance", "Room Service", "Emergency"
    std::string description;
    int priority;          // 1=High, 2=Medium, 3=Low
    std::string status;    // "Pending", "InProgress", "Completed"
    std::string requestTime;
    std::string assignedTo;
    
    ServiceRequest() : requestId(0), roomNumber(0), priority(3) {}
    
    ServiceRequest(int id, int room, std::string t, std::string desc, int pri, 
                   std::string stat, std::string time, std::string assigned)
        : requestId(id), roomNumber(room), type(t), description(desc), 
          priority(pri), status(stat), requestTime(time), assignedTo(assigned) {}
    
    crow::json::wvalue toJSON() const {
        crow::json::wvalue json;
        json["requestId"] = requestId;
        json["roomNumber"] = roomNumber;
        json["type"] = type;
        json["description"] = description;
        json["priority"] = priority;
        json["status"] = status;
        json["requestTime"] = requestTime;
        json["assignedTo"] = assignedTo;
        return json;
    }
    
    std::string toFileString() const {
        return std::to_string(requestId) + "|" + std::to_string(roomNumber) + "|" + type + "|" +
               description + "|" + std::to_string(priority) + "|" + status + "|" + 
               requestTime + "|" + assignedTo;
    }
    
    // For priority queue comparison (higher priority = lower number)
    bool operator<(const ServiceRequest& other) const {
        return priority > other.priority;
    }
};

// ==================== DSA: BINARY SEARCH TREE FOR ROOMS ====================

class RoomBSTNode {
public:
    Room room;
    RoomBSTNode* left;
    RoomBSTNode* right;
    
    RoomBSTNode(Room r) : room(r), left(nullptr), right(nullptr) {}
};

class RoomBST {
private:
    RoomBSTNode* root;
    
    RoomBSTNode* insertHelper(RoomBSTNode* node, Room room) {
        if (node == nullptr) {
            return new RoomBSTNode(room);
        }
        
        if (room.roomNumber < node->room.roomNumber) {
            node->left = insertHelper(node->left, room);
        } else if (room.roomNumber > node->room.roomNumber) {
            node->right = insertHelper(node->right, room);
        }
        return node;
    }
    
    RoomBSTNode* searchHelper(RoomBSTNode* node, int roomNumber) {
        if (node == nullptr || node->room.roomNumber == roomNumber) {
            return node;
        }
        
        if (roomNumber < node->room.roomNumber) {
            return searchHelper(node->left, roomNumber);
        }
        return searchHelper(node->right, roomNumber);
    }
    
    void inorderHelper(RoomBSTNode* node, std::vector<Room>& rooms) {
        if (node == nullptr) return;
        inorderHelper(node->left, rooms);
        rooms.push_back(node->room);
        inorderHelper(node->right, rooms);
    }
    
    void collectByTypeHelper(RoomBSTNode* node, const std::string& type, std::vector<Room>& rooms) {
        if (node == nullptr) return;
        collectByTypeHelper(node->left, type, rooms);
        if (node->room.type == type) {
            rooms.push_back(node->room);
        }
        collectByTypeHelper(node->right, type, rooms);
    }
    
    void collectByStatusHelper(RoomBSTNode* node, const std::string& status, std::vector<Room>& rooms) {
        if (node == nullptr) return;
        collectByStatusHelper(node->left, status, rooms);
        if (node->room.status == status) {
            rooms.push_back(node->room);
        }
        collectByStatusHelper(node->right, status, rooms);
    }
    
    RoomBSTNode* deleteHelper(RoomBSTNode* node, int roomNumber) {
        if (node == nullptr) return nullptr;
        
        if (roomNumber < node->room.roomNumber) {
            node->left = deleteHelper(node->left, roomNumber);
        } else if (roomNumber > node->room.roomNumber) {
            node->right = deleteHelper(node->right, roomNumber);
        } else {
            // Node found
            if (node->left == nullptr) {
                RoomBSTNode* temp = node->right;
                delete node;
                return temp;
            } else if (node->right == nullptr) {
                RoomBSTNode* temp = node->left;
                delete node;
                return temp;
            }
            
            // Two children: Get inorder successor
            RoomBSTNode* temp = findMin(node->right);
            node->room = temp->room;
            node->right = deleteHelper(node->right, temp->room.roomNumber);
        }
        return node;
    }
    
    RoomBSTNode* findMin(RoomBSTNode* node) {
        while (node->left != nullptr) {
            node = node->left;
        }
        return node;
    }
    
public:
    RoomBST() : root(nullptr) {}
    
    void insert(Room room) {
        root = insertHelper(root, room);
    }
    
    Room* search(int roomNumber) {
        RoomBSTNode* node = searchHelper(root, roomNumber);
        return (node != nullptr) ? &(node->room) : nullptr;
    }
    
    std::vector<Room> getAllRooms() {
        std::vector<Room> rooms;
        inorderHelper(root, rooms);
        return rooms;
    }
    
    std::vector<Room> getRoomsByType(const std::string& type) {
        std::vector<Room> rooms;
        collectByTypeHelper(root, type, rooms);
        return rooms;
    }
    
    std::vector<Room> getRoomsByStatus(const std::string& status) {
        std::vector<Room> rooms;
        collectByStatusHelper(root, status, rooms);
        return rooms;
    }
    
    bool updateRoomStatus(int roomNumber, const std::string& newStatus) {
        Room* room = search(roomNumber);
        if (room != nullptr) {
            room->status = newStatus;
            return true;
        }
        return false;
    }
    
    void deleteRoom(int roomNumber) {
        root = deleteHelper(root, roomNumber);
    }
};

// ==================== DSA: HASH TABLE FOR USERS ====================

template<typename T>
class HashNode {
public:
    std::string key;
    T value;
    HashNode* next;
    
    HashNode(std::string k, T v) : key(k), value(v), next(nullptr) {}
};

template<typename T>
class HashTable {
private:
    static const int TABLE_SIZE = 100;
    HashNode<T>* table[TABLE_SIZE];
    
    int hashFunction(const std::string& key) {
        int hash = 0;
        for (char c : key) {
            hash = (hash * 31 + c) % TABLE_SIZE;
        }
        return hash;
    }
    
public:
    HashTable() {
        for (int i = 0; i < TABLE_SIZE; i++) {
            table[i] = nullptr;
        }
    }
    
    void insert(const std::string& key, const T& value) {
        int index = hashFunction(key);
        HashNode<T>* newNode = new HashNode<T>(key, value);
        
        if (table[index] == nullptr) {
            table[index] = newNode;
        } else {
            HashNode<T>* current = table[index];
            while (current->next != nullptr) {
                if (current->key == key) {
                    current->value = value;
                    delete newNode;
                    return;
                }
                current = current->next;
            }
            if (current->key == key) {
                current->value = value;
                delete newNode;
            } else {
                current->next = newNode;
            }
        }
    }
    
    T* search(const std::string& key) {
        int index = hashFunction(key);
        HashNode<T>* current = table[index];
        
        while (current != nullptr) {
            if (current->key == key) {
                return &(current->value);
            }
            current = current->next;
        }
        return nullptr;
    }
    
    bool remove(const std::string& key) {
        int index = hashFunction(key);
        HashNode<T>* current = table[index];
        HashNode<T>* prev = nullptr;
        
        while (current != nullptr) {
            if (current->key == key) {
                if (prev == nullptr) {
                    table[index] = current->next;
                } else {
                    prev->next = current->next;
                }
                delete current;
                return true;
            }
            prev = current;
            current = current->next;
        }
        return false;
    }
    
    std::vector<T> getAllValues() {
        std::vector<T> values;
        for (int i = 0; i < TABLE_SIZE; i++) {
            HashNode<T>* current = table[i];
            while (current != nullptr) {
                values.push_back(current->value);
                current = current->next;
            }
        }
        return values;
    }
};

// ==================== DSA: LINKED LIST FOR BOOKINGS/ORDERS ====================

template<typename T>
class ListNode {
public:
    T data;
    ListNode* next;
    
    ListNode(T d) : data(d), next(nullptr) {}
};

template<typename T>
class LinkedList {
private:
    ListNode<T>* head;
    int size;
    
public:
    LinkedList() : head(nullptr), size(0) {}
    
    void append(const T& data) {
        ListNode<T>* newNode = new ListNode<T>(data);
        
        if (head == nullptr) {
            head = newNode;
        } else {
            ListNode<T>* current = head;
            while (current->next != nullptr) {
                current = current->next;
            }
            current->next = newNode;
        }
        size++;
    }
    
    std::vector<T> toVector() {
        std::vector<T> result;
        ListNode<T>* current = head;
        while (current != nullptr) {
            result.push_back(current->data);
            current = current->next;
        }
        return result;
    }
    
    int getSize() { return size; }
    
    bool remove(int id) {
        if (head == nullptr) return false;
        
        if (getIdFromData(head->data) == id) {
            ListNode<T>* temp = head;
            head = head->next;
            delete temp;
            size--;
            return true;
        }
        
        ListNode<T>* current = head;
        while (current->next != nullptr) {
            if (getIdFromData(current->next->data) == id) {
                ListNode<T>* temp = current->next;
                current->next = current->next->next;
                delete temp;
                size--;
                return true;
            }
            current = current->next;
        }
        return false;
    }
    
private:
    int getIdFromData(const Booking& b) { return b.bookingId; }
    int getIdFromData(const FoodOrder& f) { return f.orderId; }
};

#endif // HOTEL_SYSTEM_H