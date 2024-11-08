const defs = @import("defs.zig");

const TD_VALID = (1 << 0);
const TD_BLOCK = (0 << 1);
const TD_TABLE = (1 << 1);
const TD_ACCESS = (1 << 10);
const TD_KERNEL_PERMS = (1 << 54);
const TD_INNER_SHARABLE = (3 << 8);
const TD_KERNEL_TABLE_FLAGS = (TD_TABLE | TD_VALID);
const TD_KERNEL_BLOCK_FLAGS = (TD_ACCESS | TD_INNER_SHARABLE | TD_KERNEL_PERMS | (MATTR_NORMAL_NC_INDEX << 2) | TD_BLOCK | TD_VALID);
const TD_DEVICE_BLOCK_FLAGS = (TD_ACCESS | TD_INNER_SHARABLE | TD_KERNEL_PERMS | (MATTR_DEVICE_nGnRnE_INDEX << 2) | TD_BLOCK | TD_VALID);
const MATTR_DEVICE_nGnRnE = 0x0;
const MATTR_NORMAL_NC = 0x44;
const MATTR_DEVICE_nGnRnE_INDEX = 0;
const MATTR_NORMAL_NC_INDEX = 1;
const MAIR_EL1_VAL = ((MATTR_NORMAL_NC << (8 * MATTR_NORMAL_NC_INDEX)) | MATTR_DEVICE_nGnRnE << (8 * MATTR_DEVICE_nGnRnE_INDEX));
const ID_MAP_PAGES = 6;
const ID_MAP_TABLE_SIZE = (ID_MAP_PAGES * defs.PAGE_SIZE);
const ENTRIES_PER_TABLE = 512;
const PGD_SHIFT = (defs.PAGE_SHIFT + 3 * defs.TABLE_SHIFT);
const PUD_SHIFT = (defs.PAGE_SHIFT + 2 * defs.TABLE_SHIFT);
const PMD_SHIFT = (defs.PAGE_SHIFT + defs.TABLE_SHIFT);
const PUD_ENTRY_MAP_SIZE = (1 << PUD_SHIFT);

const BLOCK_SIZE = 0x40000000;

fn create_table_entry(tbl: u64, next_tbl: u64, va: u64, shift: u6, flags: u64) void 
{
    var table_index = va >> shift;
    table_index &= (ENTRIES_PER_TABLE - 1);
    const descriptor = next_tbl | flags;
    const ptr: *volatile u64 = @ptrFromInt(tbl + (table_index << 3));

    ptr.* = descriptor;
}

fn create_block_map(pmd: u64, vstart: u64, vend: u64, pa: u64) void 
{
    var _vstart = vstart >> defs.SECTION_SHIFT;
    _vstart &= (ENTRIES_PER_TABLE - 1);

    var _vend = vend >> defs.SECTION_SHIFT;
    _vend -= 1;
    _vend &= (ENTRIES_PER_TABLE - 1);

    var paAdj = pa >> defs.SECTION_SHIFT;
    paAdj <<= defs.SECTION_SHIFT;

    {
        var _pa = paAdj;

        if (paAdj >= defs.DEVICE_START) 
        {
            _pa |= TD_DEVICE_BLOCK_FLAGS;
        } else {
            _pa |= TD_KERNEL_BLOCK_FLAGS;
        }
        const ptr: *volatile u64 = @ptrFromInt(pmd + (_vstart << 3));
        ptr.* = _pa;

        paAdj += defs.SECTION_SIZE;
        _vstart += 1;
    }

    while (_vstart <= _vend) 
    {
        var _pa = paAdj;

        if (paAdj >= defs.DEVICE_START) 
        {
            _pa |= TD_DEVICE_BLOCK_FLAGS;
        } else 
        {
            _pa |= TD_KERNEL_BLOCK_FLAGS;
        }

        const ptr: *volatile u64 = @ptrFromInt(pmd + (_vstart << 3));
        ptr.* = _pa;

        paAdj += defs.SECTION_SIZE;
        _vstart += 1;
    }
}

extern fn id_pgd_addr() u64;
extern fn memzero(addr: u64, size: u64) void;

pub fn initMMU() void 
{
    const id_pgd = id_pgd_addr();

    memzero(id_pgd, ID_MAP_TABLE_SIZE);

    var map_base: u64 = 0;
    var tbl = id_pgd;
    var next_tbl = tbl + defs.PAGE_SIZE;

    create_table_entry(tbl, next_tbl, map_base, PGD_SHIFT, TD_KERNEL_TABLE_FLAGS);

    tbl += defs.PAGE_SIZE;
    next_tbl += defs.PAGE_SIZE;

    var block_tbl = tbl;

    for (0..4) |i| 
    {
        create_table_entry(tbl, next_tbl, map_base, PUD_SHIFT, TD_KERNEL_TABLE_FLAGS);

        next_tbl += defs.PAGE_SIZE;
        map_base += PUD_ENTRY_MAP_SIZE;

        block_tbl += defs.PAGE_SIZE;

        const offset = BLOCK_SIZE * i;
        create_block_map(block_tbl, offset, offset + BLOCK_SIZE, offset);
    }
}
