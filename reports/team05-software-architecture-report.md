# Wireshark - Software Architecture Report

# Context Diagram (L1)

The System Context diagram is the first level of the C4 model. Its purpose is to place
Wireshark within its external environment — showing who uses it and which external
systems it depends on — without exposing any internal implementation detail.

![Context Diagram](./architecture-diagrams/context_diagram.svg)

## Main System

At the centre of the diagram sits **Wireshark System**, enclosed within the Wireshark Foundation boundary. It is a network protocol analyzer that captures, decodes, and visualizes network traffic. At this level of abstraction the system is a single black box; its internal structure (GUI, dumpcap, dissectors, etc.) is intentionally hidden.

## Actor

The only human actor is **User**: anyone who needs to inspect network traffic (network analyst, security engineer, or developer). The user drives the system and reads its output.

## External Systems

Two external libraries give Wireshark access to raw network traffic at the OS kernel level:

- **libpcap** — the standard capture library on Unix-like systems (Linux, macOS). It
  exposes APIs for packet capture, BPF filter compilation, and interface management.
- **npcap** — its functional equivalent on Windows, replacing the legacy WinPcap with
  updated driver support and modern Windows APIs.

Both are modelled as external systems because they fall outside the Wireshark Foundation's development boundary: Wireshark consumes their public APIs without owning their source code. Data flows from the library into Wireshark, not the other way around.

---

Delegating raw capture to external libraries is an intentional architectural choice: it abstracts away OS-specific packet capture APIs, making Wireshark portable across platforms without owning low-level driver code. The privilege boundary itself becomes explicit at the Container level, where `dumpcap` emerges as a separate elevated-privilege process while the GUI runs as a normal user.

---

# Container Diagram (L2)

The container diagram represents the second level of the C4 model and zooms into the
"Wireshark System" boundary. Here the system stops being a black box and breaks down
into its main processes and storage, showing how responsibilities are distributed and
how containers communicate with each other.

![Container Diagram](./architecture-diagrams/container_diagram.svg)

## Wireshark App

The Wireshark App is the main container, written in C, C++ and Qt. It is the process
that the user launches directly and interacts with via GUI. Its responsibilities cover
the entire analysis pipeline: orchestrating capture sessions, reading `.pcapng` files
produced by dumpcap, dissecting network protocols and displaying results on screen.

All the user's operational choices (interface, filters, save format) pass through this
container. The Wireshark App never directly accesses the network card: it entirely
delegates that responsibility to dumpcap, with which it communicates via IPC Sync Pipe.

## Dumpcap

Dumpcap is a separate executable written in C, designed to do one single thing:
intercept network packets and write them to disk in `.pcapng` format. It is the only
container in the system that requires elevated privileges (root on Unix, Administrator
on Windows), since it must open raw sockets via libpcap or npcap.

Confining privileged code in a minimal process drastically reduces the attack surface:
if a malicious packet were to cause a crash, the damage would be limited to dumpcap,
leaving the operating system and GUI intact. Dumpcap has no graphical interface or
analysis logic: it receives instructions from the Wireshark App via the sync pipe
(interface to capture, BPF filters, output path) and responds with status messages.

## File System

The File System is not a process but a storage, explicitly represented as a container
because it plays a precise architectural role: it acts as a shared buffer between
dumpcap and the Wireshark App.

Dumpcap writes packets to a temporary `.pcapng` file as fast as possible, avoiding
bottlenecks caused by the GUI rendering speed. The Wireshark App asynchronously reads
that same file to decode and display packets as they arrive. The same relationship also
covers the writing of permanent capture files saved by the user and the reading of
pre-existing `.pcapng` files opened for offline analysis.

---

The container diagram makes two further architectural decisions explicit:

1. Decoupling via IPC: communication between the GUI and dumpcap occurs via sync pipe,
   not via direct function calls. The two processes can crash, be updated or replaced
   independently of each other.

2. File System as shared buffer: writing to disk before displaying decouples the
   capture speed from the rendering speed, making the system robust even under high
   network traffic.

---

# Component Diagrams (L3)

The component diagram represents the third level of the C4 model and zooms into the
Wireshark App and Dumpcap containers, breaking them down into the components they are
made of.

## 1. Wireshark App


![Wireshark App Component Diagram](./architecture-diagrams/wiresharkapp_component_diagram.svg)


In this section we are analyzing the main components that compose the Wireshark Container.
Being Wireshark a monolith written almost entirely in C, we've decided to identify components as described on the C4 website, so as "a number of C files in a particular directory".

Some components, such as the various Utils, have not been included since they have been considered not relevant for the core system architecture.
Alongside each component, the coresponding directory will be indicated.

### Core
```console
/
```

