pub const raspi = 4;

pub const DEVICE_BASE = 0xFE000000;
pub const DEVICE_START = 0x3B400000;

pub const PHYS_MEMORY_SIZE = 0x40000000;

pub const PAGE_MASK = 0xfffffffffffff000;
pub const PAGE_SHIFT = 12;
pub const TABLE_SHIFT = 9;
pub const SECTION_SHIFT = (PAGE_SHIFT + TABLE_SHIFT);

pub const PAGE_SIZE = (1 << PAGE_SHIFT);
pub const SECTION_SIZE = (1 << SECTION_SHIFT);

pub const LOW_MEMORY = (2 * SECTION_SIZE);
pub const HIGH_MEMORY = 0x40000000;

pub const PAGING_MEMORY = (HIGH_MEMORY - LOW_MEMORY);
pub const PAGING_PAGES = (PAGING_MEMORY / PAGE_SIZE);

pub const PG_DIR_SIZE = (3 * PAGE_SIZE);
pub const MM_TYPE_PAGE_TABLE = 0x3;
pub const MM_TYPE_PAGE = 0x3;
pub const MM_TYPE_BLOCK = 0x1;
pub const MM_ACCESS = (0x1 << 10);
pub const MM_ACCESS_PERMISSION = (0x01 << 6);

pub const SCTLR_RESERVED = (3 << 28) | (3 << 22) | (1 << 20) | (1 << 11);
pub const SCTLR_EE_LITTLE_ENDIAN = (0 << 25);
pub const SCTLR_EOE_LITTLE_ENDIAN = (0 << 24);
pub const SCTLR_I_CACHE_DISABLED = (0 << 12);
pub const SCTLR_D_CACHE_DISABLED = (0 << 2);
pub const SCTLR_I_CACHE_ENABLED = (1 << 12);
pub const SCTLR_D_CACHE_ENABLED = (1 << 2);
pub const SCTLR_MMU_DISABLED = (0 << 0);
pub const SCTLR_MMU_ENABLED = (1 << 0);

pub const SCTLR_VALUE_MMU_DISABLED = (SCTLR_RESERVED | SCTLR_EE_LITTLE_ENDIAN | SCTLR_I_CACHE_DISABLED | SCTLR_D_CACHE_DISABLED | SCTLR_MMU_DISABLED);
pub const SCTLR_VALUE_MMU_ENABLED = 1 << 0;

pub const TCR_TG1_4K = (2 << 30);
pub const TCR_T1SZ = ((64 - 48) << 16);
pub const TCR_TG0_4K = (0 << 14);
pub const TCR_T0SZ = (64 - 48);
pub const TCR_VALUE = (TCR_TG1_4K | TCR_T1SZ | TCR_TG0_4K | TCR_T0SZ);

// ***************************************
// HCR_EL2, Hypervisor Configuration Register (EL2), Page 2487 of AArch64-Reference-Manual.
// ***************************************

pub const HCR_RW = (1 << 31);
pub const HCR_VALUE = HCR_RW;

// ***************************************
// SCR_EL3, Secure Configuration Register (EL3), Page 2648 of AArch64-Reference-Manual.
// ***************************************

pub const SCR_RESERVED = (3 << 4);
pub const SCR_RW = (1 << 10);
pub const SCR_NS = (1 << 0);
pub const SCR_VALUE = (SCR_RESERVED | SCR_RW | SCR_NS);

// ***************************************
// SPSR_EL3, Saved Program Status Register (EL3) Page 389 of AArch64-Reference-Manual.
// ***************************************

pub const SPSR_MASK_ALL = (7 << 6);
pub const SPSR_EL1h = (5 << 0);
pub const SPSR_VALUE = SPSR_MASK_ALL | SPSR_EL1h;

// Memory region attributes:
//
//   n = AttrIndx[2:0]
//            n    MAIR
//   DEVICE_nGnRnE    000    00000000
//   NORMAL_NC        001    01000100
//
pub const MT_DEVICE_nGnRnE = 0x0;
pub const MT_NORMAL_NC = 0x1;
pub const MT_DEVICE_nGnRnE_FLAGS = 0x00;
pub const MT_NORMAL_NC_FLAGS = 0x44;
pub const MAIR_VALUE = (MT_DEVICE_nGnRnE_FLAGS << (8 * MT_DEVICE_nGnRnE)) | (MT_NORMAL_NC_FLAGS << (8 * MT_NORMAL_NC));

pub const MMU_FLAGS = (MM_TYPE_BLOCK | (MT_NORMAL_NC << 2) | MM_ACCESS);
pub const MMU_DEVICE_FLAGS = (MM_TYPE_BLOCK | (MT_DEVICE_nGnRnE << 2) | MM_ACCESS);

pub const VECTOR_TABLE: [16]usize align(128) = undefined;

pub const PAGE_TABLE_SIZE = 512;
pub const PAGE_TABLE_ALIGNMENT = 0x4000;

pub const L1_RAM_ATTRS = 0x0000000000000045;
pub const L1_DEVICE_ATTRS = 0x0000000000000041;

pub const L0_ATTR = 0xC3;

var L0_PAGE_TABLE: [PAGE_TABLE_SIZE]usize align(PAGE_TABLE_ALIGNMENT) = undefined;
var L1_PAGE_TABLE: [PAGE_TABLE_SIZE]usize align(PAGE_TABLE_ALIGNMENT) = undefined;

pub const CPACR_EL1_FPEN = (1 << 21) | (1 << 20); // don't trap SIMD/FP registers
pub const CPACR_EL1_ZEN = (1 << 17) | (1 << 16); // don't trap SVE instructions
pub const CPACR_VALUE = (CPACR_EL1_FPEN | CPACR_EL1_ZEN);
