#pragma once

#include <moose/types.h>

// clang-format off

enum amd64_cpuid {
    /* EAX=1, ECX */ 
    CPUID_SSE3 = 0u,      // Streaming SIMD Extensions 3
    CPUID_PCLMULQDQ = 1u, // PCLMULDQ Instruction
    CPUID_DTES64 = 2u,    // 64-Bit Debug Store
    CPUID_MONITOR = 3u,   // MONITOR/MWAIT Instructions
    CPUID_DS_CPL = 4u,    // CPL Qualified Debug Store
    CPUID_VMX = 5u,       // Virtual Machine Extensions
    CPUID_SMX = 6u,       // Safer Mode Extensions
    CPUID_EST = 7u,       // Enhanced Intel SpeedStep® Technology
    CPUID_TM2 = 8u,       // Thermal Monitor 2
    CPUID_SSSE3 = 9u,     // Supplemental Streaming SIMD Extensions 3
    CPUID_CNXT_ID = 10u,  // L1 Context ID
    CPUID_SDBG = 11u,     // Silicon Debug (IA32_DEBUG_INTERFACE MSR)
    CPUID_FMA = 12u,      // Fused Multiply Add
    CPUID_CX16 = 13u,     // CMPXCHG16B Instruction
    CPUID_XTPR = 14u,     // xTPR Update Control
    CPUID_PDCM = 15u,     // Perfmon and Debug Capability (IA32_PERF_CAPABILITIES MSR)
    /* ECX Bit 16 */          // Reserved
    CPUID_PCID = 17u,         // Process Context Identifiers
    CPUID_DCA = 18u,          // Direct Cache Access
    CPUID_SSE4_1 = 19u,       // Streaming SIMD Extensions 4.1
    CPUID_SSE4_2 = 20u,       // Streaming SIMD Extensions 4.2
    CPUID_X2APIC = 21u,       // Extended xAPIC Support
    CPUID_MOVBE = 22u,        // MOVBE Instruction
    CPUID_POPCNT = 23u,       // POPCNT Instruction
    CPUID_TSC_DEADLINE = 24u, // Time Stamp Counter Deadline
    CPUID_AES = 25u,          // AES Instruction Extensions
    CPUID_XSAVE = 26u,        // XSAVE/XSTOR States
    CPUID_OSXSAVE = 27u,      // OS-Enabled Extended State Management
    CPUID_AVX = 28u,          // Advanced Vector Extensions
    CPUID_F16C = 29u,         // 16-bit floating-point conversion instructions
    CPUID_RDRAND = 30u,       // RDRAND Instruction
    CPUID_HYPERVISOR = 31u,   // Hypervisor present (always zero on physical CPUs)
    /* EAX=1, EDX */        
    CPUID_FPU = 32u,        // Floating-point Unit On-Chip
    CPUID_VME = 33u,        // Virtual Mode Extension
    CPUID_DE = 34u,         // Debugging Extension
    CPUID_PSE = 35u,        // Page Size Extension
    CPUID_TSC = 36u,        // Time Stamp Counter
    CPUID_MSR = 37u,        // Model Specific Registers
    CPUID_PAE = 38u,        // Physical Address Extension
    CPUID_MCE = 39u,        // Machine-Check Exception
    CPUID_CX8 = 40u,        // CMPXCHG8 Instruction
    CPUID_APIC = 41u,       // On-chip APIC Hardware
    /* EDX Bit 10 */        // Reserved
    CPUID_SEP = 43u,        // Fast System Call
    CPUID_MTRR = 44u,       // Memory Type Range Registers
    CPUID_PGE = 45u,        // Page Global Enable
    CPUID_MCA = 46u,        // Machine-Check Architecture
    CPUID_CMOV = 47u,       // Conditional Move Instruction
    CPUID_PAT = 48u,        // Page Attribute Table
    CPUID_PSE36 = 49u,      // 36-bit Page Size Extension
    CPUID_PSN = 50u,        // Processor serial number is present and enabled
    CPUID_CLFLUSH = 51u,    // CLFLUSH Instruction
    /* EDX Bit 20 */        // Reserved
    CPUID_DS = 53u,         // CLFLUSH Instruction
    CPUID_ACPI = 54u,       // CLFLUSH Instruction
    CPUID_MMX = 55u,        // CLFLUSH Instruction
    CPUID_FXSR = 56u,       // CLFLUSH Instruction
    CPUID_SSE = 57u,        // Streaming SIMD Extensions
    CPUID_SSE2 = 58u,       // Streaming SIMD Extensions 2
    CPUID_SS = 59u,         // Self-Snoop
    CPUID_HTT = 60u,        // Multi-Threading
    CPUID_TM = 61u,         // Thermal Monitor
    CPUID_IA64 = 62u,       // IA64 processor emulating x86
    CPUID_PBE = 63u,        // Pending Break Enable
    /* EAX=7, EBX */        //
    CPUID_FSGSBASE = 64u,   // Access to base of %fs and %gs
    CPUID_TSC_ADJUST = 65u, // IA32_TSC_ADJUST MSR
    CPUID_SGX = 66u,        // Software Guard Extensions
    CPUID_BMI1 = 67u,       // Bit Manipulation Instruction Set 1
    CPUID_HLE = 68u,        // TSX Hardware Lock Elision
    CPUID_AVX2 = 69u,       // Advanced Vector Extensions 2
    CPUID_FDP_EXCPTN_ONLY = 70u, // FDP_EXCPTN_ONLY
    CPUID_SMEP = 71u,            // Supervisor Mode Execution Protection
    CPUID_BMI2 = 72u,            // Bit Manipulation Instruction Set 2
    CPUID_ERMS = 73u,            // Enhanced REP MOVSB/STOSB
    CPUID_INVPCID = 74u,         // INVPCID Instruction
    CPUID_RTM = 75u,             // TSX Restricted Transactional Memory
    CPUID_PQM = 76u,             // Platform Quality of Service Monitoring
    CPUID_ZERO_FCS_FDS = 77u,    // FPU CS and FPU DS deprecated
    CPUID_MPX = 78u,             // Intel MPX (Memory Protection Extensions)
    CPUID_PQE = 79u,             // Platform Quality of Service Enforcement
    CPUID_AVX512_F = 80u,        // AVX-512 Foundation
    CPUID_AVX512_DQ = 81u,       // AVX-512 Doubleword and Quadword Instructions
    CPUID_RDSEED = 82u,          // RDSEED Instruction
    CPUID_ADX = 83u,         // Intel ADX (Multi-Precision Add-Carry Instruction Extensions)
    CPUID_SMAP = 84u,        // Supervisor Mode Access Prevention
    CPUID_AVX512_IFMA = 85u, // AVX-512 Integer Fused Multiply-Add Instructions
    CPUID_PCOMMIT = 86u,     // PCOMMIT Instruction
    CPUID_CLFLUSHOPT = 87u,  // CLFLUSHOPT Instruction
    CPUID_CLWB = 88u,        // CLWB Instruction
    CPUID_INTEL_PT = 89u,    // Intel Processor Tracing
    CPUID_AVX512_PF = 90u,   // AVX-512 Prefetch Instructions
    CPUID_AVX512_ER = 91u,   // AVX-512 Exponential and Reciprocal Instructions
    CPUID_AVX512_CD = 92u,   // AVX-512 Conflict Detection Instructions
    CPUID_SHA = 93u,         // Intel SHA Extensions
    CPUID_AVX512_BW = 94u,   // AVX-512 Byte and Word Instructions
    CPUID_AVX512_VL = 95u,   // AVX-512 Vector Length Extensions
    /* EAX=7, ECX */         //
    CPUID_PREFETCHWT1 = 96u, // PREFETCHWT1 Instruction
    CPUID_AVX512_VBMI = 97u, // AVX-512 Vector Bit Manipulation Instructions
    CPUID_UMIP = 98u,        // UMIP
    CPUID_PKU = 99u,         // Memory Protection Keys for User-mode pages
    CPUID_OSPKE = 100u,      // PKU enabled by OS
    CPUID_WAITPKG = 101u,    // Timed pause and user-level monitor/wait
    CPUID_AVX512_VBMI2 = 102u, // AVX-512 Vector Bit Manipulation Instructions 2
    CPUID_CET_SS = 103u,       // Control Flow Enforcement (CET) Shadow Stack
    CPUID_GFNI = 104u,         // Galois Field Instructions
    CPUID_VAES = 105u,         // Vector AES instruction set (VEX-256/EVEX)
    CPUID_VPCLMULQDQ = 106u,   // CLMUL instruction set (VEX-256/EVEX)
    CPUID_AVX512_VNNI = 107u,  // AVX-512 Vector Neural Network Instructions
    CPUID_AVX512_BITALG = 108u, // AVX-512 BITALG Instructions
    CPUID_TME_EN = 109u,        // IA32_TME related MSRs are supported
    CPUID_AVX512_VPOPCNTDQ = 110u,     // AVX-512 Vector Population Count Double and Quad-word
    /* ECX Bit 15 */                   // Reserved
    CPUID_INTEL_5_LEVEL_PAGING = 112u, // Intel 5-Level Paging
    CPUID_RDPID = 113u,                // RDPID Instruction
    CPUID_KL = 114u,                   // Key Locker
    /* ECX Bit 24 */                   // Reserved
    CPUID_CLDEMOTE = 116u,             // Cache Line Demote
    /* ECX Bit 26 */                   // Reserved
    CPUID_MOVDIRI = 118u,              // MOVDIRI Instruction
    CPUID_MOVDIR64B = 119u,            // MOVDIR64B Instruction
    CPUID_ENQCMD = 120u,               // ENQCMD Instruction
    CPUID_SGX_LC = 121u,               // SGX Launch Configuration
    CPUID_PKS = 122u,                  // Protection Keys for Supervisor-Mode Pages
    /* EAX=7, EDX */                   //
    /* ECX Bit 0-1 */                  // Reserved
    CPUID_AVX512_4VNNIW = 125u,        // AVX-512 4-register Neural Network Instructions
    CPUID_AVX512_4FMAPS = 126u,        // AVX-512 4-register Multiply Accumulation Single precision
    CPUID_FSRM = 127u,                       // Fast Short REP MOVSB
    /* ECX Bit 5-7 */ // Reserved
    CPUID_AVX512_VP2INTERSECT = 131u,  // AVX-512 VP2INTERSECT Doubleword and Quadword Instructions
    CPUID_SRBDS_CTRL = 132u,           // Special Register Buffer Data Sampling Mitigations
    CPUID_MD_CLEAR = 133u,         // VERW instruction clears CPU buffers
    CPUID_RTM_ALWAYS_ABORT = 134u, // All TSX transactions are aborted
    /* ECX Bit 12 */               // Reserved
    CPUID_TSX_FORCE_ABORT = 136u,  // TSX_FORCE_ABORT MSR
    CPUID_SERIALIZE = 137u,        // Serialize instruction execution
    CPUID_HYBRID = 138u,           // Mixture of CPU types in processor topology
    CPUID_TSXLDTRK = 139u,         // TSX suspend load address tracking
    /* ECX Bit 17 */               // Reserved
    CPUID_PCONFIG = 141u,          // Platform configuration (Memory Encryption
                                   // Technologies Instructions)
    CPUID_LBR = 142u,              // Architectural Last Branch Records
    CPUID_CET_IBT = 143u,          // Control flow enforcement (CET) indirect branch tracking
    /* ECX Bit 21 */ // Reserved
    CPUID_AMX_BF16 = 145u,    // Tile computation on bfloat16 numbers
    CPUID_AVX512_FP16 = 146u, // AVX512-FP16 half-precision floating-point instructions
    CPUID_AMX_TILE = 147u,    // Tile architecture
    CPUID_AMX_INT8 = 148u,    // Tile computation on 8-bit integers
    CPUID_SPEC_CTRL = 149u,   // Speculation Control
    CPUID_STIBP = 150u,       // Single Thread Indirect Branch Predictor
    CPUID_L1D_FLUSH = 151u,   // IA32_FLUSH_CMD MSR
    CPUID_IA32_ARCH_CAPABILITIES = 152u, // IA32_ARCH_CAPABILITIES MSR
    CPUID_IA32_CORE_CAPABILITIES = 153u, // IA32_CORE_CAPABILITIES MSR
    CPUID_SSBD = 154u,                   // Speculative Store Bypass Disable
    /* EAX=80000001h, ECX */             //
    CPUID_LAHF_LM = 155u,                // LAHF/SAHF in long mode
    CPUID_CMP_LEGACY = 156u,             // Hyperthreading not valid
    CPUID_SVM = 157u,                    // Secure Virtual Machine
    CPUID_EXTAPIC = 158u,                // Extended APIC Space
    CPUID_CR8_LEGACY = 159u,             // CR8 in 32-bit mode
    CPUID_ABM = 160u,                    // Advanced Bit Manipulation
    CPUID_SSE4A = 161u,                  // SSE4a
    CPUID_MISALIGNSSE = 162u,            // Misaligned SSE Mode
    CPUID__3DNOWPREFETCH = 163u,         // PREFETCH and PREFETCHW Instructions
    CPUID_OSVW = 164u,                   // OS Visible Workaround
    CPUID_IBS = 165u,                    // Instruction Based Sampling
    CPUID_XOP = 166u,                    // XOP instruction set
    CPUID_SKINIT = 167u,                 // SKINIT/STGI Instructions
    CPUID_WDT = 168u,                    // Watchdog timer
    CPUID_LWP = 169u,                    // Light Weight Profiling
    CPUID_FMA4 = 170u,                   // FMA4 instruction set
    CPUID_TCE = 171u,                    // Translation Cache Extension
    CPUID_NODEID_MSR = 172u,             // NodeID MSR
    CPUID_TBM = 173u,                    // Trailing Bit Manipulation
    CPUID_TOPOEXT = 174u,                // Topology Extensions
    CPUID_PERFCTR_CORE = 175u,           // Core Performance Counter Extensions
    CPUID_PERFCTR_NB = 176u,             // NB Performance Counter Extensions
    CPUID_DBX = 177u,                    // Data Breakpoint Extensions
    CPUID_PERFTSC = 178u,                // Performance TSC
    CPUID_PCX_L2I = 179u,                // L2I Performance Counter Extensions
    /* EAX=80000001h, EDX */             //
    CPUID_SYSCALL = 180u,                // SYSCALL/SYSRET Instructions
    CPUID_MP = 181u,                     // Multiprocessor Capable
    CPUID_NX = 182u,                     // NX bit
    CPUID_MMXEXT = 183u,                 // Extended MMX
    CPUID_FXSR_OPT = 184u,               // FXSAVE/FXRSTOR Optimizations
    CPUID_PDPE1GB = 185u,                // Gigabyte Pages
    CPUID_RDTSCP = 186u,                 // RDTSCP Instruction
    CPUID_LM = 187u,                     // Long Mode
    CPUID__3DNOWEXT = 188u,              // Extended 3DNow!
    CPUID__3DNOW = 189u,                 // 3DNow!
    /* EAX=80000007h, EDX */             //
    CPUID_CONSTANT_TSC = 190u,           // Invariant TSC
    CPUID_NONSTOP_TSC = 191u,            // Invariant TSC
    CPUID___End = 255u
};

void init_cpuid(void);
int cpu_supports(enum amd64_cpuid feature);

// clang-formatn
