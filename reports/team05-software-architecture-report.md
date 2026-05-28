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

## Solid Principles violations
- Single-Responsibility: respected, each component interacts with only one other.
- Open/Closed: respected
  - EPAN uses a plugin architecture to allow to add custom Dissectors
  - It's possible to add new formats to Wiretap
- Liskov Substitution: respected in GUI (Qt framework), not applicable to the rest of the code.
- Interface Segregation: violated, many interfaces (headers) are very big (EPAN)
- Dependency Inversion: interfaces are not present, functions are called directly
