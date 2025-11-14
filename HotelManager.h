#ifndef HOTEL_MANAGER_H
#define HOTEL_MANAGER_H

#include "hotel_system.h"
#include <algorithm>

class HotelManager {
private:
    RoomBST roomTree;
    HashTable<User> userTable;
    LinkedList<Booking> bookingList;
    LinkedList<FoodOrder> foodOrderList;
    std::priority_queue<ServiceRequest> serviceRequestQueue;
    std::queue<Booking> waitingQueue;  // For when rooms are full
    
    // File paths
    const std::string ROOMS_FILE = "data/rooms.dat";
    const std::string USERS_FILE = "data/users.dat";
    const std::string BOOKINGS_FILE = "data/bookings.dat";
    const std::string ORDERS_FILE = "data/orders.dat";
    const std::string REQUESTS_FILE = "data/requests.dat";
    
public:
    HotelManager() {
        loadAllData();
        initializeDefaultData();
    }
    
    ~HotelManager() {
        saveAllData();
    }
    
    // ==================== INITIALIZATION ====================
    
    void initializeDefaultData() {
        // Check if users exist, if not create defaults
        if (userTable.search("user") == nullptr) {
            userTable.insert("user", User("user", "1234", "Guest User", "user@hotel.com", "1234567890", "user"));
            userTable.insert("admin", User("admin", "admin123", "Admin", "admin@hotel.com", "9876543210", "admin"));
            userTable.insert("staff", User("staff", "staff123", "Staff Member", "staff@hotel.com", "5555555555", "staff"));
        }
        
        // Initialize rooms if empty
        auto rooms = roomTree.getAllRooms();
        if (rooms.empty()) {
            // Single rooms (101-110)
            for (int i = 101; i <= 110; i++) {
                roomTree.insert(Room(i, "Single", 1500.0, "Available", 1, "AC, TV, WiFi"));
            }
            // Double rooms (201-210)
            for (int i = 201; i <= 210; i++) {
                roomTree.insert(Room(i, "Double", 2500.0, "Available", 2, "AC, TV, WiFi, Mini-bar"));
            }
            // Suite rooms (301-305)
            for (int i = 301; i <= 305; i++) {
                roomTree.insert(Room(i, "Suite", 5000.0, "Available", 3, "AC, TV, WiFi, Mini-bar, Jacuzzi"));
            }
            // Deluxe rooms (401-403)
            for (int i = 401; i <= 403; i++) {
                roomTree.insert(Room(i, "Deluxe", 8000.0, "Available", 4, "AC, TV, WiFi, Mini-bar, Jacuzzi, Ocean View"));
            }
        }
    }
    
    // ==================== USER AUTHENTICATION ====================
    
    crow::json::wvalue login(const std::string& userId, const std::string& password) {
        User* user = userTable.search(userId);
        crow::json::wvalue response;
        
        if (user != nullptr && user->password == password) {
            response["success"] = true;
            response["role"] = user->role;
            response["userId"] = user->userId;
            response["name"] = user->name;
        } else {
            response["success"] = false;
            response["message"] = "Invalid credentials";
        }
        
        return response;
    }
    
    bool registerUser(const User& user) {
        if (userTable.search(user.userId) != nullptr) {
            return false;  // User already exists
        }
        userTable.insert(user.userId, user);
        return true;
    }
    
    // ==================== ROOM MANAGEMENT ====================
    
    std::vector<crow::json::wvalue> getAllRooms() {
        auto rooms = roomTree.getAllRooms();
        std::vector<crow::json::wvalue> jsonRooms;
        for (const auto& room : rooms) {
            jsonRooms.push_back(room.toJSON());
        }
        return jsonRooms;
    }
    
    std::vector<crow::json::wvalue> getAvailableRooms() {
        auto rooms = roomTree.getRoomsByStatus("Available");
        std::vector<crow::json::wvalue> jsonRooms;
        for (const auto& room : rooms) {
            jsonRooms.push_back(room.toJSON());
        }
        return jsonRooms;
    }
    
