# Activity Journal - Lorenzo Iantosca (s344969)

## May 12th
- Cloned the Wireshark Git repository and initiated environment setup for architectural analysis;
- Configured the build system using CMake and Ninja, resolving dependencies for Qt6 and the Strawberry Perl environment;
- Managed toolchain troubleshooting related to architecture mismatch (x86 vs x64), eventually adopting the x64 Native Tools Command Prompt for Visual Studio 2022;
- Generated the `compile_commands.json` database and performed data cleaning via Regular Expressions to strip MSVC-specific flags (`/Fo`, `/Fd`) incompatible with the Clang-based parser of SourceTrail;
- Successfully completed the full indexing of the Wireshark codebase (over 3800 files) within the SourceTrail static analysis suite;
- Executed Shell scripts via Git Bash to extract quantitative metrics for High Fan-in (Top Files) and High Fan-out (Bottom Files) analysis;
- Mapped import dependencies for core orchestrators (`tshark.c`, `wireshark_main_window_slots.cpp`) and architectural pillars (`packet.h`), identifying key structural patterns and layered architecture violations.