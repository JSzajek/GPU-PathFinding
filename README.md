# **GPU Path Finding**
A project to develop algorithms using OpenCL to improve Path Finding.

## **Getting Started**

### **Prerequisites**
Ensure you have the following installed on your system:
- **C++20** or later
- A modern **C++ compiler** (e.g., GCC, Clang, MSVC)

### **How to Build**
#### **Step 1: Clone the Repository**
```
git clone https://github.com/JSzajek/GPU-PathFinding.git
```

#### **Step 2: Setup Project Using Premake**
##### **Windows**
```
Run GenerateProjects.bat
```

#### **Step 3: Build the Project**
##### **Windows**
Open the generated .sln file and build the project.

## **Current State**
- Heuristic Path Finding

<img src="/OutputExample/Heuristic_PathFinding.gif" alt="Heuristic_PathFinding" width="512"/>

Works but doesn't handle navigating around hard edges well.


## **License**
This project is licensed under the Apache License. See the LICENSE file for more details.