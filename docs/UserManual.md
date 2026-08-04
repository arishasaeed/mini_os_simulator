# Mini Operating System Simulator - User Manual

This manual provides detailed guidelines on launching, configuring, and analyzing each of the 8 simulation modules within the **Mini Operating System Simulator**.

---

## 1. Getting Started

1.  **Launch the Application:** Compile and execute the program using CMake or Qt Creator.
2.  **Dark/Light Mode Toggling:** Click the theme toggle button (e.g. `☀️ Light Mode` or `🌙 Dark Mode`) in the top-right header to instantly redraw the interface.
3.  **Real-Time Status Header:** The header displays:
    *   `Processes:` Total processes currently created in the PCB pool.
    *   `Free Memory:` Total remaining memory in the allocation system.
    *   `VFS Free:` Total remaining blocks in the Virtual File System disk.

---

## 2. Module Guidelines

### Module 1: Process Management
Allows you to simulate a process pool, inspect Process Control Blocks (PCBs), and observe state transitions.
*   **Create Process:** Enter a custom process name (optional), select Arrival Time, CPU Burst Time, Priority, and Memory requirements. Click `Create Process`. A new READY process is added to the pool.
*   **PCB Inspection:** The table lists all active processes, their PIDs, and metadata.
*   **State Control:** Select a process in the table and:
    *   Click `Suspend Selection` to transition the process state to **WAITING**.
    *   Click `Resume Selection` to move the process back to **READY**.
    *   Click `Delete Selection` to remove the process from the system.
*   **State Transition Monitor:** Select any process in the table. The lower visualizer will highlight its current state circle in the standard state transition diagram.

---

### Module 2: CPU Scheduling
Provides scheduling simulations with Gantt charts.
*   **Load Active Processes:** Click `Load Active Processes` to pull the process pool from Module 1.
*   **Select Algorithm:** Choose from FCFS, SJF (Preemptive/Non-preemptive), Priority (Preemptive/Non-preemptive), or Round Robin.
*   **Configure Quantum:** For Round Robin, adjust the `Time Quantum` spin box.
*   **Run Simulation:** Click `Run Simulation` to execute the scheduling strategy:
    *   **Gantt Chart:** The timeline block chart will display the time slices assigned to each process in order.
    *   **Metrics Table:** View individual finish times, turnaround times ($T_{finish} - T_{arrival}$), waiting times ($T_{turnaround} - T_{burst}$), and response times.
    *   **Aggregate Stats:** Check the bottom bar for Avg Waiting Time, Avg Turnaround, CPU Utilization, and Throughput.

---

### Module 3: Memory Management
Simulates memory partition allocation and fragmentation.
*   **Initialize Memory:**
    *   Select **Fixed Partitioning** and enter a comma-separated list of partition sizes (e.g., `100,150,200,250`) to define static blocks.
    *   Select **Dynamic Partitioning** to initialize a single, contiguous 1000MB free memory block.
    *   Click `Reinitialize Memory`.
*   **Allocate Process:** Enter the process ID, size requirement in MB, and select the allocation algorithm (**First Fit**, **Best Fit**, **Worst Fit**, or **Next Fit**). Click `Request Allocation`.
*   **Memory Block Map:** The horizontal bar visualizes:
    *   **Blue Blocks:** Allocated memory labeled with process details.
    *   **Slate Blocks:** Free space.
    *   **Red Slices:** Internal fragmentation (in Fixed mode).
*   **Deallocate Process:** Select an allocated block in the table and click `Release Chosen Partition`. In Dynamic mode, adjacent free spaces will automatically coalesce (merge) to eliminate external fragmentation.

---

### Module 4: Page Replacement
Demonstrates how operating systems handle virtual memory page replacements.
*   **Inputs:** Enter a comma-separated reference string (e.g., `7,0,1,2,0,3`) and configure the number of frame slots (e.g., `3` or `4`).
*   **Select Policy:** Choose between **FIFO**, **LRU**, and **Optimal**. Click `Run Simulation`.
*   **Grid Grid Display:**
    *   Columns represent steps in the page reference string.
    *   Rows represent frame slots.
    *   Green highlighted final row indicates a **Page Hit** (the page was already in memory).
    *   Red highlighted final row indicates a **Page Fault** (a page was replaced).
*   **Counters:** View Page Hits, Page Faults, and Page Fault Rate.

---

### Module 5: Deadlock Handling
Provides simulations of Banker's safety state checking and Resource Allocation Graphs (RAG).
*   **Initialize Matrices:** Input the process count, resource types count, and total resource vector (e.g., `10, 5, 7` for A, B, and C). Click `Initialize`.
*   **Edit Matrices:** Directly type integer values into the table cells:
    *   `Allocation Matrix:` Resources currently held.
    *   `Max Matrix:` Max resources the process might request.
    *   `Request Matrix:` Active requests causing potential deadlock.
*   **Banker's Safety Check:** Click `Calculate Banker's Safety Sequence` to run the safety algorithm. The output will show the safe sequence (e.g., `P1 -> P3 -> P4 -> P2 -> P0`) or alert you if the state is unsafe.
*   **Graph Deadlock Cycle Detection:** Click `Detect Graph Deadlock Cycle`. The Resource Allocation Graph (RAG) visualizer will plot process circles and resource squares. If a deadlock cycle is detected, the participating nodes and arrows will turn bright red.
*   **Recovery:** Enter the process ID in the `Term ID` spin box and click `Kill Process` to release its holdings back to the available pool, resolving the deadlock.

---

### Module 6: Disk Scheduling
Simulates disk head movements on a magnetic platter.
*   **Configure Head:** Input the starting cylinder position and the number of cylinders (e.g. 200).
*   **Configure Requests:** Input a comma-separated request list (e.g. `98, 183, 37`).
*   **Run Algorithm:** Select FCFS, SSTF, SCAN, C-SCAN, LOOK, or C-LOOK, select the initial search direction, and click `Run Simulation`.
*   **Seek Graph:** The visualizer plots head movement trajectory. Dotted lines indicate resets (in circular modes).
*   **Metrics:** Shows Total Cylinder Seek Distance and Average Seek Time.

---

### Module 7: Virtual File System
A database-free virtual directory structure showing physical block allocation.
*   **Tree Navigation:** View folders and files in a hierarchical tree. Double-click a folder to enter it. Click `Go to Parent (..)` to move up.
*   **Create Folder:** Enter a name and click `Make Folder`.
*   **Create File:** Enter a name, size in blocks, and select the allocation method:
    *   **Sequential:** Requires contiguous blocks.
    *   **Linked:** Chains free blocks together.
    *   **Indexed:** Allocates an index block pointing to data blocks.
*   **Physical Allocation Blocks Grid:** The 64-block grid dynamically colors sectors occupied by files.
    *   **Sequential files** appear as contiguous block blocks.
    *   **Linked files** claim scattered blocks.
    *   **Indexed files** claim blocks plus a yellow index block labeled `(IDX)`.
*   **Rename & Delete:** Select a file in the tree, enter a new name, or click `Delete Selected Item` to release the block sectors.

---

### Module 8: Performance Analytics
Aggregates scheduling metrics for comparative academic presentations.
*   Click `Refresh Comparison Metrics` to run CPU, Page, and Disk algorithms on your current input sets.
*   Inspect side-by-side tables comparing waiting times, page fault rates, and seek distances.
