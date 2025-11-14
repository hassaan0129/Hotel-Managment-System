#define CROW_MAIN
#include "crow_all.h"
#include "HotelManager.h"
#include <fstream>
#include <sstream>
#include <string>

// Create data directory if it doesn't exist
void createDataDirectory() {
    // Use C++ standard library approach
    #ifdef _WIN32
        system("if not exist data mkdir data");
    #else
        system("mkdir -p data");
    #endif
}

// ---------- Utility: read a file from /static folder ----------
std::string readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return "";
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main() {
    createDataDirectory();
    
    crow::SimpleApp app;
    HotelManager hotelManager;  // Initialize the hotel management system

    // ==================== STATIC FILES ====================
    CROW_ROUTE(app, "/assets/<path>")
    ([](const crow::request&, std::string path) {
        std::string filePath = "static/" + path;
        auto content = readFile(filePath);
        if (content.empty()) return crow::response(404);

        crow::response res(content);
        if (path.find(".css") != std::string::npos)
            res.add_header("Content-Type", "text/css");
        else if (path.find(".js") != std::string::npos)
            res.add_header("Content-Type", "application/javascript");
        else if (path.find(".png") != std::string::npos)
            res.add_header("Content-Type", "image/png");
        else if (path.find(".jpg") != std::string::npos)
            res.add_header("Content-Type", "image/jpeg");
        else if (path.find(".html") != std::string::npos)
            res.add_header("Content-Type", "text/html");
        return res;
    });

    // ==================== HOMEPAGE ====================
    CROW_ROUTE(app, "/")([]() {
        std::string html = readFile("static/index.html");
        if (html.empty()) return crow::response(404, "index.html not found");
        crow::response res(html);
        res.add_header("Content-Type", "text/html");
        return res;
    });

    // ==================== LOGIN ====================
    CROW_ROUTE(app, "/login").methods(crow::HTTPMethod::Get, crow::HTTPMethod::Post)
    ([](const crow::request& req) {
        if (req.method == crow::HTTPMethod::Get) {
            auto html = readFile("static/login.html");
            if (html.empty()) return crow::response(404, "login.html not found");
            crow::response res(html);
            res.add_header("Content-Type", "text/html");
            return res;
        }

        if (req.method == crow::HTTPMethod::Post) {
            try {
                auto body = crow::json::load(req.body);
                if (!body) return crow::response(400, "Invalid JSON");

                std::string id = body["id"].s();
                std::string password = body["password"].s();
                std::string role = body["role"].s();

                // --- Hardcoded credentials (works with backend DSA) ---
                if (role == "user" && id == "user" && password == "1234") {
                    crow::response res;
                    res.code = 302;
                    res.add_header("Location", "/dashboard");
                    return res;
                } else if (role == "admin" && id == "admin" && password == "admin123") {
                    crow::response res;
                    res.code = 302;
                    res.add_header("Location", "/admin_dashboard");
                    return res;
                } else if (role == "staff" && id == "staff" && password == "staff123") {
                    crow::response res;
                    res.code = 302;
                    res.add_header("Location", "/staff_dashboard");
                    return res;
                } else {
                    return crow::response(401, "Invalid credentials!");
                }
            } catch (...) {
                return crow::response(400, "Error processing JSON");
            }
        }
        return crow::response(405, "Method not allowed");
    });

    // ==================== USER DASHBOARD ====================
    CROW_ROUTE(app, "/dashboard")([]() {
        auto html = readFile("static/dashboard.html");
        if (html.empty()) return crow::response(404, "dashboard.html not found");
        crow::response res(html);
        res.add_header("Content-Type", "text/html");
        return res;
    });

    CROW_ROUTE(app, "/user_book_room")([]() {
        auto html = readFile("static/user_book_room.html");
        return html.empty() ? crow::response(404) : crow::response(html);
    });

    CROW_ROUTE(app, "/user_order_food")([]() {
        auto html = readFile("static/user_order_food.html");
        return html.empty() ? crow::response(404) : crow::response(html);
    });

    CROW_ROUTE(app, "/user_view_bill")([]() {
        auto html = readFile("static/user_view_bill.html");
        return html.empty() ? crow::response(404) : crow::response(html);
    });

    // ==================== ROOM API ENDPOINTS ====================
    
    // Get all available rooms
    CROW_ROUTE(app, "/api/rooms/available")
    ([&hotelManager]() {
        auto rooms = hotelManager.getAvailableRooms();
        crow::json::wvalue response;
        response["rooms"] = std::move(rooms);
        return crow::response(response);
    });

    // Get rooms by type
    CROW_ROUTE(app, "/api/rooms/type/<string>")
    ([&hotelManager](std::string type) {
        auto rooms = hotelManager.getRoomsByType(type);
        crow::json::wvalue response;
        response["rooms"] = std::move(rooms);
        return crow::response(response);
    });

    // Get specific room details
    CROW_ROUTE(app, "/api/rooms/<int>")
    ([&hotelManager](int roomNumber) {
        auto room = hotelManager.getRoomDetails(roomNumber);
        return crow::response(room);
    });

    // Get all rooms (admin only)
    CROW_ROUTE(app, "/api/admin/rooms")
    ([&hotelManager]() {
        auto rooms = hotelManager.getAllRooms();
        crow::json::wvalue response;
        response["rooms"] = std::move(rooms);
        return crow::response(response);
    });

    // Add new room (admin only)
    CROW_ROUTE(app, "/api/admin/rooms/add").methods(crow::HTTPMethod::Post)
    ([&hotelManager](const crow::request& req) {
        try {
            auto body = crow::json::load(req.body);
            if (!body) return crow::response(400, "Invalid JSON");

            Room room(
                body["roomNumber"].i(),
                body["type"].s(),
                body["pricePerNight"].d(),
                body["status"].s(),
                body["floor"].i(),
                body["features"].s()
            );

            bool success = hotelManager.addRoom(room);
            crow::json::wvalue response;
            response["success"] = success;
            response["message"] = success ? "Room added successfully" : "Room already exists";
            return crow::response(response);
        } catch (...) {
            return crow::response(400, "Error processing request");
        }
    });

    // Update room (admin only)
    CROW_ROUTE(app, "/api/admin/rooms/update/<int>").methods(crow::HTTPMethod::Put)
    ([&hotelManager](const crow::request& req, int roomNumber) {
        try {
            auto body = crow::json::load(req.body);
            if (!body) return crow::response(400, "Invalid JSON");

            Room room(
                roomNumber,
                body["type"].s(),
                body["pricePerNight"].d(),
                body["status"].s(),
                body["floor"].i(),
                body["features"].s()
            );

            bool success = hotelManager.updateRoom(roomNumber, room);
            crow::json::wvalue response;
            response["success"] = success;
            response["message"] = success ? "Room updated successfully" : "Room not found";
            return crow::response(response);
        } catch (...) {
            return crow::response(400, "Error processing request");
        }
    });

    // Delete room (admin only)
    CROW_ROUTE(app, "/api/admin/rooms/delete/<int>").methods(crow::HTTPMethod::Delete)
    ([&hotelManager](int roomNumber) {
        bool success = hotelManager.deleteRoom(roomNumber);
        crow::json::wvalue response;
        response["success"] = success;
        response["message"] = success ? "Room deleted successfully" : "Cannot delete occupied room";
        return crow::response(response);
    });

    // ==================== BOOKING API ENDPOINTS ====================
    
    // Create new booking
    CROW_ROUTE(app, "/api/bookings/create").methods(crow::HTTPMethod::Post)
    ([&hotelManager](const crow::request& req) {
        try {
            auto body = crow::json::load(req.body);
            if (!body) return crow::response(400, "Invalid JSON");

            std::string userId = body["userId"].s();
            int roomNumber = body["roomNumber"].i();
            std::string checkIn = body["checkInDate"].s();
            std::string checkOut = body["checkOutDate"].s();
            int nights = body["nights"].i();

            auto result = hotelManager.createBooking(userId, roomNumber, checkIn, checkOut, nights);
            return crow::response(result);
        } catch (...) {
            return crow::response(400, "Error processing booking");
        }
    });

    // Get user bookings
    CROW_ROUTE(app, "/api/bookings/user/<string>")
    ([&hotelManager](std::string userId) {
        auto bookings = hotelManager.getUserBookings(userId);
        crow::json::wvalue response;
        response["bookings"] = std::move(bookings);
        return crow::response(response);
    });

    // Get all bookings (admin/staff)
    CROW_ROUTE(app, "/api/bookings/all")
    ([&hotelManager]() {
        auto bookings = hotelManager.getAllBookings();
        crow::json::wvalue response;
        response["bookings"] = std::move(bookings);
        return crow::response(response);
    });

    // Check-in
    CROW_ROUTE(app, "/api/bookings/checkin/<int>").methods(crow::HTTPMethod::Post)
    ([&hotelManager](int bookingId) {
        bool success = hotelManager.checkIn(bookingId);
        crow::json::wvalue response;
        response["success"] = success;
        response["message"] = success ? "Check-in successful" : "Booking not found";
        return crow::response(response);
    });

    // Check-out
    CROW_ROUTE(app, "/api/bookings/checkout/<int>").methods(crow::HTTPMethod::Post)
    ([&hotelManager](int bookingId) {
        auto result = hotelManager.checkOut(bookingId);
        return crow::response(result);
    });

    // Cancel booking
    CROW_ROUTE(app, "/api/bookings/cancel/<int>").methods(crow::HTTPMethod::Post)
    ([&hotelManager](int bookingId) {
        bool success = hotelManager.cancelBooking(bookingId);
        crow::json::wvalue response;
        response["success"] = success;
        response["message"] = success ? "Booking cancelled" : "Cannot cancel booking";
        return crow::response(response);
    });

    // ==================== FOOD ORDER API ENDPOINTS ====================
    
    // Create food order
    CROW_ROUTE(app, "/api/orders/create").methods(crow::HTTPMethod::Post)
    ([&hotelManager](const crow::request& req) {
        try {
            auto body = crow::json::load(req.body);
            if (!body) return crow::response(400, "Invalid JSON");

            std::string userId = body["userId"].s();
            int roomNumber = body["roomNumber"].i();
            double totalPrice = body["totalPrice"].d();
            
            std::vector<std::pair<std::string, int>> items;
            auto itemsJson = body["items"];
            for (size_t i = 0; i < itemsJson.size(); i++) {
                std::string name = itemsJson[i]["name"].s();
                int quantity = itemsJson[i]["quantity"].i();
                items.push_back({name, quantity});
            }

            auto result = hotelManager.createFoodOrder(userId, roomNumber, items, totalPrice);
            return crow::response(result);
        } catch (...) {
            return crow::response(400, "Error processing order");
        }
    });

    // Get user orders
    CROW_ROUTE(app, "/api/orders/user/<string>")
    ([&hotelManager](std::string userId) {
        auto orders = hotelManager.getUserOrders(userId);
        crow::json::wvalue response;
        response["orders"] = std::move(orders);
        return crow::response(response);
    });

    // Get all orders (staff)
    CROW_ROUTE(app, "/api/orders/all")
    ([&hotelManager]() {
        auto orders = hotelManager.getAllOrders();
        crow::json::wvalue response;
        response["orders"] = std::move(orders);
        return crow::response(response);
    });

    // ==================== SERVICE REQUEST API ENDPOINTS ====================
    
    // Create service request
    CROW_ROUTE(app, "/api/service/create").methods(crow::HTTPMethod::Post)
    ([&hotelManager](const crow::request& req) {
        try {
            auto body = crow::json::load(req.body);
            if (!body) return crow::response(400, "Invalid JSON");

            int roomNumber = body["roomNumber"].i();
            std::string type = body["type"].s();
            std::string description = body["description"].s();
            int priority = body["priority"].i();

            auto result = hotelManager.createServiceRequest(roomNumber, type, description, priority);
            return crow::response(result);
        } catch (...) {
            return crow::response(400, "Error processing request");
        }
    });

    // Get pending service requests
    CROW_ROUTE(app, "/api/service/pending")
    ([&hotelManager]() {
        auto requests = hotelManager.getPendingServiceRequests();
        crow::json::wvalue response;
        response["requests"] = std::move(requests);
        return crow::response(response);
    });

    // ==================== BILLING API ENDPOINTS ====================
    
    // Get user bill
    CROW_ROUTE(app, "/api/bill/<string>")
    ([&hotelManager](std::string userId) {
        auto bill = hotelManager.getUserBill(userId);
        return crow::response(bill);
    });

    // ==================== DASHBOARD & REPORTS ====================
    
    // Get dashboard statistics
    CROW_ROUTE(app, "/api/dashboard/stats")
    ([&hotelManager]() {
        auto stats = hotelManager.getDashboardStats();
        return crow::response(stats);
    });

    // ==================== STAFF ROUTES ====================
    CROW_ROUTE(app, "/staff_dashboard")([]() {
        auto html = readFile("static/staff_dashboard.html");
        return html.empty() ? crow::response(404) : crow::response(html);
    });

    CROW_ROUTE(app, "/check_in")([](){
        auto html = readFile("static/staff_check_in.html");
        return html.empty() ? crow::response(404) : crow::response(html);
    });

    CROW_ROUTE(app, "/room_cleaning")([](){
        auto html = readFile("static/staff_room_cleaning.html");
        return html.empty() ? crow::response(404) : crow::response(html);
    });

    CROW_ROUTE(app, "/service_requests")([](){
        auto html = readFile("static/staff_service_requests.html");
        return html.empty() ? crow::response(404) : crow::response(html);
    });

    CROW_ROUTE(app, "/reports")([](){
        auto html = readFile("static/staff_reports.html");
        return html.empty() ? crow::response(404) : crow::response(html);
    });

    // ==================== ADMIN ROUTES ====================
    CROW_ROUTE(app, "/admin_dashboard")([]() {
        auto html = readFile("static/admin_dashboard.html");
        return html.empty() ? crow::response(404) : crow::response(html);
    });

    CROW_ROUTE(app, "/admin_manage_rooms")([]() {
        auto html = readFile("static/admin_manage_rooms.html");
        return html.empty() ? crow::response(404) : crow::response(html);
    });

    CROW_ROUTE(app, "/admin_manage_staff")([]() {
        auto html = readFile("static/admin_manage_staff.html");
        return html.empty() ? crow::response(404) : crow::response(html);
    });

    CROW_ROUTE(app, "/admin_view_reports")([]() {
        auto html = readFile("static/admin_view_reports.html");
        return html.empty() ? crow::response(404) : crow::response(html);
    });

    CROW_ROUTE(app, "/admin_settings")([]() {
        auto html = readFile("static/admin_settings.html");
        return html.empty() ? crow::response(404) : crow::response(html);
    });

    // ==================== RUN SERVER ====================
    std::cout << "==================================" << std::endl;
    std::cout << "Hotel Management System Starting" << std::endl;
    std::cout << "Server running on port 18080" << std::endl;
    std::cout << "==================================" << std::endl;
    
    app.port(18080).multithreaded().run();
}