# Architecture Report

## C4 Model - Context Level

The main software systems, stakeholders, and external libraries that interact directly with Wireshark as external entities are listed below:

### External Libraries

#### libpcap/npcap
For security and stability reasons, the operating system (Windows, macOS, or Linux) does not allow a standard program (Wireshark) to talk directly to the network interface card. To solve this problem, Wireshark's architecture relies on external libraries (`libpcap` for macOS/Linux, `npcap` for Windows) provided by the operating system.
*   Wireshark's capture module (`dumpcap`) calls the `libpcap`/`npcap` library APIs, which request to capture data traffic through special permissions. Parameters entered in the `dumpcap` command are translated by `libpcap`/`npcap` via the `pcap_compile()` function. Through a mini-compiler, the string is transformed into a machine language called **BPF (Berkeley Packet Filter) Bytecode**. Via the `pcap_setfilter()` function, the newly compiled mini-program can be injected directly into the **Kernel** of the Operating System (the operation performed in *Kernel Space* allows for a significant increase in temporal efficiency).
*   During listening, the Kernel records the data passing through the network and packs it into a memory buffer. The byte packets are retrieved by the `libpcap`/`npcap` functions to be normalized into a C language data structure that is uniform for all, called `struct pcap_pkthdr` (**Pcap Packet Header**). Fundamental metadata (*timestamp*, *original-length*, *captured-length*) is added to this container.

### Software Systems

#### File System
*   To avoid bottlenecks (**I/O Bottleneck**), `dumpcap` writes packets at maximum speed to a temporary `.pcapng` file in the File System. Simultaneously, the Wireshark GUI asynchronously reads that same temporary file from the File System to display the data on the screen. In this way, the File System acts as a shared buffer between the two processes.
*   At startup, Wireshark queries the File System to search for dynamic library files (`.dll` on Windows, `.so` on Linux) or Lua language scripts within specific folders. If found, it loads them into memory (**Open-Closed Principle**).
*   Wireshark continuously reads from and writes to the File System configuration files (*preferences*, *colorfilters*, *recent*) located in the user directory. This allows for the isolation of settings (**Separation of Concerns**) and the transfer of entire "Profiles" from one PC to another by simply copying a folder.

### Stakeholders

---

## C4 Model - Container Level

The main containers of Wireshark are listed below:

### dumpcap (.exe/.app)
A command-line program written in C whose sole purpose is to capture network packets and save them to disk.

*   **Invocation and Configuration:** When a network capture begins, the `dumpcap` executable is launched in the background with any necessary parameters. The parameters entered via the UI (interface, path, capture-filter, promiscuous-mode, etc.) are translated by the UI to generate a long command line to invoke `dumpcap`. The latter, in turn, calls the Operating System library APIs (`libpcap`/`npcap`) to capture network traffic in the desired mode.
*   **Capture Cycle:** `dumpcap` enters a continuous listening loop (via the `pcap_loop()` function). Every time a physical packet crosses the network card, the Kernel copies it into a memory buffer; `libpcap` normalizes it and invokes a function (callback) inside `dumpcap` to deliver the packet.
*   **Persistence:** `dumpcap` takes the normalized packet from `libpcap`/`npcap` and writes it to a temporary file on the hard disk (in `.pcapng` format).
*   **IPC Communication:** `dumpcap` notifies the Wireshark GUI of the saving of new packets via a system pipe. The GUI reads the file asynchronously, decodes the bytes, and displays them on the screen for the user.

Only `dumpcap` needs to run with Administrator/Root privileges to access the network card. The Wireshark GUI (which is massive and potentially vulnerable) runs as a normal user. If a malicious packet crashes the GUI, the operating system remains safe. Furthermore, by being separate, `dumpcap` handles only the I/O operation (Network -> Disk) at maximum speed.

## Component Level

### Wireshark

**DIAGRAM**

In this section we are analyzing the main components that compose the Wireshark Container.
Being Wireshark a monolith written almost entirely in C, we've decided to identify components as described on the C4 website, so as "a number of C files in a particular directory".

