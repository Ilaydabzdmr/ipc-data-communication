# ⚙️ Inter-Process Communication in C (Pipe vs Shared Memory)

This project demonstrates **Inter-Process Communication (IPC)** mechanisms in C using:

- 🔄 Pipes
- 🧠 Shared Memory

The goal is to compare different IPC techniques in terms of performance and behavior.

---

## 🚀 PROJECT OVERVIEW

The project simulates communication between processes using two approaches:

- **Pipe-based communication**
- **Shared memory-based communication**

Each implementation processes data and logs results for comparison.

---

## 📌 KEY CONCEPT

This project highlights the trade-off between simplicity (pipes) and performance (shared memory) in IPC design.

---

## 🧩 IPC METHODS

### 🔄 Pipes
- Unidirectional communication  
- Kernel-managed buffer  
- Simple but limited performance  

### 🧠 Shared Memory
- Direct memory access between processes  
- Faster data exchange  
- Requires synchronization  

---

## 📂 PROJECT STRUCTURE



├── src/

│ ├── pipe/

│ └── shared_memory/

├── scripts/

│ └── test_script.sh

├── logs/

│ ├── db_server_logs.txt

│ └── web_server_logs.txt

└──  README.md


---

## ⚙️ COMPILATION

Compile pipe version:
- gcc src/pipe/myData_pipe.c -o pipe_data
- gcc src/pipe/myMore_pipe.c -o pipe_more

Compile shared memory version:
- gcc src/shared_memory/myData_shm.c -o shm_data
- gcc src/shared_memory/myMore_shm.c -o shm_more


---

## ▶️ USAGE

Run executables:
./pipe_data
./shm_data

Or use the test script:
bash scripts/test_script.sh

---

## 📊 OUTPUT
- Process communication logs
- Data transfer behavior
- Performance comparison insights

---

## 🧠 LEARNING OUTCOMES
- Understanding IPC mechanisms in Linux
- Comparing Pipe vs Shared Memory
- Low-level system programming in C

---

## LICENSE

MIT License
