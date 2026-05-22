# Software Report
## Code Dependencies

In order to understand the structural coupling of the Wireshark source code, a static analysis of the file and modular dependencies was conducted using the "Understand" and "Sourcetrail" tools. 

### Import Map and Subsystem Dependencies
An Architecture Dependency Graph was generated to evaluate the interactions between the three main layers of the application: 
- `UI` (Presentation)
- `EPAN` (Core dissection engine)
- `WSUTIL` (Base utilities)

The quantitative analysis partially confirms a top-down layered architecture, but also highlights some architectural violations:
* **Expected Top-Down Flow:** the system exhibits a strong reliance on the foundational layers. 

        `EPAN` heavily depends on `WSUTIL` (869,607 dependencies), confirming `WSUTIL`'s role as the fundamental utility module. 
        Furthermore, the `UI` properly relies on `EPAN` (8,437 dependencies) to trigger dissections and retrieve packet data.

* **Architectural Violations (Design smells):** a strict layered architecture dictates that lower layers must not depend on upper layers. 
However, the dependency graph revealed bidirectional coupling: 

        `EPAN` has 61 outgoing dependencies directed towards the `UI` layer
        `WSUTIL` has 2 dependencies pointing back to `EPAN`.
        While numerically small compared to the main flow, these represent architectural leaks where the core engine and base utilities 
        are unnecessarily coupled with higher-level abstractions.

<p align="center">
  <img src="./dependency%20graphs/subsystem_butterfly_coupling.png" alt="Understand Dependency Graph showing bidirectional coupling" style="border-radius: 8px; box-shadow: 0 4px 8px rgba(0,0,0,0.15); margin-bottom: 10px;">
  <br>
  <em>Figure: Dependency graph generated via Understand, illustrating the massive expected top-down flow alongside the minor architectural violations (bidirectional red arrows).</em>
</p>

### Top and Bottom Files by Dependency

A static analysis of `#include` directives across the codebase was performed to identify the structural pillars (High Fan-in) and the main orchestrators (High Fan-out) of the system.

#### Bottom Files (High Fan-out)
These files represent the modules with the highest number of outgoing dependencies. They import a massive amount of header files to function.

| Rank | File Path | Outgoing Dependencies | Layer / Subsystem |
| :--- | :--- | :--- | :--- |
| 1 | `./ui/qt/wireshark_main_window_slots.cpp` | 136 | UI (Presentation) |
| 2 | `./ui/stratoshark/stratoshark_main_window_slots.cpp` | 98 | UI (Presentation) |
| 3 | `./wiretap/file_access.c` | 90 | Wiretap (Capture) |
| 4 | `./ui/qt/lua_debugger/lua_debugger_dialog.cpp` | 88 | UI (Presentation) |
| 5 | `./tshark.c` | 83 | CLI Entry Point |
| 6 | `./strato.c` | 77 | CLI Entry Point |
| 7 | `./ui/qt/main.cpp` | 74 | UI (Presentation) |
| 8 | `./ui/stratoshark/stratoshark_main.cpp` | 69 | UI (Presentation) |
| 9 | `./ui/qt/lua_debugger/lua_debugger_breakpoints.cpp` | 68 | UI (Presentation) |
| 10 | `./ui/qt/wireshark_main_window.cpp` | 66 | UI (Presentation) |
| 11 | `./ui/qt/packet_list.cpp` | 63 | UI (Presentation) |
| 12 | `./ui/qt/main_application.cpp` | 62 | UI (Presentation) |
| 13 | `./epan/epan.c` | 61 | EPAN (Core) |
| 14 | `./ui/stratoshark/stratoshark_main_window.cpp` | 58 | UI (Presentation) |
| 15 | `./sharkd_session.c` | 55 | Daemon Entry Point |

**Architectural Motivation:** The data strongly reflects the layered architecture. The files dominating this list are primarily entry points (`tshark.c`, `strato.c`) and UI orchestrators (`wireshark_main_window_slots.cpp`). Sitting at the very top of the execution pyramid, these files act as "god modules". To correctly render the GUI and coordinate the application, they must interact with the capture engine (`wiretap`), the dissection core (`epan`), configuration files, and numerous Qt graphical widgets. This structural requirement forces them to have an extreme Fan-out.