    std::vector<crow::json::wvalue> getRoomsByType(const std::string& type) {
        auto rooms = roomTree.getRoomsByType(type);
        std::vector<crow::json::wvalue> jsonRooms;
        for (const auto& room : rooms) {
            if (room.status == "Available") {
                jsonRooms.push_back(room.toJSON());
            }
        }
        return jsonRooms;
    }
    
    crow::json::wvalue getRoomDetails(int roomNumber) {
        Room* room = roomTree.search(roomNumber);
        if (room != nullptr) {
            return room->toJSON();
        }
        crow::json::wvalue error;
        error["error"] = "Room not found";
        return error;
    }
    
    bool addRoom(const Room& room) {
        if (roomTree.search(room.roomNumber) != nullptr) {
            return false;  // Room already exists
        }
        roomTree.insert(room);
        return true;
    }
    
    bool updateRoom(int roomNumber, const Room& updatedRoom) {
        Room* room = roomTree.search(roomNumber);
        if (room != nullptr) {
            *room = updatedRoom;
            return true;
        }
        return false;
    }
    
    bool deleteRoom(int roomNumber) {
        Room* room = roomTree.search(roomNumber);
        if (room != nullptr && room->status == "Available") {
            roomTree.deleteRoom(roomNumber);
            return true;
        }
        return false;
    }
    
    bool updateRoomStatus(int roomNumber, const std::string& status) {
        return roomTree.updateRoomStatus(roomNumber, status);
    }
    
    // ==================== BOOKING MANAGEMENT ====================
    
    crow::json::wvalue createBooking(const std::string& userId, int roomNumber, 
                                      const std::string& checkIn, const std::string& checkOut, 
                                      int nights) {
        crow::json::wvalue response;
        
        Room* room = roomTree.search(roomNumber);
        if (room == nullptr) {
            response["success"] = false;
            response["message"] = "Room not found";
            return response;
        }
        
        if (room->status != "Available") {
            response["success"] = false;
            response["message"] = "Room not available";
            
            // Add to waiting queue
            Booking waitingBooking(generateID(), userId, roomNumber, checkIn, checkOut, 
                                  nights, room->pricePerNight * nights, "Waiting", 
                                  getCurrentDateTime());
            waitingQueue.push(waitingBooking);
            response["waitingPosition"] = (int)waitingQueue.size();
            return response;
        }
        
        // Create booking
        int bookingId = generateID();
        double totalAmount = room->pricePerNight * nights;
        Booking booking(bookingId, userId, roomNumber, checkIn, checkOut, nights, 
                       totalAmount, "Confirmed", getCurrentDateTime());
        
        bookingList.append(booking);
        roomTree.updateRoomStatus(roomNumber, "Reserved");
        
        response["success"] = true;
        response["bookingId"] = bookingId;
        response["totalAmount"] = totalAmount;
        response["message"] = "Booking confirmed successfully";
        
        return response;
    }
    
    std::vector<crow::json::wvalue> getUserBookings(const std::string& userId) {
        auto bookings = bookingList.toVector();
        std::vector<crow::json::wvalue> userBookings;
        
        for (const auto& booking : bookings) {
            if (booking.userId == userId) {
                userBookings.push_back(booking.toJSON());
            }
        }
        
        return userBookings;
    }
    
    std::vector<crow::json::wvalue> getAllBookings() {
        auto bookings = bookingList.toVector();
        std::vector<crow::json::wvalue> jsonBookings;
        
        for (const auto& booking : bookings) {
            jsonBookings.push_back(booking.toJSON());
        }
        
        return jsonBookings;
    }
    
    bool checkIn(int bookingId) {
        auto bookings = bookingList.toVector();
        for (auto& booking : bookings) {
            if (booking.bookingId == bookingId && booking.status == "Confirmed") {
                booking.status = "CheckedIn";
                roomTree.updateRoomStatus(booking.roomNumber, "Occupied");
                return true;
            }
        }
        return false;
    }
    
