# C4 System Context Diagram (L1) — Wireshark

The System Context diagram is the first level of the C4 model. Its purpose is to place
Wireshark within its external environment — showing who uses it and which external
systems it depends on — without exposing any internal implementation detail.

```plantuml
@startuml C4_Elements
!include https://raw.githubusercontent.com/plantuml-stdlib/C4-PlantUML/master/C4_Context.puml

title C4 System Context Diagram (L1) - Wireshark

Person(wiresharkUser, "User", "A person who wants to analyze network traffic")

Boundary(wiresharkBoundary, "Wireshark", "Wireshark Foundation") {
    System(wireshark, "Wireshark System", "Network protocol analyzer. Captures, decodes, and visualizes network traffic.")
}

System_Ext(npcap, "npcap", "Captures raw packets on Windows systems")
System_Ext(libpcap, "libpcap", "Captures raw packets on Unix-like systems")

Rel(wiresharkUser, wireshark, "Configures captures and analyzes traffic")
Rel(npcap, wireshark, "Provides raw packets")
Rel(libpcap, wireshark, "Provides raw packets")

SHOW_LEGEND()
@enduml
```

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

# C4 Container Diagram (L2) — Wireshark

The container diagram represents the second level of the C4 model and zooms into the
"Wireshark System" boundary. Here the system stops being a black box and breaks down
into its main processes and storage, showing how responsibilities are distributed and
how containers communicate with each other.

```plantuml
@startuml C4_Elements_L2
!include https://raw.githubusercontent.com/plantuml-stdlib/C4-PlantUML/master/C4_Container.puml

title C4 Container Diagram (L2) - Wireshark

Person(wiresharkUser, "User", "A person who wants to analyze network traffic")

System_Ext(npcap, "npcap", "Captures raw packets on Windows systems")
System_Ext(libpcap, "libpcap", "Captures raw packets on Unix-like systems")

System_Boundary(wiresharkBoundary, "Wireshark System") {
    Container(wiresharkApp, "Wireshark App", "Container: C, C++, Qt", "GUI application. Orchestrates the capture, reads files, decodes protocols and displays data.")
    Container(dumpcap, "Dumpcap", "Container: C", "Privileged executable. Intercepts network traffic and saves raw packets directly to disk.")
    ContainerDb(filesystem, "File System", "Container: Local OS File System", "Stores temporary and saved .pcapng files")
}

Rel(wiresharkUser, wiresharkApp, "Consults packets and configures capture")
Rel(wiresharkApp, dumpcap, "Controls and monitors status", "IPC Sync Pipe")
Rel(npcap, dumpcap, "Provides raw packets", "Windows calls")
Rel(libpcap, dumpcap, "Provides raw packets", "Unix calls")
Rel(dumpcap, filesystem, "Writes raw packets to", "File I/O")
Rel(wiresharkApp, filesystem, "Reads packets from and writes packets to", "File I/O")

SHOW_LEGEND()
@enduml
```

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

# C4 Component Diagrams (L3)

The component diagram represents the third level of the C4 model and zooms into the
Wireshark App and Dumpcap containers, breaking them down into the components they are
made of.

## 1. Wireshark App

<!-- TODO: C4 L3 WIRESHARK-APP -->

## 2. Dumpcap

```plantuml
@startuml C4_Elements_L3
!include https://raw.githubusercontent.com/plantuml-stdlib/C4-PlantUML/master/C4_Component.puml

title C4 Component Diagram (L3) - Dumpcap

Container(wireshark, "Wireshark App", "Container: C, C++, Qt", "GUI application. Orchestrates the capture, reads files, decodes protocols and displays data.")
ContainerDb(filesystem, "File System", "Container: Local OS File System", "Stores temporary and saved .pcapng files")
System_Ext(npcap, "npcap", "Captures raw packets on Windows systems")
System_Ext(libpcap, "libpcap", "Captures raw packets on Unix-like systems")

Container_Boundary(dumpcap_res, "Dumpcap") {

    Component(sync_pipe, "Syncpipe Controller", "Component: C", "Manages IPC communication and control signals with the parent process.")

    Component(cap_eng, "Capture Engine", "Component: C", "Interfaces with the OS network libraries to fetch raw frames.")

    Component(packet_queue, "Packet Queue", "Component: C", "Decouples packet reception from file writing in multi-thread mode.")

    Component(file_writer, "Ring Buffer", "Component: C", "Manages asynchronous writing and automatic rotation of pcapng files.")
}

Rel(wireshark, sync_pipe, "Launches and communicates with", "IPC Sync Pipe")
Rel(sync_pipe, cap_eng, "Initiates capture session", "Internal Call")
Rel(npcap, cap_eng, "Provides raw packets", "Windows calls")
Rel(libpcap, cap_eng, "Provides raw packets", "Unix calls")
Rel(cap_eng, packet_queue, "Enqueues packets (multi-thread mode)", "GAsyncQueue")
Rel(packet_queue, file_writer, "Dequeues and forwards packets", "Memory Buffer")
Rel(cap_eng, file_writer, "Writes packets directly (single-thread mode)", "Direct Call")
Rel(file_writer, sync_pipe, "Reports file state and packet counters", "Internal Call")
Rel(file_writer, filesystem, "Writes/Read raw packets to/from", "File I/O")

SHOW_LEGEND()
@enduml
```

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