<p align="center">
  <img src="./dependency%20graphs/tshark_dependency_graph.png" alt="Dependency Graph of tshark.c showing high fan-out" width="60%" style="border-radius: 8px; box-shadow: 0 4px 8px rgba(0,0,0,0.15); margin-bottom: 10px;">
  <br>
  <em>Figure: SourceTrail dependency graph of <code>tshark.c</code>, illustrating a massive amount of outgoing dependencies typical of an orchestrator file.</em>
</p>

<p align="center">
  <img src="./dependency%20graphs/main_window_dependency_graph.png" alt="Dependency Graph of wireshark_main_window_slots showing high fan-out" width="50%" style="border-radius: 8px; box-shadow: 0 4px 8px rgba(0,0,0,0.15); margin-bottom: 10px;">
  <br>
  <em>Figure: SourceTrail dependency graph of <code>wireshark_main_window_slots.cpp</code>.</em>
</p>


#### Top Files (High Fan-in)
These files represent the most imported headers in the entire project. They are the foundational building blocks upon which the rest of the application relies.

| Rank | Header File | Incoming Dependencies | Layer / Subsystem |
| :--- | :--- | :--- | :--- |
| 1 | `"config.h"` | 2373 | Global Configuration |
| 2 | `<epan/packet.h>` | 2022 | EPAN (Core) |
| 3 | `<epan/expert.h>` | 969 | EPAN (Core) |
| 4 | `<wsutil/array.h>` | 757 | WSUTIL (Utilities) |
| 5 | `<epan/prefs.h>` | 744 | EPAN (Core) |
| 6 | `<epan/tfs.h>` | 617 | EPAN (Core) |
| 7 | `<string.h>` | 354 | C Standard Library |
| 8 | `<epan/asn1.h>` | 353 | EPAN (Core) |
| 9 | `<epan/proto_data.h>` | 328 | EPAN (Core) |
| 10 | `<epan/conversation.h>` | 312 | EPAN (Core) |
| 11 | `<glib.h>` | 285 | External Utility (GLib) |
| 12 | `<epan/to_str.h>` | 253 | EPAN (Core) |
| 13 | `<stdlib.h>` | 251 | C Standard Library |
| 14 | `<config.h>` | 251 | Global Configuration |
| 15 | `"packet-tcp.h"` | 247 | Dissector Header |

**Architectural Motivation:**
The Fan-in ranking highlights the rigid modularity of Wireshark's dissection engine. At the very top, aside from global configurations (`config.h`), we find `epan/packet.h`. This file contains the fundamental C structures defining what a "network packet" is. Because Wireshark relies on thousands of independent dissector modules to decode different network protocols, every single one of them is forced to include this core definition. 
These files are the foundational pillars of the software: they are highly stable, as a single modification to `epan/packet.h` would invalidate the compilation cache of over 2000 files, forcing a near-total recompilation of the project.


<p align="center">
  <img src="./dependency%20graphs/packet_dependency_graph.png" alt="Dependency Graph of packet.h showing high fan-in" width="30%" style="border-radius: 8px; box-shadow: 0 4px 8px rgba(0,0,0,0.15); margin-bottom: 10px;">
  <br>
  <em>Figure: SourceTrail dependency graph of <code>packet.h</code> showing massive high fan-in.</em>
</p>

### Logical Coupling and Architectural Anomalies
In addition to static structural dependencies, an evolutionary analysis of the system was conducted by mining the Git version control history. The goal was to identify **knowledge dependencies** via **change coupling** (modules and files that frequently change together in the same commits despite potentially lacking an explicit structural dependency). 

To ensure the validity and mathematical soundness of the findings, a **cross-validation approach** was adopted by cross-checking two independent methodologies:
1. An automated behavioral and evolution analysis platform (**CodeScene**).
2. A custom targeted Python script (`cochange_analyzer.py`) analyzing the raw transaction history.

Both tools successfully identified severe coupling, though their different analytical heuristics (raw mathematical frequency vs. time-decayed behavioral analysis) highlighted distinct architectural hotspots:

**1. Cross-layer knowledge leak (identified via Python script):**
The custom script revealed a high-risk `shotgun surgery` smell across system boundaries. Nearly 90% of the time a core architectural preference is altered (`epan/prefs.c`), developers are forced to synchronously update the layout implementation in the Qt presentation layer (`ui/qt/layout_preferences_frame.h`). This uncovers an implicit knowledge leak, as the core engine should ideally remain agnostic of GUI configurations.


