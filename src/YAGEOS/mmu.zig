// Memory Management Unit Setup.
// We use a simplified setup to enable virtual memory for unaligned memory access.

const defs = @import("defs.zig");

// Define constants representing bit flags for table and block descriptors.
const TD_VALID = (1 << 0);           // Marks descriptor as valid.
const TD_BLOCK = (0 << 1);           // Specifies a block descriptor.
const TD_TABLE = (1 << 1);           // Specifies a table descriptor.
const TD_ACCESS = (1 << 10);         // Allows access to the descriptor.
const TD_KERNEL_PERMS = (1 << 54);   // Grants kernel-level permissions.
const TD_INNER_SHARABLE = (3 << 8);  // Sets the descriptor as inner-sharable.

// Flags for kernel table and block descriptors.
const TD_KERNEL_TABLE_FLAGS = (TD_TABLE | TD_VALID);
const TD_KERNEL_BLOCK_FLAGS = (TD_ACCESS | TD_INNER_SHARABLE | TD_KERNEL_PERMS 
                              | (MATTR_NORMAL_NC_INDEX << 2) | TD_BLOCK | TD_VALID);
const TD_DEVICE_BLOCK_FLAGS = (TD_ACCESS | TD_INNER_SHARABLE | TD_KERNEL_PERMS 
                               | (MATTR_DEVICE_nGnRnE_INDEX << 2) | TD_BLOCK | TD_VALID);

// Memory attribute values and indexes for different memory types.
const MATTR_DEVICE_nGnRnE = 0x0;  // Device, non-Gathering, non-Reordering, no Early acknowledgment.
const MATTR_NORMAL_NC = 0x44;     // Normal, Non-Cacheable memory.
const MATTR_DEVICE_nGnRnE_INDEX = 0;
const MATTR_NORMAL_NC_INDEX = 1;

// Defines the memory attribute index register value for setting memory attributes.
const MAIR_EL1_VAL = ((MATTR_NORMAL_NC << (8 * MATTR_NORMAL_NC_INDEX)) 
                     | MATTR_DEVICE_nGnRnE << (8 * MATTR_DEVICE_nGnRnE_INDEX));

// Constants for table and page sizes.
const ID_MAP_PAGES = 6;
const ID_MAP_TABLE_SIZE = (ID_MAP_PAGES * defs.PAGE_SIZE); // Total size for the identity map.
const ENTRIES_PER_TABLE = 512;   // Number of entries in each table.
const PGD_SHIFT = (defs.PAGE_SHIFT + 3 * defs.TABLE_SHIFT); // Offset for the page global directory.
const PUD_SHIFT = (defs.PAGE_SHIFT + 2 * defs.TABLE_SHIFT); // Offset for the page upper directory.
const PMD_SHIFT = (defs.PAGE_SHIFT + defs.TABLE_SHIFT);     // Offset for the page middle directory.
const PUD_ENTRY_MAP_SIZE = (1 << PUD_SHIFT);                // Map size for each PUD entry.
const BLOCK_SIZE = 0x40000000;  // Block size in bytes.

// Creates a table entry in a page table for virtual memory management.
fn create_table_entry(tbl: u64, next_tbl: u64, va: u64, shift: u6, flags: u64) void 
{
    var table_index = va >> shift;                 // Calculate table index by shifting virtual address.
    table_index &= (ENTRIES_PER_TABLE - 1);        // Mask to fit within table entry range.
    const descriptor = next_tbl | flags;           // Descriptor contains the address of the next table and flags.
    const ptr: *volatile u64 = @ptrFromInt(tbl + (table_index << 3));  // Pointer to table entry in memory.
    ptr.* = descriptor;                            // Set table entry to descriptor.
}

// Maps a range of virtual addresses to physical addresses with block descriptors.
fn create_block_map(pmd: u64, vstart: u64, vend: u64, pa: u64) void 
{
    var _vstart = vstart >> defs.SECTION_SHIFT;    // Calculate block-aligned starting index for vstart.
    _vstart &= (ENTRIES_PER_TABLE - 1);            // Mask to fit within table entry range.

    var _vend = vend >> defs.SECTION_SHIFT;        // Calculate block-aligned ending index for vend.
    _vend -= 1;
    _vend &= (ENTRIES_PER_TABLE - 1);

    var paAdj = pa >> defs.SECTION_SHIFT;          // Adjust physical address for block alignment.
    paAdj <<= defs.SECTION_SHIFT;

    // First entry setup
    {
        var _pa = paAdj;

        // Set block flags based on physical address (kernel or device).
        if (paAdj >= defs.DEVICE_START) 
        {
            _pa |= TD_DEVICE_BLOCK_FLAGS;
        } else {
            _pa |= TD_KERNEL_BLOCK_FLAGS;
        }
        
        const ptr: *volatile u64 = @ptrFromInt(pmd + (_vstart << 3)); // Pointer to table entry.
        ptr.* = _pa;  // Set block entry.

        // Update for next block.
        paAdj += defs.SECTION_SIZE;
        _vstart += 1;
    }

    // Map subsequent blocks from _vstart to _vend.
    while (_vstart <= _vend) 
    {
        var _pa = paAdj;

        if (paAdj >= defs.DEVICE_START) 
        {
            _pa |= TD_DEVICE_BLOCK_FLAGS;
        } else {
            _pa |= TD_KERNEL_BLOCK_FLAGS;
        }

        const ptr: *volatile u64 = @ptrFromInt(pmd + (_vstart << 3)); // Pointer to table entry.
        ptr.* = _pa;  // Set block entry.

        paAdj += defs.SECTION_SIZE;
        _vstart += 1;
    }
}

// External functions that provide addresses for page tables and zero out memory.
extern fn id_pgd_addr() u64;
extern fn memzero(addr: u64, size: u64) void;

// Initializes the Memory Management Unit (MMU) by setting up identity mapping.
pub fn initMMU() void 
{
    const id_pgd = id_pgd_addr();          // Get the address of the page global directory.
    memzero(id_pgd, ID_MAP_TABLE_SIZE);    // Clear memory for the identity map table.

    var map_base: u64 = 0;                 // Start address for identity mapping.
    var tbl = id_pgd;
    var next_tbl = tbl + defs.PAGE_SIZE;   // Address of the next table.

    create_table_entry(tbl, next_tbl, map_base, PGD_SHIFT, TD_KERNEL_TABLE_FLAGS);  // Set up PGD entry.

    tbl += defs.PAGE_SIZE;
    next_tbl += defs.PAGE_SIZE;

    var block_tbl = tbl;

    // Create entries for each PUD and map blocks.
    for (0..4) |i| 
    {
        create_table_entry(tbl, next_tbl, map_base, PUD_SHIFT, TD_KERNEL_TABLE_FLAGS); // Create PUD entry.
        next_tbl += defs.PAGE_SIZE;
        map_base += PUD_ENTRY_MAP_SIZE;

        block_tbl += defs.PAGE_SIZE;
        const offset = BLOCK_SIZE * i;     // Offset for each block.
        create_block_map(block_tbl, offset, offset + BLOCK_SIZE, offset); // Map each block.
    }
}
