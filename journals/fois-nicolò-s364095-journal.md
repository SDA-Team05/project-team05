# Activity Journal - Nicolò Fois (s364095)

## May 3rd
Activity done since last commit:
- Analyzed external libraries (npcap/libpcap)
- Investigated context diagram stakeholders
- Contributed to information gathering for the definition of the Context Diagram (L1)

## May 4th
Activity done since last commit:
- Conducted an in-depth architectural analysis of libpcap/npcap
- Started the Reverse Engineering of dumpcap as a Level 2 Container, specifically mapping the Inter-Process Communication (IPC) via pipes and the asynchronous data flow between libpcap, dumpcap, and the Wireshark GUI
- Initiated the formal Software Architecture Report, documenting the Context Level (L1) and the dumpcap interaction with the system (L2)

## May 14th
Activity done since last commit:
- Conducted a detailed analysis of the Capture and Wiretap modules
- Investigated the internal data flow and interfacing mechanisms between these modules, the Core, and the File System (Hard Disk)

## May 17th
Activity done since last commit:
- Refined the Context Diagram (L1) by treating Wireshark as a pure black box, properly abstracting internal deployable components like Dumpcap
- Drafted the Container Diagram (L2) using PlantUML and the formal C4 Model to clarify the separation of privileges and the roles of the components
- Integrated both automatic and detailed custom legends into the PlantUML scripts to ensure clear architectural documentation