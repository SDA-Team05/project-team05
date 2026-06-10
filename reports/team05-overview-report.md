# Wireshark - Overview

## Purpose of the System
Wireshark is the world's foremost and most widely-used network protocol analyzer (often referred to as a "sniffer"). It allows users to capture network traffic in real-time and observe what is happening on their network at a microscopic level. The primary purpose of Wireshark is to provide deep inspection of hundreds of protocols, enabling users to analyze packet details, filter traffic, and troubleshoot network problems.

Beyond simple packet capture, Wireshark acts as an essential diagnostic tool. It features a rich graphical user interface that provides color-coded packet visualization, advanced search and filtering capabilities, and statistical reports.

Wireshark can read and write a vast array of capture file formats (such as .pcap and .pcapng) and supports live capture across various platforms, including Windows, macOS, Linux, and UNIX.

## Main Stakeholders
*   **Wireshark Foundation:** The non-profit organization that handles the project, manages intellectual property, funds infrastructure, and organizes the global SharkFest educational conferences.
*   **Developers**: Software Engineers that maintain the code and update it to support new protocols.
*   **Educators and Students:** Wireshark is widely used as a practical educational tool to learn about network protocol internals and observe theoretical networking concepts in action.
*   **End users:** This broad category includes professionals who utilize Wireshark for daily operations. Some examples are:
    *   **Network Administrators and Engineers:** Utilize the tool for routine network troubleshooting, performance analysis, and identifying connectivity or routing issues.
    *   **Security Professionals:** Rely on Wireshark for examining security breaches, analyzing malicious traffic, and conducting network forensics.
    *   **Software Developers:** Use the system to debug protocol implementations, client-server communications, and networked applications.

## System Description
Wireshark is a complex and highly modular open-source software project. It is primarily written in C, with the GUI written in C++. The architecture separates concerns into distinct layers:
- A packet capture layer leveraging `libpcap`/`npcap`.
- A `core` that handles the packet analysis and dissection, thanks to the dedicated packet analysis engine `epan` (Ethereal Packet Analysis Engine, named after the original name of Wireshark, *Ethereal*).
- The Graphical User Interface (`gui`).

The system is highly extensible, allowing developers to add custom protocol dissectors via plugin support.

## Basic Code Statistics
Based on an analysis of the repository's current state, here are the fundamental metrics of the Wireshark codebase, as of June 10th, 2026:

*   **Total Files:** 7,411 files
*   **Lines of Code (LOC):** ~7,929,660 lines of code
*   **Modules / Packages:** ~25 main top-level modules (Key directories include `epan`, `wiretap`, `ui`, `plugins`, `wsutil`, `capture`, `extcap`, etc.)
*   **Core Developers:** 34 Active Developers, 11 Recently Active Developers, 13 Non Active/Ex Developers
    *   **Source**: [https://wiki.wireshark.org/Developers](https://wiki.wireshark.org/Developers)
*   **Contributors:** 1899 unique contributors who have submitted patches, bug fixes, and new dissectors over the project's lifetime.
    *   **Source**: Git repository
