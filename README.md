# osprey
Large fish eating bird 

# C++ Telemetry & Blog Server 🌐

A high-performance, lightweight web server built entirely in C++ that hosts a blog about blogging pages while silently performing advanced device fingerprinting, IP enrichment, and real-time GPS geolocation tracking. The server automatically routes itself to the public internet using Cloudflare Quick Tunnels.

## 🚀 About the Project

This project bridges the gap between low-level backend engineering and advanced front-end telemetry. Instead of using traditional, heavy web servers like Apache or Nginx, this project relies on a custom C++ backend to handle raw HTTP requests, parse headers, and manage connections. 

When a visitor accesses the live link, the front-end JavaScript invisibly gathers highly specific hardware, screen, font, WebGL, and network data, while prompting the browser for HTML5 GPS coordinates to pin-point the visitor's location in the server terminal in real-time.

### ✨ Key Features
* **Zero-Config Global Hosting:** Automatically spawns a `cloudflared` process to generate a secure, globally accessible HTTPS link (`.trycloudflare.com`) on startup.
* **Hardware & Browser Fingerprinting:** Captures screen resolution, OS, browser type, WebGL renderer data, hardware concurrency, and installed fonts.
* **Real-Time IP Enrichment:** Automatically queries `ip-api.com` to resolve the visitor's public IP into an ISP, ASN, City, Country, and detects potential proxy or hosting usage.
* **GPS Geolocation:** Prompts the user for HTML5 Geolocation on page load and prints a clickable Google Maps link directly in the C++ server terminal.

---

## ⚙️ System Requirements

Before you begin, ensure your Linux system has the following installed:
* **C++ Compiler:** A compiler supporting C++11 or higher (`g++`).
* **Curl or Wget:** Used to fetch header files.
* **Cloudflare Daemon (`cloudflared`):** Required for generating public secure tunnels.

---

## 🛠️ Setup and Installation Guide

### 1. Clone the Repository
Clone the project repository to your local machine and navigate into the folder:
```bash
git clone https://github.com/KDN-23101/osprey.git
cd osprey
```
### 2. Download the required cpp-httplib header
```bash
wget https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h
```
### 3. Download the required nlohmann/json header
```bash
mkdir nlohmann
wget https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp -O nlohmann/json.hpp
```
### 4. Install Cloudflare Quick Tunnels (cloudflared)
```bash
curl -L --output cloudflared.deb https://github.com/cloudflare/cloudflared/releases/latest/download/cloudflared-linux-amd64.deb
sudo dpkg -i cloudflared.deb
```
### 5. Compile the C++ server
```bash
g++ -O3 osprey.cpp -o osprey -lpthread
```

### 6. Start the server
```bash
./osprey
```