Written in C, this component acts as the central brain or orchestrator of the application. It contains the primary business logic, coordinating data flow and state changes between the user interface, the capture subsystem, and the dissection engine.

### GUI
```console
/ui
```

The only C++ component, utilizes the Qt framework to maintain cross-platform visual consistency and handling asynchronous UI updates without blocking the main rendering thread.

### Capture
```console
/capture
```

A C-based abstraction layer acting as the direct interface to the underlying capture engine. It manages the lifecycle of capture sessions, including configuration, initialization, state tracking, and session termination.

The packet capture operation requires elevated system privileges (root or administrator access) to put network interfaces into promiscuous mode. To minimize security vulnerabilities, Wireshark runs with standard user privileges, while only Dumpcap runs with elevated privileges.

The Capture component launches and establishes a connection with Dumpcap via an Inter-Process Communication (IPC) Sync Pipe.

Dumpcap captures raw packets from the network interface and directly streams them into the File System as a temporary .pcapng file using high-performance File I/O.

Once a block of packets is successfully committed to disk, Dumpcap signals the Capture component via the IPC pipe, which subsequently alerts the Core.

By isolating the capture mechanics inside the standalone Dumpcap binary, the vast attack surface of the Wireshark App (especially the thousands of dissectors in Epan) is safely segregated from root-level OS access.

### Epan (Enhanced Packet ANalyzer)
```console
/epan
```

The packet analyzing engine, implemented in C. Epan is the most complex intellectual asset within Wireshark. It is responsible for packet dissection, dependency tracking between protocols, applying display filters, and executing plugins/macros.
It provides the following APIs:
  - Protocol Tree: dissection information for an individual packet.
  - Dissectors: the various protocol dissectors.
  - Display Filters: the display filter engine.

Moreover, Epan provides support for implementing custom dissectors as separate modules.

### Wiretap

```console
/wiretap
```
Wiretap is specialized C library purpose-built for file format abstraction.
It acts as a translation layer capable of reading and writing packet data across more than 20 distinct capture file formats (such as .pcap, .pcapng, and .cap).
Also, it enables the development of custom plug-ins, allowing developers to make custom packet filters.

### Complete flow

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

### Solid Principles analysis

The violations of the SOLID principles in Wireshark occur against the Interface Segregation Principle (ISP). Since C is a procedural language lacking native interface support, header files act as the nterfaces. Under this definition, major headers like the one for Epan (epan.h) heavily violate ISP: they expose a massive array of functions, forcing individual modules to depend on a sprawling API surface far larger than what they actually utilize.


## 2. Dumpcap

![Dumpcap Component Diagram](./architecture-diagrams/dumpcap_component_diagram.svg)

### Syncpipe Controller

The Syncpipe Controller manages all communications with the parent process (Wireshark
App) via the sync pipe. It propagates the start of the capture session into Dumpcap
and, in the opposite direction, receives updates on the active file and packet counter
from the Ring Buffer to forward them to Wireshark. This mechanism allows the GUI to
know in real time which file to read and how many packets have been captured.

### Capture Engine

The Capture Engine is the operational core of Dumpcap. It interfaces with libpcap/npcap
to receive raw packets from the network. During initialization it opens the capture handles
and applies the BPF filter directly on the handle via `pcap_setfilter()`: filtering
occurs at the kernel level, so the Capture Engine already receives only the packets
matching the user's filter. It then enters the listening loop and routes received
packets to the downstream components.

### Packet Queue

The Packet Queue is an asynchronous buffer (GAsyncQueue) that decouples packet
reception from disk writing in multi-thread mode. The Capture Engine enqueues packets
without waiting for the write to complete; a dedicated thread dequeues and delivers them
to the Ring Buffer. This decoupling prevents any slowness in writing from affecting
reception speed, reducing the risk of packet drops under high load.

### Ring Buffer

The Ring Buffer saves packets to disk in `.pcapng` format and manages automatic file
rotation based on configurable thresholds (size, duration, packet count). It receives
packets from the Packet Queue (multi-thread) or directly from the Capture Engine
(single-thread). Once written, it notifies the Syncpipe Controller with the current
file state and updated counters, closing the feedback loop toward Wireshark.

---

1. The Capture Engine integrates both privilege management and BPF filter setup directly
   during initialization, rather than delegating them to separate components.

2. The pipeline supports two write paths — multi-thread via the Packet Queue and
   single-thread via direct write — making it adaptable to varying network load without
   requiring changes to the other components.

3. Status information flows upward: the Ring Buffer notifies the Syncpipe Controller
   once packets are written, reflecting the actual direction of data in the pipeline.


## C4 Model - Context Level

The main software systems, stakeholders, and external libraries that interact directly with Wireshark as external entities are listed below:

# Architectural characteristics

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