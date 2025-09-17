# 🧠 Memory Layout Benchmarks

This repository has **two main goals**:  

1. **Sharpen my understanding of memory and cache behavior** in C++.  
2. **Showcase how optimized data structures can improve program performance** using simple, practical examples.  

---

### `AoS-SoA/`

This folder demonstrates the difference between two common **data layout strategies**:  

- **AoS (Array of Structures)** – store complete objects in a single array.  
- **SoA (Structure of Arrays)** – store each field of the objects in separate arrays.  

---

## ⚡ Benchmark Explanation

The benchmark compares **AoS** and **SoA** for **field-centric operations**:  

- **Field-centric operations**:  
  When we often access the *same field* across many objects.  
  - **SoA is faster** because values of the same field are stored contiguously in memory, improving **cache utilization**.  

- **Object-centric operations**:  
  When we frequently work with *entire objects*.  
  - **AoS is more efficient** because all fields of an object are stored together in memory.  

---

## 📊 Example Results
<img width="227" height="54" alt="Screenshot_1" src="https://github.com/user-attachments/assets/b9c8187a-693c-4395-a65b-4226fc1f7cba" />

---

## 🛠️ How to compile

To run it, just clone the repository and compile using g++ or any C++17-compatible compiler.

If u compile with g++, run this:
---
g++ -O2 -std=c++17 aos_vs_soa.cpp -o benchmark
./benchmark
---
We use "-O2" to enable compiler optimizations for more realistic results.