Some components, such as the various Utils, have not been included since they have been considered not relevant for the core system architecture.
Alongside each component, the coresponding directory will be indicated.

#### Core
```console
/
```

Written in C, this component acts as the central brain or orchestrator of the application. It contains the primary business logic, coordinating data flow and state changes between the user interface, the capture subsystem, and the dissection engine.

#### GUI
```console
/ui
```

The only C++ component, utilizes the Qt framework to maintain cross-platform visual consistency and handling asynchronous UI updates without blocking the main rendering thread.

#### Capture
```console
/capture
```

A C-based abstraction layer acting as the direct interface to the underlying capture engine. It manages the lifecycle of capture sessions, including configuration, initialization, state tracking, and session termination.

The packet capture operation requires elevated system privileges (root or administrator access) to put network interfaces into promiscuous mode. To minimize security vulnerabilities, Wireshark runs with standard user privileges, while only Dumpcap runs with elevated privileges.

The Capture component launches and establishes a connection with Dumpcap via an Inter-Process Communication (IPC) Sync Pipe.

Dumpcap captures raw packets from the network interface and directly streams them into the File System as a temporary .pcapng file using high-performance File I/O.

Once a block of packets is successfully committed to disk, Dumpcap signals the Capture component via the IPC pipe, which subsequently alerts the Core.

By isolating the capture mechanics inside the standalone Dumpcap binary, the vast attack surface of the Wireshark App (especially the thousands of dissectors in Epan) is safely segregated from root-level OS access.

#### Epan (Enhanced Packet ANalyzer)
```console
/epan
```

The packet analyzing engine, implemented in C. Epan is the most complex intellectual asset within Wireshark. It is responsible for packet dissection, dependency tracking between protocols, applying display filters, and executing plugins/macros.
It provides the following APIs:
  - Protocol Tree: dissection information for an individual packet.
  - Dissectors: the various protocol dissectors.
  - Display Filters: the display filter engine.

Moreover, Epan provides support for implementing custom dissectors as separate modules.

#### Wiretap

```console
/wiretap
```
Wiretap is specialized C library purpose-built for file format abstraction.
It acts as a translation layer capable of reading and writing packet data across more than 20 distinct capture file formats (such as .pcap, .pcapng, and .cap).
Also, it enables the development of custom plug-ins, allowing developers to make custom packet filters.

#### Complete flow

**Initialization**

The process begins when a user initiates a capture through the Graphical User Interface. Because capturing raw network packets requires elevated system privileges (such as root or administrator access), Wireshark deliberately isolates this risk. The Core does not launch the capture itself; instead, it delegates this task to the Capture component.

The Capture component acts as the bridge-builder, spawning and initializing a completely separate, highly privileged command-line utility called Dumpcap. By keeping Dumpcap isolated, the rest of the massive Wireshark application can safely run with standard user privileges.

**The Capture Loop**

Once active, Dumpcap interfaces directly with the network card, grabbing raw packets off the wire at lightning speed. To prevent memory bloat during massive data streams, Wireshark adopts a "write-first, read-later" strategy. Dumpcap continuously writes these raw packets directly into a temporary file on the local hard disk.

The coordination from this point forward is synchronized:

As soon as Dumpcap finishes writing a block of packets to the disk, it notifies the Capture component via an Inter-Process Communication (IPC) pipe.

The Capture component immediately passes this signal up to the application's central brain, the Core, letting it know that fresh data is ready to be processed.

**Reading and Dissection**

Once receiving the notification from the Capture component, the Core calls upon Wiretap. Wiretap opens the temporary file on the hard disk, reads the newly written raw packet blocks, and abstracts away the underlying file format complexities.

Once Wiretap reads the data, the Core passes the raw bytes to Epan (the Enhanced Packet Analyzer) to dissect the protocols, and finally routes the structured, human-readable results back up to the GUI for the user to see.

