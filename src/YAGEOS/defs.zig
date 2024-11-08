pub const raspi = 4;  // Raspberry Pi version (4 for Raspberry Pi 4).

// Device memory base and starting address.
pub const DEVICE_BASE = 0xFE000000;   // Base physical address for device memory.
pub const DEVICE_START = 0x3B400000;  // Device memory start address in virtual address space.

// Page and memory section settings.
pub const PAGE_MASK = 0xfffffffffffff000; // Mask for aligning addresses to page boundaries.
pub const PAGE_SHIFT = 12;                // Page shift value, used to calculate page size (4 KB).
pub const TABLE_SHIFT = 9;                // Table shift value, used for table indexing.
pub const SECTION_SHIFT = (PAGE_SHIFT + TABLE_SHIFT); // Section shift (combines page and table shifts).

pub const PAGE_SIZE = (1 << PAGE_SHIFT);  // Page size in bytes (4 KB).
pub const SECTION_SIZE = (1 << SECTION_SHIFT); // Section size (2 MB).

// Memory layout configuration.
pub const LOW_MEMORY = (2 * SECTION_SIZE); // Low memory boundary, 4 MB.

// Page directory and table settings.
pub const PG_DIR_SIZE = (3 * PAGE_SIZE);  // Total size for page directories (3 levels).
pub const MM_TYPE_PAGE_TABLE = 0x3;       // Type for page table entries.
pub const MM_TYPE_PAGE = 0x3;             // Type for regular page entries.
pub const MM_TYPE_BLOCK = 0x1;            // Type for block entries (used for larger memory blocks).
pub const MM_ACCESS = (0x1 << 10);        // Access flag for memory entries.
pub const MM_ACCESS_PERMISSION = (0x01 << 6); // Permission flag for memory access control.

// System Control Register (SCTLR) bit flags for system configuration.
pub const SCTLR_RESERVED = (3 << 28) | (3 << 22) | (1 << 20) | (1 << 11); // Reserved SCTLR bits.
pub const SCTLR_EE_LITTLE_ENDIAN = (0 << 25);   // Sets system to little-endian mode.
pub const SCTLR_EOE_LITTLE_ENDIAN = (0 << 24);  // Endianness for data, also little-endian.
pub const SCTLR_I_CACHE_DISABLED = (0 << 12);   // Disables instruction cache.
pub const SCTLR_D_CACHE_DISABLED = (0 << 2);    // Disables data cache.
pub const SCTLR_I_CACHE_ENABLED = (1 << 12);    // Enables instruction cache.
pub const SCTLR_D_CACHE_ENABLED = (1 << 2);     // Enables data cache.
pub const SCTLR_MMU_DISABLED = (0 << 0);        // Disables MMU.
pub const SCTLR_MMU_ENABLED = (1 << 0);         // Enables MMU.

pub const SCTLR_VALUE_MMU_DISABLED = (SCTLR_RESERVED | SCTLR_EE_LITTLE_ENDIAN 
                                     | SCTLR_I_CACHE_DISABLED | SCTLR_D_CACHE_DISABLED | SCTLR_MMU_DISABLED);
pub const SCTLR_VALUE_MMU_ENABLED = (SCTLR_MMU_ENABLED | SCTLR_I_CACHE_ENABLED | SCTLR_D_CACHE_ENABLED);

// Translation Control Register (TCR) settings for translation granules and address sizes.
pub const TCR_TG1_4K = (2 << 30);              // 4 KB granule for the upper half of address space.
pub const TCR_T1SZ = ((64 - 48) << 16);        // Size of the virtual address space for upper half.
pub const TCR_TG0_4K = (0 << 14);              // 4 KB granule for the lower half of address space.
pub const TCR_T0SZ = (64 - 48);                // Size of the virtual address space for lower half.
pub const TCR_VALUE = (TCR_TG1_4K | TCR_T1SZ | TCR_TG0_4K | TCR_T0SZ); // Combined TCR value.

// Hypervisor Configuration Register (HCR) settings.
pub const HCR_RW = (1 << 31);  // Set CPU to use AArch64 mode for exception levels.
pub const HCR_VALUE = HCR_RW;  // HCR value with RW setting.

// Secure Configuration Register (SCR) settings.
pub const SCR_RESERVED = (3 << 4);    // Reserved bits in SCR.
pub const SCR_RW = (1 << 10);         // AArch64 mode for secure state.
pub const SCR_NS = (1 << 0);          // Set non-secure state.
pub const SCR_VALUE = (SCR_RESERVED | SCR_RW | SCR_NS); // Combined SCR value.

// Saved Program Status Register (SPSR) settings for masking and exception level.
pub const SPSR_MASK_ALL = (7 << 6);   // Mask all exceptions.
pub const SPSR_EL1h = (5 << 0);       // Exception level 1, handler mode.
pub const SPSR_VALUE = SPSR_MASK_ALL | SPSR_EL1h; // Combined SPSR value.

// Memory attributes for the Memory Attribute Indirection Register (MAIR).
pub const MT_DEVICE_nGnRnE = 0x0;           // Device, non-Gathering, non-Reordering, no Early acknowledgment.
pub const MT_NORMAL_NC = 0x1;               // Normal, Non-Cacheable memory.
pub const MT_DEVICE_nGnRnE_FLAGS = 0x00;    // Memory attribute flags for device memory.
pub const MT_NORMAL_NC_FLAGS = 0x44;        // Memory attribute flags for normal non-cacheable memory.
pub const MAIR_VALUE = (MT_DEVICE_nGnRnE_FLAGS << (8 * MT_DEVICE_nGnRnE)) 
                      | (MT_NORMAL_NC_FLAGS << (8 * MT_NORMAL_NC)); // Combined MAIR value.

// Flags for MMU entries for different types of memory.
pub const MMU_FLAGS = (MM_TYPE_BLOCK | (MT_NORMAL_NC << 2) | MM_ACCESS);       // Flags for normal memory blocks.
pub const MMU_DEVICE_FLAGS = (MM_TYPE_BLOCK | (MT_DEVICE_nGnRnE << 2) | MM_ACCESS); // Flags for device memory blocks.

// Vector table definition (128-byte aligned).
pub const VECTOR_TABLE: [16]usize align(128) = undefined; // Exception vector table.

// Page table configurations.
pub const PAGE_TABLE_SIZE = 512;          // Number of entries in a page table.
pub const PAGE_TABLE_ALIGNMENT = 0x4000;  // Alignment requirement for page tables.

// Level 1 memory attributes.
pub const L1_RAM_ATTRS = 0x0000000000000045;    // Attributes for RAM in level 1 table.
pub const L1_DEVICE_ATTRS = 0x0000000000000041; // Attributes for device memory in level 1 table.

// Level 0 memory attributes.
pub const L0_ATTR = 0xC3;  // Level 0 attributes.

// Page table memory definitions.
var L0_PAGE_TABLE: [PAGE_TABLE_SIZE]usize align(PAGE_TABLE_ALIGNMENT) = undefined; // Level 0 page table.
var L1_PAGE_TABLE: [PAGE_TABLE_SIZE]usize align(PAGE_TABLE_ALIGNMENT) = undefined; // Level 1 page table.

// Coprocessor Access Control Register (CPACR) flags.
pub const CPACR_EL1_FPEN = (1 << 21) | (1 << 20); // Enables access to SIMD/FP registers.
pub const CPACR_EL1_ZEN = (1 << 17) | (1 << 16);  // Enables access to SVE (Scalable Vector Extension) instructions.
pub const CPACR_VALUE = (CPACR_EL1_FPEN | CPACR_EL1_ZEN); // Combined CPACR value.
