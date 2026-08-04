# Mini Operating System Simulator - Test Cases

This document outlines standard test cases designed to verify the correctness of the simulation algorithms compared to theoretical Operating System calculations.

---

## Test Case 1: CPU Scheduling

### Input Process Pool
*   **P1:** Arrival = 0, Burst = 8, Priority = 3
*   **P2:** Arrival = 1, Burst = 4, Priority = 1
*   **P3:** Arrival = 2, Burst = 9, Priority = 4
*   **P4:** Arrival = 3, Burst = 5, Priority = 2

### Algorithm Expected Results

#### 1. FCFS (First Come First Served)
*   **Execution Order:** P1 $\to$ P2 $\to$ P3 $\to$ P4
*   **P1:** Finish = 8, Turnaround = 8, Waiting = 0
*   **P2:** Finish = 12, Turnaround = 11, Waiting = 7
*   **P3:** Finish = 21, Turnaround = 19, Waiting = 10
*   **P4:** Finish = 26, Turnaround = 23, Waiting = 18
*   **Averages:** Avg Waiting Time = **8.75 ms**, Avg Turnaround = **15.25 ms**

#### 2. SJF (Non-Preemptive Shortest Job First)
*   **Execution Order:** P1 (runs 0-8) $\to$ P2 (burst 4) $\to$ P4 (burst 5) $\to$ P3 (burst 9)
*   **P1:** Finish = 8, Turnaround = 8, Waiting = 0
*   **P2:** Finish = 12, Turnaround = 11, Waiting = 7
*   **P4:** Finish = 17, Turnaround = 14, Waiting = 9
*   **P3:** Finish = 26, Turnaround = 24, Waiting = 15
*   **Averages:** Avg Waiting Time = **7.75 ms**, Avg Turnaround = **14.25 ms**

#### 3. SRTF (Preemptive SJF)
*   **Execution Timeline:**
    *   Time 0: P1 starts.
    *   Time 1: P2 arrives (burst 4, remaining P1 is 7). P2 preempts P1.
    *   Time 5: P2 finishes. Remaining processes are P1 (rem 7), P3 (rem 9), P4 (rem 5). P4 runs.
    *   Time 10: P4 finishes. Remaining processes are P1 (rem 7), P3 (rem 9). P1 runs.
    *   Time 17: P1 finishes. P3 runs.
    *   Time 26: P3 finishes.
*   **P1:** Finish = 17, Turnaround = 17, Waiting = 9
*   **P2:** Finish = 5, Turnaround = 4, Waiting = 0
*   **P3:** Finish = 26, Turnaround = 24, Waiting = 15
*   **P4:** Finish = 10, Turnaround = 7, Waiting = 2
*   **Averages:** Avg Waiting Time = **6.50 ms**, Avg Turnaround = **13.00 ms**

#### 4. Round Robin (Quantum = 2)
*   **Execution Timeline:** P1 (0-2) $\to$ P2 (2-4) $\to$ P3 (4-6) $\to$ P1 (6-8) $\to$ P4 (8-10) $\to$ P2 (10-12, finishes) $\to$ P3 (12-14) $\to$ P1 (14-16) $\to$ P4 (16-18) $\to$ P3 (18-20) $\to$ P1 (20-22, finishes) $\to$ P4 (22-23, finishes) $\to$ P3 (23-26, finishes)
*   **Averages:** Avg Waiting Time = **10.25 ms**, Avg Turnaround = **16.75 ms**

---

## Test Case 2: Memory Allocation

### Partitions Configuration (Fixed Mode)
*   Partitions: `100MB, 150MB, 200MB, 250MB, 300MB`

### Allocation Queue
*   **Request 1:** P1 requires `120MB`
*   **Request 2:** P2 requires `220MB`
*   **Request 3:** P3 requires `90MB`

### Expected Mappings