The Core then passes these raw bytes to Epan for deep protocol dissection.

Once Epan completes the analysis, the Core pushes the structured data back up to the GUI via Signals & Slots to update the screen for the end-user.

#### Solid Principles analysis

The violations of the SOLID principles in Wireshark occur against the Interface Segregation Principle (ISP). Since C is a procedural language lacking native interface support, header files act as the nterfaces. Under this definition, major headers like the one for Epan (epan.h) heavily violate ISP: they expose a massive array of functions, forcing individual modules to depend on a sprawling API surface far larger than what they actually utilize.

## Architectural characteristics

Wireshark is a premier network protocol analyzer whose long-term success relies heavily on a robust, highly modular software architecture. Its system design prioritizes key architectural characteristics—primarily **extensibility**, **maintainability**, and **portability**—which are intentionally supported by low component coupling and high functional cohesion.

---

### Extensibility

Network environments continuously evolve, requiring Wireshark to support thousands of active protocols. To manage this scale without bloated, unmanageable code, Wireshark relies heavily on a core architectural quality: **extensibility**.

#### Architectural Support

This characteristic is achieved through a pluggable pipeline architecture driven by **"dissectors"** (modules that break down packet payloads). The core engine dictates *when* packets are processed but leaves the *how* to individual protocol plugins. Wireshark utilizes a registration design pattern, allowing new dissectors (written in C or Lua) to dynamically attach themselves to the engine at runtime.

#### Coupling & Cohesion Analysis

* **Low Component Coupling:** The coupling between the core engine (`epan`) and individual protocol dissectors is highly minimized. They communicate through standard, abstract Data Coupling (passing raw packet buffers and standard header parameters). A change within the HTTP/2 dissector will not break the core architecture or impact the DNS dissector.
* **High Functional Cohesion:** Each dissector possesses ultimate functional cohesion; it has exactly one responsibility—parsing a specific layer of a single protocol format.

---

### Maintainability (Separation of Concerns)

Wireshark is dual-purpose: it handles intensive backend processing (bit-level packet capture, state tracking) alongside complex frontend visualization (packet trees, coloring rules, charts). Merging these domains would make maintenance an absolute nightmare.

#### Architectural Support

The architecture structurally enforces a strict **Separation of Concerns** by splitting the application into distinct layers:

* **Capture Layer:** A lightweight, isolated process dedicated solely to packet capture.
* **Analysis Layer:** The engine that handles packet dissection and protocol state machines.
* **UI Layer:** The graphical interface that presents data to users.

#### Coupling & Cohesion Analysis

* **Low Coupling via IPC:** The capture engine (`dumpcap`) is completely decoupled from the graphical UI layer. They run as separate processes and pass data using standard streams or pipes. If the GUI freezes or crashes under a heavy rendering load, packet capturing remains entirely uninterrupted.
* **High Layer Cohesion:** Each architectural layer exhibits pristine **Layer Cohesion**. The UI layer contains zero packet-parsing logic—it simply queries APIs exposed by `epan`. This means developers can completely overhaul or replace the user interface without rewriting a single line of the underlying decoding algorithms.

---

### Portability

Wireshark must reliably run across a diverse range of environments, from Linux servers using command-line workflows (`tshark`) to Windows and macOS desktops.

#### Architectural Support

Portability is built into the architecture by isolating OS-specific operations behind clean hardware-abstraction libraries. Instead of implementing direct network card packet captures natively, Wireshark relies on the **`libpcap`/`winpcap**` ecosystem.

#### Coupling & Cohesion Analysis

* **Low Logical Coupling:** By wrapping platform-specific dependencies into a distinct abstraction layer, the core dissection engine is logically decoupled from host operating system calls.
* **High Cohesion:** The capture backend focuses strictly on bridging network hardware buffers to cross-platform standard file formats (`pcapng`). This clean separation ensures that adapting Wireshark to new operating systems rarely requires modifications to its massive, core protocol dissection libraries.