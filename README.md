# 🧮 DSA Mini Project — IT Infrastructure Management Software

This project demonstrates multiple **Data Structures and Algorithms** through a practical simulation of an **IT Infrastructure Management System**.

It integrates different data structures (Queue, Array/List, BST, Heap) to manage servers, assets, and maintenance tasks efficiently.

---

## 🚀 Project Overview

| Module | Data Structure Used | Description |
|--------|----------------------|-------------|
| **Server Health Queue** | Queue | Processes server health reports in FIFO order for real-time monitoring. |
| **Asset Inventory** | Array/List | Stores and searches for server assets by unique IDs, including status tracking. |
| **Load Balancer** | Binary Search Tree (BST) | Manages server loads and supports quick min/max load lookups. |
| **Maintenance Task Scheduler** | Heap (Min-Heap) | Prioritizes maintenance tasks so the most urgent is always processed first. |

---

## 🗂️ File Structure

dsa-mini-project/
│
├── main.c # Contains the main() function and top-level menu system
├── functions.c # Contains all module logic (Queue, BST, Heap, Array)
└── README.md # Project documentation


---

## 🛠️ Compilation and Execution

To compile and run the project:

### **Using GCC (Linux / macOS / Windows WSL)**
```bash
gcc main.c functions.c 
./a.exe