    crow::json::wvalue checkOut(int bookingId) {
        crow::json::wvalue response;
        auto bookings = bookingList.toVector();
        
        for (auto& booking : bookings) {
            if (booking.bookingId == bookingId && booking.status == "CheckedIn") {
                booking.status = "CheckedOut";
                roomTree.updateRoomStatus(booking.roomNumber, "Available");
                
                response["success"] = true;
                response["totalBill"] = booking.totalAmount;
                response["message"] = "Check-out successful";
                
                // Process waiting queue
                if (!waitingQueue.empty()) {
                    Booking waitingBooking = waitingQueue.front();
                    waitingQueue.pop();
                    // Notify or auto-confirm waiting booking
                }
                
                return response;
            }
        }
        
        response["success"] = false;
        response["message"] = "Booking not found or already checked out";
        return response;
    }
    
    bool cancelBooking(int bookingId) {
        auto bookings = bookingList.toVector();
        for (auto& booking : bookings) {
            if (booking.bookingId == bookingId) {
                if (booking.status == "Confirmed" || booking.status == "Pending") {
                    booking.status = "Cancelled";
                    roomTree.updateRoomStatus(booking.roomNumber, "Available");
                    return true;
                }
            }
        }
        return false;
    }
    
    // ==================== FOOD ORDER MANAGEMENT ====================
    
    crow::json::wvalue createFoodOrder(const std::string& userId, int roomNumber, 
                                        const std::vector<std::pair<std::string, int>>& items,
                                        double totalPrice) {
        crow::json::wvalue response;
        
        FoodOrder order;
        order.orderId = generateID();
        order.userId = userId;
        order.roomNumber = roomNumber;
        order.items = items;
        order.totalPrice = totalPrice;
        order.status = "Pending";
        order.orderTime = getCurrentDateTime();
        
        foodOrderList.append(order);
        
        response["success"] = true;
        response["orderId"] = order.orderId;
        response["message"] = "Order placed successfully";
        
        return response;
    }
    
    std::vector<crow::json::wvalue> getUserOrders(const std::string& userId) {
        auto orders = foodOrderList.toVector();
        std::vector<crow::json::wvalue> userOrders;
        
        for (const auto& order : orders) {
            if (order.userId == userId) {
                userOrders.push_back(order.toJSON());
            }
        }
        
        return userOrders;
    }
    
    std::vector<crow::json::wvalue> getAllOrders() {
        auto orders = foodOrderList.toVector();
        std::vector<crow::json::wvalue> jsonOrders;
        
        for (const auto& order : orders) {
            jsonOrders.push_back(order.toJSON());
        }
        
        return jsonOrders;
    }
    
    // ==================== SERVICE REQUEST MANAGEMENT ====================
    
    crow::json::wvalue createServiceRequest(int roomNumber, const std::string& type,
                                             const std::string& description, int priority) {
        crow::json::wvalue response;
        
        ServiceRequest request(generateID(), roomNumber, type, description, priority,
                              "Pending", getCurrentDateTime(), "Unassigned");
        
        serviceRequestQueue.push(request);
        
        response["success"] = true;
        response["requestId"] = request.requestId;
        response["message"] = "Service request created";
        
        return response;
    }
    
    std::vector<crow::json::wvalue> getPendingServiceRequests() {
        std::vector<crow::json::wvalue> requests;
        std::priority_queue<ServiceRequest> tempQueue = serviceRequestQueue;
        
        while (!tempQueue.empty()) {
            ServiceRequest req = tempQueue.top();
            tempQueue.pop();
            if (req.status == "Pending") {
                requests.push_back(req.toJSON());
            }
        }
        
        return requests;
    }
    
    ServiceRequest* getNextServiceRequest() {
        if (!serviceRequestQueue.empty()) {
            static ServiceRequest req = serviceRequestQueue.top();
            return &req;
        }
        return nullptr;
    }
    
    // ==================== BILLING ====================
    
