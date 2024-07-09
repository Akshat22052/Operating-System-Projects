# Operating-System-Projects

# Project Overview

This repository contains several projects developed as part of a hands-on learning experience in system-level programming and operating systems concepts. Below are detailed descriptions of each project.

---

### SimpleScheduler

**Description:**
SimpleScheduler is a basic process scheduler written in C, designed to simulate CPU time-sharing by managing multiple processes across a limited number of CPU cores. It employs a priority-based scheduling policy where processes receive a set quantum of execution time before being placed back into the ready queue.

**Features:**
- Processes can be assigned a priority level from 1 to 4, with a default priority of 1 if none is specified.
- Uses a heuristic to determine how a process's priority affects its execution order.
- Generates statistical output to demonstrate how different priority levels impact job scheduling and execution.

**Usage:**
Detailed instructions on how to compile and run the SimpleScheduler will be provided in the project directory.

---

### SimpleShell

**Description:**
SimpleShell is a command-line interpreter that runs in a loop, processing user commands, executing them, and waiting for more input until a termination signal (Ctrl-C) is received. This project serves as a practical exercise in managing system calls related to process handling in a Unix-like environment.

**Features:**
- Executes existing Unix commands.
- Supports pipeline implementation to facilitate the execution of multiple commands at once.
- Includes functionality for creating daemon processes.

**Usage:**
Detailed instructions on how to compile and run the SimpleShell will be provided in the project directory.

---

### SmartLoader

**Description:**
SmartLoader enhances a previous assignment's SimpleLoader, evolving it into the more advanced SimpleSmartLoader. This new loader is considered "smart" because it employs a lazy loading strategy, similar to modern operating systems like Linux, loading program segments into memory only when needed during execution.

**Features:**
- Avoids upfront bulk loading, using a page-by-page allocation approach.
- Manages multiple page faults efficiently.
- Works exclusively with 32-bit ELF executables without using any standard glibc APIs.

**Usage:**
Detailed instructions on how to compile and run the SmartLoader will be provided in the project directory.

---

### Multithreading

**Description:**
The Multithreading project is a basic implementation of multithreading concepts in C. It demonstrates how to create, manage, and synchronize multiple threads within a program.

**Features:**
- Creation and management of multiple threads.
- Synchronization mechanisms to avoid race conditions and ensure thread safety.

**Usage:**
Detailed instructions on how to compile and run the Multithreading project will be provided in the project directory.

---

### SimpleLoader

**Description:**
SimpleLoader is a basic program loader that focuses on loading and executing 32-bit ELF executables. This project serves as an introduction to the concepts of program loading and memory management.

**Features:**
- Loads program segments into memory for execution.
- Basic memory management without the advanced features of SmartLoader.

**Usage:**
Detailed instructions on how to compile and run the SimpleLoader will be provided in the project directory.

---

## Getting Started

### Prerequisites

- GCC Compiler
- Make (optional, for ease of compilation)
- Unix-like operating system (Linux, macOS, etc.)

### Installation

Clone the repository to your local machine:
```bash
git clone https://github.com/Akshat22052/Operating-System-Projects
cd your-repository
