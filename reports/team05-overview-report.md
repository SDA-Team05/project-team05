# Wireshark Software Architecture - Overview

## Purpose of the System
Wireshark is the world's foremost and most widely-used network protocol analyzer (often referred to as a "sniffer"). It allows users to capture network traffic in real-time and observe what is happening on their network at a microscopic level. The primary purpose of Wireshark is to provide deep inspection of hundreds of protocols, enabling users to analyze packet details, filter traffic, and troubleshoot network problems. It features a rich graphical user interface (built with Qt) alongside command-line equivalents (like TShark). Wireshark can read and write a vast array of capture file formats and supports live capture across various platforms.

## Main Stakeholders
*   **Network Administrators and Engineers:** Utilize the tool for routine network troubleshooting, performance analysis, and identifying connectivity or routing issues.
*   **Security Professionals:** Rely on Wireshark for examining security breaches, analyzing malicious traffic, and conducting network forensics.
*   **Software Developers:** Use the system to debug protocol implementations, client-server communications, and networked applications.
*   **Educators and Students:** Use Wireshark as a practical educational tool to learn about network protocol internals and observe theoretical networking concepts in action.

## System Description
Wireshark is a complex and highly modular open-source software project. It is primarily written in C and C++. The architecture separates concerns into distinct layers: a packet capture layer leveraging `libpcap`/`npcap`, a robust packet dissection engine known as `epan` (Ethereal Packet Analyzer), a capture file reading and writing library called `wiretap`, and various user interface modules (`ui`). The system is highly extensible, allowing developers to add custom protocol dissectors via a plugin architecture.

## Basic Code Statistics
Based on an analysis of the repository's current state, here are the fundamental metrics of the Wireshark codebase:

*   **Total Files:** 7,411 files
*   **Lines of Code (LOC):** ~7,929,660 lines of code
*   **Modules / Packages:** ~25 main top-level modules (Key directories include `epan`, `wiretap`, `ui`, `plugins`, `wsutil`, `capture`, `extcap`, etc.)
*   **Developers / Contributors:** 1,775 unique contributors in the project's Git history.
