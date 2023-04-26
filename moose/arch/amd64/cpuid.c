#include <moose/arch/amd64/asm.h>
#include <moose/arch/amd64/cpuid.h>
#include <moose/assert.h>
#include <moose/bitops.h>

static bitmap_t cpuid_bitmap[BITS_TO_BITMAP(CPUID___End)];

void init_cpuid(void) {
    struct cpuid processor_info;
    cpuid(0x1, 0, &processor_info);

    if (processor_info.ecx & (1 << 0))
        set_bit(CPUID_SSE3, cpuid_bitmap);
    if (processor_info.ecx & (1 << 1))
        set_bit(CPUID_PCLMULQDQ, cpuid_bitmap);
    if (processor_info.ecx & (1 << 2))
        set_bit(CPUID_DTES64, cpuid_bitmap);
    if (processor_info.ecx & (1 << 3))
        set_bit(CPUID_MONITOR, cpuid_bitmap);
    if (processor_info.ecx & (1 << 4))
        set_bit(CPUID_DS_CPL, cpuid_bitmap);
    if (processor_info.ecx & (1 << 5))
        set_bit(CPUID_VMX, cpuid_bitmap);
    if (processor_info.ecx & (1 << 6))
        set_bit(CPUID_SMX, cpuid_bitmap);
    if (processor_info.ecx & (1 << 7))
        set_bit(CPUID_EST, cpuid_bitmap);
    if (processor_info.ecx & (1 << 8))
        set_bit(CPUID_TM2, cpuid_bitmap);
    if (processor_info.ecx & (1 << 9))
        set_bit(CPUID_SSSE3, cpuid_bitmap);
    if (processor_info.ecx & (1 << 10))
        set_bit(CPUID_CNXT_ID, cpuid_bitmap);
    if (processor_info.ecx & (1 << 11))
        set_bit(CPUID_SDBG, cpuid_bitmap);
    if (processor_info.ecx & (1 << 12))
        set_bit(CPUID_FMA, cpuid_bitmap);
    if (processor_info.ecx & (1 << 13))
        set_bit(CPUID_CX16, cpuid_bitmap);
    if (processor_info.ecx & (1 << 14))
        set_bit(CPUID_XTPR, cpuid_bitmap);
    if (processor_info.ecx & (1 << 15))
        set_bit(CPUID_PDCM, cpuid_bitmap);
    if (processor_info.ecx & (1 << 17))
        set_bit(CPUID_PCID, cpuid_bitmap);
    if (processor_info.ecx & (1 << 18))
        set_bit(CPUID_DCA, cpuid_bitmap);
    if (processor_info.ecx & (1 << 19))
        set_bit(CPUID_SSE4_1, cpuid_bitmap);
    if (processor_info.ecx & (1 << 20))
        set_bit(CPUID_SSE4_2, cpuid_bitmap);
    if (processor_info.ecx & (1 << 21))
        set_bit(CPUID_X2APIC, cpuid_bitmap);
    if (processor_info.ecx & (1 << 22))
        set_bit(CPUID_MOVBE, cpuid_bitmap);
    if (processor_info.ecx & (1 << 23))
        set_bit(CPUID_POPCNT, cpuid_bitmap);
    if (processor_info.ecx & (1 << 24))
        set_bit(CPUID_TSC_DEADLINE, cpuid_bitmap);
    if (processor_info.ecx & (1 << 25))
        set_bit(CPUID_AES, cpuid_bitmap);
    if (processor_info.ecx & (1 << 26))
        set_bit(CPUID_XSAVE, cpuid_bitmap);
    if (processor_info.ecx & (1 << 27))
        set_bit(CPUID_OSXSAVE, cpuid_bitmap);
    if (processor_info.ecx & (1 << 28))
        set_bit(CPUID_AVX, cpuid_bitmap);
    if (processor_info.ecx & (1 << 29))
        set_bit(CPUID_F16C, cpuid_bitmap);
    if (processor_info.ecx & (1 << 30))
        set_bit(CPUID_RDRAND, cpuid_bitmap);
    if (processor_info.ecx & (1 << 31))
        set_bit(CPUID_HYPERVISOR, cpuid_bitmap);

    if (processor_info.edx & (1 << 0))
        set_bit(CPUID_FPU, cpuid_bitmap);
    if (processor_info.edx & (1 << 1))
        set_bit(CPUID_VME, cpuid_bitmap);
    if (processor_info.edx & (1 << 2))
        set_bit(CPUID_DE, cpuid_bitmap);
    if (processor_info.edx & (1 << 3))
        set_bit(CPUID_PSE, cpuid_bitmap);
    if (processor_info.edx & (1 << 4))
        set_bit(CPUID_TSC, cpuid_bitmap);
    if (processor_info.edx & (1 << 5))
        set_bit(CPUID_MSR, cpuid_bitmap);
    if (processor_info.edx & (1 << 6))
        set_bit(CPUID_PAE, cpuid_bitmap);
    if (processor_info.edx & (1 << 7))
        set_bit(CPUID_MCE, cpuid_bitmap);
    if (processor_info.edx & (1 << 8))
        set_bit(CPUID_CX8, cpuid_bitmap);
    if (processor_info.edx & (1 << 9))
        set_bit(CPUID_APIC, cpuid_bitmap);
#if 0
    if (processor_info.edx & (1 << 11))
        handle_edx_bit_11_feature();
#endif
    if (processor_info.edx & (1 << 12))
        set_bit(CPUID_MTRR, cpuid_bitmap);
    if (processor_info.edx & (1 << 13))
        set_bit(CPUID_PGE, cpuid_bitmap);
    if (processor_info.edx & (1 << 14))
        set_bit(CPUID_MCA, cpuid_bitmap);
    if (processor_info.edx & (1 << 15))
        set_bit(CPUID_CMOV, cpuid_bitmap);
    if (processor_info.edx & (1 << 16))
        set_bit(CPUID_PAT, cpuid_bitmap);
    if (processor_info.edx & (1 << 17))
        set_bit(CPUID_PSE36, cpuid_bitmap);
    if (processor_info.edx & (1 << 18))
        set_bit(CPUID_PSN, cpuid_bitmap);
    if (processor_info.edx & (1 << 19))
        set_bit(CPUID_CLFLUSH, cpuid_bitmap);
    if (processor_info.edx & (1 << 21))
        set_bit(CPUID_DS, cpuid_bitmap);
    if (processor_info.edx & (1 << 22))
        set_bit(CPUID_ACPI, cpuid_bitmap);
    if (processor_info.edx & (1 << 23))
        set_bit(CPUID_MMX, cpuid_bitmap);
    if (processor_info.edx & (1 << 24))
        set_bit(CPUID_FXSR, cpuid_bitmap);
    if (processor_info.edx & (1 << 25))
        set_bit(CPUID_SSE, cpuid_bitmap);
    if (processor_info.edx & (1 << 26))
        set_bit(CPUID_SSE2, cpuid_bitmap);
    if (processor_info.edx & (1 << 27))
        set_bit(CPUID_SS, cpuid_bitmap);
    if (processor_info.edx & (1 << 28))
        set_bit(CPUID_HTT, cpuid_bitmap);
    if (processor_info.edx & (1 << 29))
        set_bit(CPUID_TM, cpuid_bitmap);
    if (processor_info.edx & (1 << 30))
        set_bit(CPUID_IA64, cpuid_bitmap);
    if (processor_info.edx & (1 << 31))
        set_bit(CPUID_PBE, cpuid_bitmap);

    struct cpuid extended_features;
    cpuid(0x7, 0, &extended_features);

    if (extended_features.ebx & (1 << 0))
        set_bit(CPUID_FSGSBASE, cpuid_bitmap);
    if (extended_features.ebx & (1 << 1))
        set_bit(CPUID_TSC_ADJUST, cpuid_bitmap);
    if (extended_features.ebx & (1 << 2))
        set_bit(CPUID_SGX, cpuid_bitmap);
    if (extended_features.ebx & (1 << 3))
        set_bit(CPUID_BMI1, cpuid_bitmap);
    if (extended_features.ebx & (1 << 4))
        set_bit(CPUID_HLE, cpuid_bitmap);
    if (extended_features.ebx & (1 << 5))
        set_bit(CPUID_AVX2, cpuid_bitmap);
    if (extended_features.ebx & (1 << 6))
        set_bit(CPUID_FDP_EXCPTN_ONLY, cpuid_bitmap);
    if (extended_features.ebx & (1 << 7))
        set_bit(CPUID_SMEP, cpuid_bitmap);
    if (extended_features.ebx & (1 << 8))
        set_bit(CPUID_BMI2, cpuid_bitmap);
    if (extended_features.ebx & (1 << 9))
        set_bit(CPUID_ERMS, cpuid_bitmap);
    if (extended_features.ebx & (1 << 10))
        set_bit(CPUID_INVPCID, cpuid_bitmap);
    if (extended_features.ebx & (1 << 11))
        set_bit(CPUID_RTM, cpuid_bitmap);
    if (extended_features.ebx & (1 << 12))
        set_bit(CPUID_PQM, cpuid_bitmap);
    if (extended_features.ebx & (1 << 13))
        set_bit(CPUID_ZERO_FCS_FDS, cpuid_bitmap);
    if (extended_features.ebx & (1 << 14))
        set_bit(CPUID_MPX, cpuid_bitmap);
    if (extended_features.ebx & (1 << 15))
        set_bit(CPUID_PQE, cpuid_bitmap);
    if (extended_features.ebx & (1 << 16))
        set_bit(CPUID_AVX512_F, cpuid_bitmap);
    if (extended_features.ebx & (1 << 17))
        set_bit(CPUID_AVX512_DQ, cpuid_bitmap);
    if (extended_features.ebx & (1 << 18))
        set_bit(CPUID_RDSEED, cpuid_bitmap);
    if (extended_features.ebx & (1 << 19))
        set_bit(CPUID_ADX, cpuid_bitmap);
    if (extended_features.ebx & (1 << 20))
        set_bit(CPUID_SMAP, cpuid_bitmap);
    if (extended_features.ebx & (1 << 21))
        set_bit(CPUID_AVX512_IFMA, cpuid_bitmap);
    if (extended_features.ebx & (1 << 22))
        set_bit(CPUID_PCOMMIT, cpuid_bitmap);
    if (extended_features.ebx & (1 << 23))
        set_bit(CPUID_CLFLUSHOPT, cpuid_bitmap);
    if (extended_features.ebx & (1 << 24))
        set_bit(CPUID_CLWB, cpuid_bitmap);
    if (extended_features.ebx & (1 << 25))
        set_bit(CPUID_INTEL_PT, cpuid_bitmap);
    if (extended_features.ebx & (1 << 26))
        set_bit(CPUID_AVX512_PF, cpuid_bitmap);
    if (extended_features.ebx & (1 << 27))
        set_bit(CPUID_AVX512_ER, cpuid_bitmap);
    if (extended_features.ebx & (1 << 28))
        set_bit(CPUID_AVX512_CD, cpuid_bitmap);
    if (extended_features.ebx & (1 << 29))
        set_bit(CPUID_SHA, cpuid_bitmap);
    if (extended_features.ebx & (1 << 30))
        set_bit(CPUID_AVX512_BW, cpuid_bitmap);
    if (extended_features.ebx & (1 << 31))
        set_bit(CPUID_AVX512_VL, cpuid_bitmap);

    if (extended_features.ecx & (1 << 0))
        set_bit(CPUID_PREFETCHWT1, cpuid_bitmap);
    if (extended_features.ecx & (1 << 1))
        set_bit(CPUID_AVX512_VBMI, cpuid_bitmap);
    if (extended_features.ecx & (1 << 2))
        set_bit(CPUID_UMIP, cpuid_bitmap);
    if (extended_features.ecx & (1 << 3))
        set_bit(CPUID_PKU, cpuid_bitmap);
    if (extended_features.ecx & (1 << 4))
        set_bit(CPUID_OSPKE, cpuid_bitmap);
    if (extended_features.ecx & (1 << 5))
        set_bit(CPUID_WAITPKG, cpuid_bitmap);
    if (extended_features.ecx & (1 << 6))
        set_bit(CPUID_AVX512_VBMI2, cpuid_bitmap);
    if (extended_features.ecx & (1 << 7))
        set_bit(CPUID_CET_SS, cpuid_bitmap);
    if (extended_features.ecx & (1 << 8))
        set_bit(CPUID_GFNI, cpuid_bitmap);
    if (extended_features.ecx & (1 << 9))
        set_bit(CPUID_VAES, cpuid_bitmap);
    if (extended_features.ecx & (1 << 10))
        set_bit(CPUID_VPCLMULQDQ, cpuid_bitmap);
    if (extended_features.ecx & (1 << 11))
        set_bit(CPUID_AVX512_VNNI, cpuid_bitmap);
    if (extended_features.ecx & (1 << 12))
        set_bit(CPUID_AVX512_BITALG, cpuid_bitmap);
    if (extended_features.ecx & (1 << 13))
        set_bit(CPUID_TME_EN, cpuid_bitmap);
    if (extended_features.ecx & (1 << 14))
        set_bit(CPUID_AVX512_VPOPCNTDQ, cpuid_bitmap);
    if (extended_features.ecx & (1 << 16))
        set_bit(CPUID_INTEL_5_LEVEL_PAGING, cpuid_bitmap);
    if (extended_features.ecx & (1 << 22))
        set_bit(CPUID_RDPID, cpuid_bitmap);
    if (extended_features.ecx & (1 << 23))
        set_bit(CPUID_KL, cpuid_bitmap);
    if (extended_features.ecx & (1 << 25))
        set_bit(CPUID_CLDEMOTE, cpuid_bitmap);
    if (extended_features.ecx & (1 << 27))
        set_bit(CPUID_MOVDIRI, cpuid_bitmap);
    if (extended_features.ecx & (1 << 28))
        set_bit(CPUID_MOVDIR64B, cpuid_bitmap);
    if (extended_features.ecx & (1 << 29))
        set_bit(CPUID_ENQCMD, cpuid_bitmap);
    if (extended_features.ecx & (1 << 30))
        set_bit(CPUID_SGX_LC, cpuid_bitmap);
    if (extended_features.ecx & (1 << 31))
        set_bit(CPUID_PKS, cpuid_bitmap);

    if (extended_features.edx & (1 << 2))
        set_bit(CPUID_AVX512_4VNNIW, cpuid_bitmap);
    if (extended_features.edx & (1 << 3))
        set_bit(CPUID_AVX512_4FMAPS, cpuid_bitmap);
    if (extended_features.edx & (1 << 4))
        set_bit(CPUID_FSRM, cpuid_bitmap);
    if (extended_features.edx & (1 << 8))
        set_bit(CPUID_AVX512_VP2INTERSECT, cpuid_bitmap);
    if (extended_features.edx & (1 << 9))
        set_bit(CPUID_SRBDS_CTRL, cpuid_bitmap);
    if (extended_features.edx & (1 << 10))
        set_bit(CPUID_MD_CLEAR, cpuid_bitmap);
    if (extended_features.edx & (1 << 11))
        set_bit(CPUID_RTM_ALWAYS_ABORT, cpuid_bitmap);
    if (extended_features.edx & (1 << 13))
        set_bit(CPUID_TSX_FORCE_ABORT, cpuid_bitmap);
    if (extended_features.edx & (1 << 14))
        set_bit(CPUID_SERIALIZE, cpuid_bitmap);
    if (extended_features.edx & (1 << 15))
        set_bit(CPUID_HYBRID, cpuid_bitmap);
    if (extended_features.edx & (1 << 16))
        set_bit(CPUID_TSXLDTRK, cpuid_bitmap);
    if (extended_features.edx & (1 << 18))
        set_bit(CPUID_PCONFIG, cpuid_bitmap);
    if (extended_features.edx & (1 << 19))
        set_bit(CPUID_LBR, cpuid_bitmap);
    if (extended_features.edx & (1 << 20))
        set_bit(CPUID_CET_IBT, cpuid_bitmap);
    if (extended_features.edx & (1 << 22))
        set_bit(CPUID_AMX_BF16, cpuid_bitmap);
    if (extended_features.edx & (1 << 23))
        set_bit(CPUID_AVX512_FP16, cpuid_bitmap);
    if (extended_features.edx & (1 << 24))
        set_bit(CPUID_AMX_TILE, cpuid_bitmap);
    if (extended_features.edx & (1 << 25))
        set_bit(CPUID_AMX_INT8, cpuid_bitmap);
    if (extended_features.edx & (1 << 26))
        set_bit(CPUID_SPEC_CTRL, cpuid_bitmap);
    if (extended_features.edx & (1 << 27))
        set_bit(CPUID_STIBP, cpuid_bitmap);
    if (extended_features.edx & (1 << 28))
        set_bit(CPUID_L1D_FLUSH, cpuid_bitmap);
    if (extended_features.edx & (1 << 29))
        set_bit(CPUID_IA32_ARCH_CAPABILITIES, cpuid_bitmap);
    if (extended_features.edx & (1 << 30))
        set_bit(CPUID_IA32_CORE_CAPABILITIES, cpuid_bitmap);
    if (extended_features.edx & (1 << 31))
        set_bit(CPUID_SSBD, cpuid_bitmap);

    struct cpuid max_extended_leaf_;
    cpuid(0x80000000, 0, &max_extended_leaf_);
    u32 max_extended_leaf = max_extended_leaf_.eax;

    if (max_extended_leaf >= 0x80000001) {
        struct cpuid extended_processor_info;
        cpuid(0x80000001, 0, &extended_processor_info);

        if (extended_processor_info.ecx & (1 << 0))
            set_bit(CPUID_LAHF_LM, cpuid_bitmap);
        if (extended_processor_info.ecx & (1 << 1))
            set_bit(CPUID_CMP_LEGACY, cpuid_bitmap);
        if (extended_processor_info.ecx & (1 << 2))
            set_bit(CPUID_SVM, cpuid_bitmap);
        if (extended_processor_info.ecx & (1 << 3))
            set_bit(CPUID_EXTAPIC, cpuid_bitmap);
        if (extended_processor_info.ecx & (1 << 4))
            set_bit(CPUID_CR8_LEGACY, cpuid_bitmap);
        if (extended_processor_info.ecx & (1 << 5))
            set_bit(CPUID_ABM, cpuid_bitmap);
        if (extended_processor_info.ecx & (1 << 6))
            set_bit(CPUID_SSE4A, cpuid_bitmap);
        if (extended_processor_info.ecx & (1 << 7))
            set_bit(CPUID_MISALIGNSSE, cpuid_bitmap);
        if (extended_processor_info.ecx & (1 << 8))
            set_bit(CPUID__3DNOWPREFETCH, cpuid_bitmap);
        if (extended_processor_info.ecx & (1 << 9))
            set_bit(CPUID_OSVW, cpuid_bitmap);
        if (extended_processor_info.ecx & (1 << 10))
            set_bit(CPUID_IBS, cpuid_bitmap);
        if (extended_processor_info.ecx & (1 << 11))
            set_bit(CPUID_XOP, cpuid_bitmap);
        if (extended_processor_info.ecx & (1 << 12))
            set_bit(CPUID_SKINIT, cpuid_bitmap);
        if (extended_processor_info.ecx & (1 << 13))
            set_bit(CPUID_WDT, cpuid_bitmap);
        if (extended_processor_info.ecx & (1 << 15))
            set_bit(CPUID_LWP, cpuid_bitmap);
        if (extended_processor_info.ecx & (1 << 16))
            set_bit(CPUID_FMA4, cpuid_bitmap);
        if (extended_processor_info.ecx & (1 << 17))
            set_bit(CPUID_TCE, cpuid_bitmap);
        if (extended_processor_info.ecx & (1 << 19))
            set_bit(CPUID_NODEID_MSR, cpuid_bitmap);
        if (extended_processor_info.ecx & (1 << 21))
            set_bit(CPUID_TBM, cpuid_bitmap);
        if (extended_processor_info.ecx & (1 << 22))
            set_bit(CPUID_TOPOEXT, cpuid_bitmap);
        if (extended_processor_info.ecx & (1 << 23))
            set_bit(CPUID_PERFCTR_CORE, cpuid_bitmap);
        if (extended_processor_info.ecx & (1 << 24))
            set_bit(CPUID_PERFCTR_NB, cpuid_bitmap);
        if (extended_processor_info.ecx & (1 << 26))
            set_bit(CPUID_DBX, cpuid_bitmap);
        if (extended_processor_info.ecx & (1 << 27))
            set_bit(CPUID_PERFTSC, cpuid_bitmap);
        if (extended_processor_info.ecx & (1 << 28))
            set_bit(CPUID_PCX_L2I, cpuid_bitmap);

        if (extended_processor_info.edx & (1 << 11))
            set_bit(CPUID_SYSCALL,
                    cpuid_bitmap); // Only available in 64 bit mode
        if (extended_processor_info.edx & (1 << 19))
            set_bit(CPUID_MP, cpuid_bitmap);
        if (extended_processor_info.edx & (1 << 20))
            set_bit(CPUID_NX, cpuid_bitmap);
        if (extended_processor_info.edx & (1 << 22))
            set_bit(CPUID_MMXEXT, cpuid_bitmap);
        if (extended_processor_info.edx & (1 << 23))
            set_bit(CPUID_RDTSCP, cpuid_bitmap);
        if (extended_processor_info.edx & (1 << 25))
            set_bit(CPUID_FXSR_OPT, cpuid_bitmap);
        if (extended_processor_info.edx & (1 << 26))
            set_bit(CPUID_PDPE1GB, cpuid_bitmap);
        if (extended_processor_info.edx & (1 << 27))
            set_bit(CPUID_RDTSCP, cpuid_bitmap);
        if (extended_processor_info.edx & (1 << 29))
            set_bit(CPUID_LM, cpuid_bitmap);
        if (extended_processor_info.edx & (1 << 30))
            set_bit(CPUID__3DNOWEXT, cpuid_bitmap);
        if (extended_processor_info.edx & (1 << 31))
            set_bit(CPUID__3DNOW, cpuid_bitmap);
    }

    if (max_extended_leaf >= 0x80000007) {
        struct cpuid cpuid_;
        cpuid(0x80000001, 0, &cpuid_);
        if (cpuid_.edx & (1 << 8)) {
            set_bit(CPUID_CONSTANT_TSC, cpuid_bitmap);
            set_bit(CPUID_NONSTOP_TSC, cpuid_bitmap);
        }
    }
}

int cpu_supports(enum amd64_cpuid feature) {
    expects(feature < CPUID___End);
    return test_bit(feature, cpuid_bitmap);
}
