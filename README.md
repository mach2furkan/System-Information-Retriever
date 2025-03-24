# System-Information-Retriever
# 🎨🚀 macOS System Information Retriever

![MacOS](https://img.shields.io/badge/macOS-Compatible-brightgreen?style=for-the-badge) ![C++](https://img.shields.io/badge/C++-11%2B-blue?style=for-the-badge) ![License](https://img.shields.io/badge/License-MIT-red?style=for-the-badge) ![Status](https://img.shields.io/badge/Status-Active-purple?style=for-the-badge)

## 🌟 Overview
🎯 **C++ System Information Retriever** for **macOS** 🖥️🔍

This project collects and displays vital system information such as:
💾 **CPU Details** | ⚡ **Total RAM** | 🏷 **OS Version** | 💽 **Disk Drives** | 🔗 **MAC Address** | 🖥 **Monitor Resolutions**

🔧 **Built using macOS-specific APIs** for high accuracy & performance.

### 📌 Project Ownership
🛠 **Developed by:** **Thanos (Furkan Aşkın)** 🏆
🔬 **Educational Purposes Only** 🧠📚

---

## 🎯 Features
✅ **Retrieve CPU Specs** 🏎️
✅ **Check Total RAM (MB)** 💾
✅ **Identify macOS Version** 🏷️
✅ **List Available Disk Drives** 💽
✅ **Find MAC Address** 🔗
✅ **Detect Monitor Resolutions** 🖥️
✅ **View Installed RAM Slots & Capacity** ⚡

---

## 🛠️ Installation & Usage

### 📌 Prerequisites
🔹 macOS system (Intel/Apple Silicon) 🍏
🔹 C++11 or later 🚀
🔹 Xcode Command Line Tools (`xcode-select --install`) 🛠

### 📌 Compilation
Use **clang++** or **g++** to compile:
```sh
clang++ -std=c++11 -o system_info system_info.cpp -framework IOKit -framework CoreFoundation
```

### 📌 Running the Program
Execute the compiled binary:
```sh
./system_info
```

---

## 📌 Code Overview
🔹 **sysctl() API** ➝ Fetch CPU, RAM, OS details 🏗️
🔹 **IOKit Framework** ➝ Retrieve monitor resolutions, power sources 🖥️⚡
🔹 **getifaddrs() & AF_LINK** ➝ Obtain MAC address 🔗

### 🎨 Sample Output
```sh
🔹 CPU: Apple M1 Pro 🏎️
🔹 RAM: 16384 MB 💾
🔹 OS Version: 12.4 🍏
🔹 Disk Drives: disk0, disk1 💽
🔹 MAC Address: 00:1A:2B:3C:4D:5E 🔗
🔹 Monitors: 2560x1440, 1920x1080 🖥️
```

---

## 🔥 Future Enhancements
- [ ] 📡 **Battery Percentage Retrieval** ⚡
- [ ] 🎮 **GPU Details Extraction** 🕹️
- [ ] 🌐 **Network Speed Insights** 🚀

## 🎉 Contributions
🛠 **Fork & Submit Pull Requests!** 🏆

## 📜 License
🔖 Licensed under the **MIT License** 📝

---

🚀💙 Developed with passion by **Furkan Aşkın** 🏆🔥

