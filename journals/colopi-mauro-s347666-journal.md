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

## May 29th
- Designed the Level 3 Component Diagram (C4 model) for the Dumpcap executable, mapping its internal C modules and boundaries.
- Analyzed and documented the "Privilege Separation" security pattern, detailing the interactions between the Syncpipe Controller, Privilege Manager, and Capture Engine.
- Formalized architectural considerations regarding the integration of external OS network libraries (libpcap/Npcap) and asynchronous File System I/O relative to the privileged container.

## May 3th 
- Gathered structural metrics of the Wireshark repository (total files, lines of code, and top-level modules) by cloning the repository and running analysis tools like cloc.
- Extracted the number of unique contributors and active developers from the project's history using git shortlog.
- Drafted the initial section of the overview.md report, defining the primary purpose of the system and identifying its main stakeholders (Network Administrators, Security Professionals, Software Developers, and Educators).

## May 4th
- Finalized the content of the Overview report, ensuring the text remained concise and strictly within the 1000-word
- Reviewed and formatted the Overview markdown file.