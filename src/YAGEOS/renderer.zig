const mmio = @import("mmio.zig");
const utils = @import("utils.zig");
const log = @import("log.zig");

const FramebufferRequest = packed struct {
    size: u32,
    request_code: u32,
    tags: [5]Tag,
    end_tag: u32,
};

const Tag = packed struct {
    id: u32,
    buffer_size: u32,
    value_length: u32,
    value: u64,
};

var mbox: [36]u32 align(16) = undefined;
var framebuffer: *volatile u32 = undefined;
var framebuffer_pitch: u32 = 0;

const MBOX_REGS = enum (u32) 
{
    const VIDEOCORE_MBOX = (mmio.MMIO_BASE + 0x0000B880);

    MBOX_READ      = (VIDEOCORE_MBOX + 0x0),
    MBOX_POLL      = (VIDEOCORE_MBOX + 0x10),
    MBOX_SENDER    = (VIDEOCORE_MBOX + 0x14),
    MBOX_STATUS    = (VIDEOCORE_MBOX + 0x18),
    MBOX_CONFIG    = (VIDEOCORE_MBOX + 0x1C),
    MBOX_WRITE     = (VIDEOCORE_MBOX + 0x20),
    MBOX_RESPONSE  = 0x80000000,
    MBOX_FULL      = 0x80000000,
    MBOX_EMPTY     = 0x40000000,
    MBOX_DATA_MASK = 0xF,
    MBOX_VC_CHANNEL = 0x8
};

const VIDEOCORE_REQUEST = 0x0;

const VIDOCORE_RESPONSE_CODES = enum(u32)
{
    SUCCESS = 0x80000000,
    ERROR = 0x80000001 
};

fn writeToVCMailbox(data : u32) void
{
    const msg = (data & ~MBOX_REGS.MBOX_DATA_MASK) | MBOX_REGS.MBOX_VC_CHANNEL;
    while(mmio.mmioRead(MBOX_REGS.MBOX_STATUS) & MBOX_REGS.MBOX_FULL != 0)
    {
        utils.delay(4);
    }
    mmio.mmioWrite(MBOX_REGS.MBOX_WRITE, msg);
}

fn readFromVCMailbox() u32
{
    while(mmio.mmioRead(MBOX_REGS.MBOX_STATUS) & MBOX_REGS.MBOX_EMPTY != 0)
    {
        utils.delay(4);
    }

    const resp = mmio.mmioRead(MBOX_REGS.MBOX_READ);
    return resp & ~MBOX_REGS.MBOX_DATA_MASK;
}

pub fn initFramebuffer() void 
{
    log.ERROR("Unimplemented func", .{});
}