| File A (Core / Architecture module) | File B (Presentation / Context layer) | Co-change % (Python) | Risk category   |
| :------------------------------     | :------------------------------       | :-----------------   | :-------------- |
| `epan/prefs.c`                      | `ui/qt/layout_preferences_frame.h`    | 85.71%               | shotgun surgery |
| `epan/prefs.h`                      | `ui/qt/layout_preferences_frame.cpp`  | 72.73%               | shotgun surgery |

**2. High-density module coupling (identified via CodeScene):**
Conversely, CodeScene behavioral engine prioritized modules with a **100% Degree of coupling** and high revision rates. 
As shown in the extracted data, CodeScene highlights an extreme internal dependency within the UI subsystem. 

| Entity (File A)                 | Coupled entity (File B)         | Degree of coupling | Average revisions |
| :------------------------------ | :------------------------------ | :----------------- | :---------------- |
| `ui/qt/packet_list.cpp`         | `ui/qt/packet_list_record.cpp`  | 100%               | 6                 |
| `epan/dissectors/packet-wlan.c` | `epan/dissectors/packet-wlan.h` | 100%               | 8                 |
| `ui/qt/packet_list_model.cpp`   | `ui/qt/packet_list_record.cpp`  | 100%               | 5                 |

This behavioral data proves that changes cascade rapidly through `packet_list` components due to tight logical binding. This architectural bottleneck forces continuous synchronization across UI files, driving up maintenance costs.

**Architectural validation & Evolution motivations:**
The empirical convergence between manual log parsing and CodeScene proprietary heuristics confirms a severe `shotgun surgery` design smell. 
Nearly 90% of the time a core architectural structure or preference definition is altered (`epan/prefs.c`), developers are forced to synchronously update the layout implementation in the `Qt` presentation layer.
This behavioral coupling uncovers an implicit knowledge leak across system boundaries. 
Ideally, a layered software design implies that core engine subcomponents should remain entirely agnostic of presentation details and layout configurations. 
By analyzing CodeScene Change Coupling view, it becomes evident that changes cascade rapidly through these modules due to the lack of an intermediate abstraction layer or data-driven binding mechanism. 
This architectural bottleneck forces continuous synchronization across different layers, driving up maintenance costs and undermining the long-term evolvability of the Wireshark platform.

### Cross-validation of Static and Evolutionary metrics: The `tshark.c` case
To further bridge the gap between static structural dependencies and evolutionary technical debt, CodeScene REST APIs were utilized to programmatically extract the project's top "hotspots" (`codescene_hotspots.json`). 

This extraction yielded a crucial architectural validation: `tshark.c`, previously identified in the static analysis as a "god module" with extreme Fan-out (83 outgoing dependencies), is mathematically ranked as one of the top 10 worst hotspots in the entire Wireshark codebase. 

According to the API data, `tshark.c` exhibits:
* **Code health score:** 1.68 / 10.0 (Critical Red Zone)
* **Recent revisions:** 87

**Architectural motivation:** 
This provides empirical proof of how structural design choices directly impact software evolvability. 
The orchestration responsibilities centralized within `tshark.c` force it to be touched continuously during development (87 revisions).
Because of its massive Fan-out, every modification is highly complex and error-prone, plummeting its `code health` to 1.68. 
This perfectly demonstrates how static structural smells (the god module anti-pattern) directly manifest as severe evolutionary technical debt over time.

<p align="center">
  <img src="./dependency%20graphs/codescene_hotspots.png" alt="CodeScene Hotspot Map" width="50%" style="border-radius: 8px; box-shadow: 0 4px 8px rgba(0,0,0,0.15); margin-bottom: 10px;">
  <br>
  <em>Figure: CodeScene hotspot virtual map displaying code health degradation driven by high change frequency and overlapping logical dependencies.</em>
</p>


## Patterns
In this section is present a list of detected patterns in Wireshark.

### Chain of Responsibility
This pattern gives the possibility to send a request along a chain of potential handlers until one of them handles it. The general idea is to decouple senders and receivers. 
If *Chain of Responsibility* was not used in Wireshark where thousands of protocols have to be supported, the code would look like a huge conditional block which would be difficult to manage.

The discussed pattern is used in the `epan` folder of the project to implement the dissectors chain: every dissector analyzes its packet level and pass the payload to the next dissector through dispatch tables without knowing who will handle it. The classic pattern has been adapted from an OOP style to a C architecture based on modules and table.

