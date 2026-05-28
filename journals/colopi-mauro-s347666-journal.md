# Activity Journal - Mauro Colopi (s347666)
## May 3rd
Activity done since last commit:
- Initial study of wireshark: purpose, intended use cases and functionalities
- Documentation dependency analysis: identifying the dependencies between the different components of the project (npcap, libpcap, etc.)
- Contribute to information gathering for C4 diagram (L1)

## May 10th
Activity done since last commit:
- Analysis of the solid principles within Wireshark's architecture

## May 15th
- Documented the strict architectural and privilege separation between dumpcap (solely responsible for raw packet capture) and epan (strictly dedicated to dissection and analysis).

## May 20th
- Conducted an in-depth architectural analysis of epan
- Started the Reverse Engineering of epan as a Level 2 Container, mapping the relationship between dumpcap, libwiretap and the Wireshark GUI
- Initiated the formal Software Architecture Report, documenting the epan interaction with the system (L2)

## May 28th
- Analysis of the external libraries c-ares, libgcrypt, libwsutil in order to understand how they work and whether or not to integrate them into the C4 (L3) diagram.
- Mapped the internal components of the epan container for the Level 3 Component Diagram, specifically isolating the Dissector Engine, the Display Filter Engine (dfilter), and the Memory Management framework (wmem).
- Investigated the dynamic loading mechanism for external plugins (e.g., Lua scripts via wslua and compiled C dissectors) to accurately represent EPAN's runtime dependencies on the Local File System.

