#define CROW_MAIN
#include "crow_all.h"
#include <fstream>
#include <sstream>
#include <string>

// ---------- Utility: read a file from /static folder ----------
std::string readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return "";
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main() {
    crow::SimpleApp app;

    // ------------------- Serve Static Files -------------------
    // Changed route to /assets/<path> to avoid conflict
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
        else if (path.find(".jpg") != std::string::npos || path.find(".jpeg") != std::string::npos)
            res.add_header("Content-Type", "image/jpeg");
        else if (path.find(".html") != std::string::npos)
            res.add_header("Content-Type", "text/html");

        return res;
    });

    // ------------------- HOMEPAGE -------------------
    CROW_ROUTE(app, "/")([]() {
        std::string html = readFile("static/index.html");
        if (html.empty())
            return crow::response(404, "index.html not found");
        crow::response res(html);
        res.add_header("Content-Type", "text/html");
        return res;
    });

    // ------------------- LOGIN PAGE (GET + POST) -------------------
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
                if (!body)
                    return crow::response(400, "Invalid JSON");

                std::string id = body["id"].s();
                std::string password = body["password"].s();
                std::string role = body["role"].s();

                // --- Hardcoded credentials ---
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

    // ===================== USER DASHBOARD ROUTES =====================
    CROW_ROUTE(app, "/dashboard")([]() {
        auto html = readFile("static/dashboard.html");
        if (html.empty())
            return crow::response(404, "dashboard.html not found");
        crow::response res(html);
        res.add_header("Content-Type", "text/html");
        return res;
    });

    // Book Room Page
    CROW_ROUTE(app, "/user_book_room")([]() {
        auto html = readFile("static/user_book_room.html");
        return html.empty() ? crow::response(404, "user_book_room.html not found") : crow::response(html);
    });

    // Order Food Page
    CROW_ROUTE(app, "/user_order_food")([]() {
        auto html = readFile("static/user_order_food.html");
        return html.empty() ? crow::response(404, "user_order_food.html not found") : crow::response(html);
    });

    // View Bill Page
    CROW_ROUTE(app, "/user_view_bill")([]() {
        auto html = readFile("static/user_view_bill.html");
        return html.empty() ? crow::response(404, "user_view_bill.html not found") : crow::response(html);
    });

    // Example POST handler
    CROW_ROUTE(app, "/submit_booking").methods(crow::HTTPMethod::Post)
    ([](const crow::request &req) {
        auto body = req.body;
        CROW_LOG_INFO << "Booking data received: " << body;
        return crow::response(200, "<h2>Booking received! We'll confirm shortly.</h2>");
    });

    // ===================== STAFF DASHBOARD ROUTES =====================
    CROW_ROUTE(app, "/staff_dashboard")([]() {
        auto html = readFile("static/staff_dashboard.html");
        return html.empty() ? crow::response(404, "staff_dashboard.html not found") : crow::response(html);
    });

    CROW_ROUTE(app, "/check_in")([](){
        auto html = readFile("static/staff_check_in.html");
        return html.empty() ? crow::response(404, "staff_check_in.html not found") : crow::response(html);
    });

    CROW_ROUTE(app, "/room_cleaning")([](){
        auto html = readFile("static/staff_room_cleaning.html");
        return html.empty() ? crow::response(404, "staff_room_cleaning.html not found") : crow::response(html);
    });

    CROW_ROUTE(app, "/service_requests")([](){
        auto html = readFile("static/staff_service_requests.html");
        return html.empty() ? crow::response(404, "staff_service_requests.html not found") : crow::response(html);
    });

    CROW_ROUTE(app, "/reports")([](){
        auto html = readFile("static/staff_reports.html");
        return html.empty() ? crow::response(404, "staff_reports.html not found") : crow::response(html);
    });

    // ===================== ADMIN DASHBOARD ROUTES =====================
    CROW_ROUTE(app, "/admin_dashboard")([]() {
        auto html = readFile("static/admin_dashboard.html");
        return html.empty() ? crow::response(404, "admin_dashboard.html not found") : crow::response(html);
    });

    CROW_ROUTE(app, "/admin_manage_rooms")([]() {
        auto html = readFile("static/admin_manage_rooms.html");
        return html.empty() ? crow::response(404, "admin_manage_rooms.html not found") : crow::response(html);
    });

    CROW_ROUTE(app, "/admin_manage_staff")([]() {
        auto html = readFile("static/admin_manage_staff.html");
        return html.empty() ? crow::response(404, "admin_manage_staff.html not found") : crow::response(html);
    });

    CROW_ROUTE(app, "/admin_view_reports")([]() {
        auto html = readFile("static/admin_view_reports.html");
        return html.empty() ? crow::response(404, "admin_view_reports.html not found") : crow::response(html);
    });

    CROW_ROUTE(app, "/admin_settings")([]() {
        auto html = readFile("static/admin_settings.html");
        return html.empty() ? crow::response(404, "admin_settings.html not found") : crow::response(html);
    });

    // ------------------- RUN SERVER -------------------
    app.port(18080).multithreaded().run();
}