The **Handler** role is developed by the struct `dissector_handle_t` defined in `epan/packet.c`and by the type function `dissector_t` declared in `epan/packet.h`. The successor link isn’t in the Handler but in the `dissector_table_t`. Every `epan/dissectors/packet-*.c` is a **Concrete Handler** which manages the packet or passes it to the successor calling it by `dissector_try_uint()` or `call_dissector()`. Some examples are `dissect_frame()` from `packet-frame.c`, `dissect_ip()` from `packet-ip.c` and `dissect_tcp()` from `packet-tcp.c`. The **Client**, which sends the first request is split into two different moments in Wireshark: chain building with every `proto_reg_handoff_*()` call in `packet-*.c`files and request send with `epan_dissect_run()` in `epan/epan.c` which hands over the execution to the packet processing core in `epan/packet.c`, which bootstraps the entire chain by calling `call_dissector()` on the initial `frame_handle`.

Instead of *Chain of Responsibility*, other solutions could be used:
-	*centralized switch/if-else*, as written above, simple to use only with few protocols;
-	 *Strategy pattern*, simpler conceptually but not capable to manage hierarchical style of protocols;
-	*Observer pattern*, guarantees total decouple but not the processing order adding complexity to debug.
  
Analyzing pros and cons, the actual pattern stays the better choice because respects hierarchical and sequential nature of network protocols.

### Strategy 
This pattern defines a family of algorihms, putting each of them into a separate class, and making them interchangeable. This pattern is particularly useful when you have multiple algorithms for a specific task and want to be able to switch between them dynamically. 

The discussed pattern is present in the `wiretap` subsystem which reads and writes capture files in different formats. Since the project uses the C language, the pattern is simulated with structs and function pointers.

The **Strategy** role is fulfilled by the struct `file_type_subtype_info` defined in `wiretap/wtap.h`, which defines the functions protoypes that each file format must implement to interact with Wireshark. Each file designed for a specific format such as `wiretap/json.c`, `wiretap/snoop.c` or `wiretap/pcapng.c` represents a **Concrete Strategy** since they contain static constant variables of the struct `file_type_subtype_info` (for example in `wiretap/json.c` is present `json_info`). The **Context** role is principally managed by `wiretap/file_access.c` and encapsulated at runtime into Wireshark’s session structures. This file allows the storage of tables or arrays of all strategies then, when a users open a file, Wireshark searches and identifies the right Concrete Strategy saving the structure pointer into the current session.

Instead of the *Strategy* pattern, other solutions could be used:
-	*monolithic switch or if/else chain*: it eliminates the overhead of function pointer calls but violates of the Open/Closed Principle;
-	*Chain of Responsibility pattern*: the file to be opened is passed through a chain of modules until it is recognized but the centralized control given by the Context is lost.

Analyzing the different options, the chosen pattern remains the better choice because it enforces strict modularity and offers a centralized orchestration.

### Abstract Factory
The Abstract Factory pattern provides an interface for creating families of related or dependent objects, so that clients don't need to specify the names of concrete classes in their code. In Wireshark, this pattern is essential for managing the initialization of thousands of different protocols while keeping the core engine decoupled from specific protocol implementations. Otherwise the core would need a manual reference to every single dissector, making the architecture rigid and impossible to extend with plugins.

The *Abstract Factory* role is implemented through the functional interface defined in epan/proto.h, specifically by the function proto_register_protocol() in epan/proto.c which defines "the contract" that every protocol has to follow in order to be integrated with the system. The *Concrete Factory* role is played by each individual dissector file in the epan/dissectors/ folder; each file contains a registration function that "produces" its protocol definition and hands it over to the core. The *Product* is the protocol_t structure (and the associated proto_id), which represents the registered protocol within the engine.
If this function didn't exist, the alternative would be a massive central file containing an endless (hardcoded) list of all protocols. This would mean that every time someone invented a new protocol, the main Wireshark file would have to be modified, making maintenance extremely complex and preventing anyone from creating private extensions.


Rather than Abstract Factory other solutions could be:
- *Builder Pattern*: it would allow for a step-by-step configuration of complex protocols. However, it would add significant verbosity, repetitive code and memory overhead for managing temporary configuration objects, making the startup process less efficient than the current single function call.
- *Prototype Pattern*: instead of using a “factory” function that builds the protocol from scratch each time, we could have a pre-configured protocol “prototype.” Each dissector would not be created via parameters, but would be cloned from a base protocol_t object (“prototype”) and then customized only in the necessary fields. This would simplify the creation of objects that are very similar to one another, reducing the need for a complex factory hierarchy. However, in the C language, the deep copy operation is complex and risky for memory. It would require very meticulous manual management of pointers to prevent two protocols from accidentally sharing the same memory area.
