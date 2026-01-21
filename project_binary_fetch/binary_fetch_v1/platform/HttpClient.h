#pragma once

#include <string>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <poll.h>

namespace Platform {

class HttpClient {
public:
    struct Response {
        int status_code = 0;
        std::string body;
        bool success = false;
    };

    static Response get(const std::string& host, const std::string& path, int port = 80, int timeout_ms = 5000) {
        Response resp;
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) return resp;

        struct hostent* server = gethostbyname(host.c_str());
        if (!server) {
            close(sock);
            return resp;
        }

        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
        serv_addr.sin_port = htons(port);

        int flags = fcntl(sock, F_GETFL, 0);
        fcntl(sock, F_SETFL, flags | O_NONBLOCK);

        connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

        struct pollfd pfd;
        pfd.fd = sock;
        pfd.events = POLLOUT;
        if (poll(&pfd, 1, timeout_ms) <= 0) {
            close(sock);
            return resp;
        }

        fcntl(sock, F_SETFL, flags);

        std::string request = "GET " + path + " HTTP/1.1\r\n";
        request += "Host: " + host + "\r\n";
        request += "Connection: close\r\n";
        request += "User-Agent: BinaryFetch/1.0\r\n";
        request += "\r\n";

        if (send(sock, request.c_str(), request.length(), 0) < 0) {
            close(sock);
            return resp;
        }

        std::string response;
        char buffer[4096];
        
        pfd.events = POLLIN;
        while (poll(&pfd, 1, timeout_ms) > 0) {
            ssize_t n = recv(sock, buffer, sizeof(buffer) - 1, 0);
            if (n <= 0) break;
            buffer[n] = '\0';
            response += buffer;
        }

        close(sock);

        size_t header_end = response.find("\r\n\r\n");
        if (header_end != std::string::npos) {
            std::string headers = response.substr(0, header_end);
            resp.body = response.substr(header_end + 4);
            
            size_t status_pos = headers.find(' ');
            if (status_pos != std::string::npos) {
                resp.status_code = std::stoi(headers.substr(status_pos + 1, 3));
                resp.success = (resp.status_code >= 200 && resp.status_code < 300);
            }
        }

        return resp;
    }

    static Response post(const std::string& host, const std::string& path, const std::string& data, 
                         int port = 80, int timeout_ms = 5000) {
        Response resp;
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) return resp;

        struct hostent* server = gethostbyname(host.c_str());
        if (!server) {
            close(sock);
            return resp;
        }

        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
        serv_addr.sin_port = htons(port);

        int flags = fcntl(sock, F_GETFL, 0);
        fcntl(sock, F_SETFL, flags | O_NONBLOCK);

        connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

        struct pollfd pfd;
        pfd.fd = sock;
        pfd.events = POLLOUT;
        if (poll(&pfd, 1, timeout_ms) <= 0) {
            close(sock);
            return resp;
        }

        fcntl(sock, F_SETFL, flags);

        std::string request = "POST " + path + " HTTP/1.1\r\n";
        request += "Host: " + host + "\r\n";
        request += "Connection: close\r\n";
        request += "Content-Type: application/octet-stream\r\n";
        request += "Content-Length: " + std::to_string(data.length()) + "\r\n";
        request += "User-Agent: BinaryFetch/1.0\r\n";
        request += "\r\n";
        request += data;

        if (send(sock, request.c_str(), request.length(), 0) < 0) {
            close(sock);
            return resp;
        }

        std::string response;
        char buffer[4096];
        
        pfd.events = POLLIN;
        while (poll(&pfd, 1, timeout_ms) > 0) {
            ssize_t n = recv(sock, buffer, sizeof(buffer) - 1, 0);
            if (n <= 0) break;
            buffer[n] = '\0';
            response += buffer;
        }

        close(sock);

        size_t header_end = response.find("\r\n\r\n");
        if (header_end != std::string::npos) {
            std::string headers = response.substr(0, header_end);
            resp.body = response.substr(header_end + 4);
            
            size_t status_pos = headers.find(' ');
            if (status_pos != std::string::npos) {
                resp.status_code = std::stoi(headers.substr(status_pos + 1, 3));
                resp.success = (resp.status_code >= 200 && resp.status_code < 300);
            }
        }

        return resp;
    }

    static std::string downloadSpeed(const std::string& host, const std::string& path, 
                                      size_t bytes, int timeout_ms = 5000) {
        auto start = std::chrono::high_resolution_clock::now();
        Response resp = get(host, path + "?bytes=" + std::to_string(bytes), 80, timeout_ms);
        auto end = std::chrono::high_resolution_clock::now();

        if (!resp.success || resp.body.empty()) return "Unknown";

        double seconds = std::chrono::duration<double>(end - start).count();
        if (seconds < 0.001) seconds = 0.001;
        
        double megabits = (resp.body.length() * 8.0) / 1000000.0;
        double mbps = megabits / seconds;

        return formatSpeed(mbps);
    }

    static std::string uploadSpeed(const std::string& host, const std::string& path,
                                    size_t bytes, int timeout_ms = 5000) {
        std::string data(bytes, 'X');
        
        auto start = std::chrono::high_resolution_clock::now();
        Response resp = post(host, path, data, 80, timeout_ms);
        auto end = std::chrono::high_resolution_clock::now();

        if (!resp.success) return "Unknown";

        double seconds = std::chrono::duration<double>(end - start).count();
        if (seconds < 0.001) seconds = 0.001;
        
        double megabits = (bytes * 8.0) / 1000000.0;
        double mbps = megabits / seconds;

        return formatSpeed(mbps);
    }

private:
    static std::string formatSpeed(double mbps) {
        char buf[32];
        if (mbps >= 1000.0) {
            snprintf(buf, sizeof(buf), "%.1f Gbps", mbps / 1000.0);
        } else if (mbps >= 1.0) {
            snprintf(buf, sizeof(buf), "%.1f Mbps", mbps);
        } else {
            snprintf(buf, sizeof(buf), "%.0f Kbps", mbps * 1000.0);
        }
        return std::string(buf);
    }
};

}