    crow::json::wvalue getUserBill(const std::string& userId) {
        crow::json::wvalue bill;
        double roomCharges = 0.0;
        double foodCharges = 0.0;
        
        // Calculate room charges
        auto bookings = bookingList.toVector();
        for (const auto& booking : bookings) {
            if (booking.userId == userId && booking.status == "CheckedIn") {
                roomCharges += booking.totalAmount;
            }
        }
        
        // Calculate food charges
        auto orders = foodOrderList.toVector();
        for (const auto& order : orders) {
            if (order.userId == userId) {
                foodCharges += order.totalPrice;
            }
        }
        
        double total = roomCharges + foodCharges;
        double tax = total * 0.18;  // 18% tax
        double grandTotal = total + tax;
        
        bill["roomCharges"] = roomCharges;
        bill["foodCharges"] = foodCharges;
        bill["subtotal"] = total;
        bill["tax"] = tax;
        bill["grandTotal"] = grandTotal;
        
        return bill;
    }
    
    // ==================== REPORTS & ANALYTICS ====================
    
    crow::json::wvalue getDashboardStats() {
        crow::json::wvalue stats;
        
        auto allRooms = roomTree.getAllRooms();
        int totalRooms = allRooms.size();
        int occupiedRooms = 0;
        int availableRooms = 0;
        
        for (const auto& room : allRooms) {
            if (room.status == "Occupied") occupiedRooms++;
            if (room.status == "Available") availableRooms++;
        }
        
        auto bookings = bookingList.toVector();
        int totalBookings = bookings.size();
        int activeBookings = 0;
        double totalRevenue = 0.0;
        
        for (const auto& booking : bookings) {
            if (booking.status == "CheckedIn") activeBookings++;
            if (booking.status == "CheckedOut") totalRevenue += booking.totalAmount;
        }
        
        stats["totalRooms"] = totalRooms;
        stats["occupiedRooms"] = occupiedRooms;
        stats["availableRooms"] = availableRooms;
        stats["occupancyRate"] = totalRooms > 0 ? (occupiedRooms * 100.0 / totalRooms) : 0;
        stats["totalBookings"] = totalBookings;
        stats["activeBookings"] = activeBookings;
        stats["totalRevenue"] = totalRevenue;
        stats["pendingServiceRequests"] = (int)serviceRequestQueue.size();
        
        return stats;
    }
    
    // ==================== FILE I/O ====================
    
    void loadAllData() {
        loadRooms();
        loadUsers();
        loadBookings();
        // Add more as needed
    }
    
    void saveAllData() {
        saveRooms();
        saveUsers();
        saveBookings();
        // Add more as needed
    }
    
    void loadRooms() {
        std::ifstream file(ROOMS_FILE);
        if (!file.is_open()) return;
        
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) {
                Room room = Room::fromFileString(line);
                roomTree.insert(room);
            }
        }
        file.close();
    }
    
    void saveRooms() {
        std::ofstream file(ROOMS_FILE);
        if (!file.is_open()) return;
        
        auto rooms = roomTree.getAllRooms();
        for (const auto& room : rooms) {
            file << room.toFileString() << "\n";
        }
        file.close();
    }
    
    void loadUsers() {
        std::ifstream file(USERS_FILE);
        if (!file.is_open()) return;
        
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) {
                User user = User::fromFileString(line);
                userTable.insert(user.userId, user);
            }
        }
        file.close();
    }
    
    void saveUsers() {
        std::ofstream file(USERS_FILE);
        if (!file.is_open()) return;
        
        auto users = userTable.getAllValues();
        for (const auto& user : users) {
            file << user.toFileString() << "\n";
        }
        file.close();
    }
    
    void loadBookings() {
        std::ifstream file(BOOKINGS_FILE);
        if (!file.is_open()) return;
        
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) {
                Booking booking = Booking::fromFileString(line);
                bookingList.append(booking);
            }
        }
        file.close();
    }
    
    void saveBookings() {
        std::ofstream file(BOOKINGS_FILE);
        if (!file.is_open()) return;
        
        auto bookings = bookingList.toVector();
        for (const auto& booking : bookings) {
            file << booking.toFileString() << "\n";
        }
        file.close();
    }
};

#endif // HOTEL_MANAGER_H