#### 1. First Fit
*   **P1 (120MB):** Fits in Block 2 (150MB). Internal Frag = **30MB**.
*   **P2 (220MB):** Fits in Block 4 (250MB). Internal Frag = **30MB**.
*   **P3 (90MB):** Fits in Block 1 (100MB). Internal Frag = **10MB**.

#### 2. Best Fit
*   **P1 (120MB):** Fits in Block 2 (150MB). Internal Frag = **30MB**.
*   **P2 (220MB):** Fits in Block 4 (250MB). Internal Frag = **30MB**.
*   **P3 (90MB):** Fits in Block 1 (100MB). Internal Frag = **10MB**.

#### 3. Worst Fit
*   **P1 (120MB):** Claims Block 5 (300MB). Internal Frag = **180MB**.
*   **P2 (220MB):** Fits in Block 4 (250MB). Internal Frag = **30MB**.
*   **P3 (90MB):** Claims Block 3 (200MB). Internal Frag = **110MB**.

---

## Test Case 3: Page Replacement

### Reference String
*   `7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1`
*   Frame Slots: `3`

### Expected Results
*   **FIFO:** Page Faults = **15**, Page Hits = **5**
*   **LRU:** Page Faults = **12**, Page Hits = **8**
*   **Optimal:** Page Faults = **9**, Page Hits = **11**

---

## Test Case 4: Deadlock Handling (Banker's Algorithm)

### System State (5 Processes, 3 Resource Types A,B,C)
*   **Total System Resources:** `[10, 5, 7]`

#### Allocation Matrix
*   P0: `[0, 1, 0]`
*   P1: `[2, 0, 0]`
*   P2: `[3, 0, 2]`
*   P3: `[2, 1, 1]`
*   P4: `[0, 0, 2]`
*   *Sum of allocations:* `[7, 2, 5]`

#### Max Claim Matrix
*   P0: `[7, 5, 3]`
*   P1: `[3, 2, 2]`
*   P2: `[9, 0, 2]`
*   P3: `[2, 2, 2]`
*   P4: `[4, 3, 3]`

#### Resulting Available Resources
*   Available = `Total - Sum(Allocations)` = `[10-7, 5-2, 7-5]` = `[3, 3, 2]`

### Expected Banker's Safety Output
*   **Safety Status:** SAFE STATE
*   **Safe Sequence:** `P1 -> P3 -> P4 -> P0 -> P2` (or similar valid permutation)

---

## Test Case 5: Disk Head Scheduling

### Request Sequence
*   Requests: `98, 183, 37, 122, 14, 124, 65, 67`
*   Start Head Position: `53`
*   Initial Direction: `Right (towards 200)`
*   Cylinder Count: `200`

### Expected Trajectory results
*   **FCFS:**
    *   Path: `53 -> 98 -> 183 -> 37 -> 122 -> 14 -> 124 -> 65 -> 67`
    *   Total Head Seek: **640 cylinders**
*   **SSTF:**
    *   Path: `53 -> 65 -> 67 -> 37 -> 14 -> 98 -> 122 -> 124 -> 183`
    *   Total Head Seek: **236 cylinders**
*   **SCAN:**
    *   Path: `53 -> 65 -> 67 -> 98 -> 122 -> 124 -> 183 -> 199 -> 37 -> 14`
    *   Total Head Seek: **331 cylinders**
*   **C-SCAN:**
    *   Path: `53 -> 65 -> 67 -> 98 -> 122 -> 124 -> 183 -> 199 -> 0 -> 14 -> 37`
    *   Total Head Seek: **382 cylinders**
*   **LOOK:**
    *   Path: `53 -> 65 -> 67 -> 98 -> 122 -> 124 -> 183 -> 37 -> 14`
    *   Total Head Seek: **299 cylinders**
*   **C-LOOK:**
    *   Path: `53 -> 65 -> 67 -> 98 -> 122 -> 124 -> 183 -> 14 -> 37`
    *   Total Head Seek: **322 cylinders**
