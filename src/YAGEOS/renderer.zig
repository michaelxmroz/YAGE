const mmio = @import("mmio.zig");
const utils = @import("utils.zig");
const log = @import("log.zig");

var mbox: [36]u32 align(16) = undefined;
var framebuffer: *volatile u32 = undefined;
var framebuffer_pitch: u32 = 0;

const VIDEOCORE_MBOX = (mmio.MMIO_BASE + 0x0000B880);
const MBOX_READ      = (VIDEOCORE_MBOX + 0x0);
const MBOX_POLL      = (VIDEOCORE_MBOX + 0x10);
const MBOX_SENDER    = (VIDEOCORE_MBOX + 0x14);
const MBOX_STATUS    = (VIDEOCORE_MBOX + 0x18);
const MBOX_CONFIG    = (VIDEOCORE_MBOX + 0x1C);
const MBOX_WRITE     = (VIDEOCORE_MBOX + 0x20);
const MBOX_FULL  = 0x80000000;
const MBOX_EMPTY     = 0x40000000;
const MBOX_DATA_MASK = 0xF;
const MBOX_VC_CHANNEL = 0x8;

const MBOX_TAGS = enum (u32) 
{
    SETPOWER   = 0x28001,
    SETCLKRATE = 0x38002,

    SETPHYWH   = 0x48003,
    SETVIRTWH  = 0x48004,
    SETVIRTOFF = 0x48009,
    SETDEPTH   = 0x48005,
    SETPXLORDR = 0x48006,
    GETFB      = 0x40001,
    GETPITCH   = 0x40008,

    LAST       = 0
};

const VIDEOCORE_REQUEST = 0x0;

const VIDOCORE_RESPONSE_CODES = enum(u32)
{
    SUCCESS = 0x80000000,
    ERROR = 0x80000001 
};

fn writeToVCMailbox(data : usize) void
{
    const msg = (data & ~@as(u32,MBOX_DATA_MASK)) | MBOX_VC_CHANNEL;
    while(mmio.mmioReadDirect(MBOX_STATUS) & MBOX_FULL != 0)
    {
        utils.delay(4);
    }
    mmio.mmioWriteDirect(MBOX_WRITE, @intCast(msg));
}

fn readFromVCMailbox() u32
{
    while(mmio.mmioReadDirect(MBOX_STATUS) & MBOX_EMPTY != 0)
    {
        utils.delay(4);
    }

    const resp = mmio.mmioReadDirect(MBOX_READ);
    return resp & ~@as(u32,MBOX_DATA_MASK);
}

pub fn initFramebuffer() void {
    // Define the framebuffer parameters
    const width = 1920;
    const height = 1080;
    const depth = 32;


    mbox[0] = 35*4; // Length of message in bytes
    mbox[1] = VIDEOCORE_REQUEST;

    mbox[2] = @intFromEnum(MBOX_TAGS.SETPHYWH); // Tag identifier
    mbox[3] = 8; // Value size in bytes
    mbox[4] = 0;
    mbox[5] = width; // Value(width)
    mbox[6] = height; // Value(height)

    mbox[7] = @intFromEnum(MBOX_TAGS.SETVIRTWH);
    mbox[8] = 8;
    mbox[9] = 8;
    mbox[10] = width;
    mbox[11] = height;

    mbox[12] = @intFromEnum(MBOX_TAGS.SETVIRTOFF);
    mbox[13] = 8;
    mbox[14] = 8;
    mbox[15] = 0; // Value(x)
    mbox[16] = 0; // Value(y)

    mbox[17] = @intFromEnum(MBOX_TAGS.SETDEPTH);
    mbox[18] = 4;
    mbox[19] = 4;
    mbox[20] = depth; // Bits per pixel

    mbox[21] = @intFromEnum(MBOX_TAGS.SETPXLORDR);
    mbox[22] = 4;
    mbox[23] = 4;
    mbox[24] = 1; // RGB

    mbox[25] = @intFromEnum(MBOX_TAGS.GETFB);
    mbox[26] = 8;
    mbox[27] = 8;
    mbox[28] = 4096; // FrameBufferInfo.pointer
    mbox[29] = 0;    // FrameBufferInfo.size

    mbox[30] = @intFromEnum(MBOX_TAGS.GETPITCH);
    mbox[31] = 4;
    mbox[32] = 4;
    mbox[33] = 0; // Bytes per line

    mbox[34] = @intFromEnum(MBOX_TAGS.LAST);

    
    // Write the mailbox address, aligned and with channel info
    writeToVCMailbox(@intFromPtr(&mbox));

    // Await VideoCore response
    if (readFromVCMailbox() != @intFromEnum(VIDOCORE_RESPONSE_CODES.SUCCESS)) {
        log.ERROR("Framebuffer request failed", .{});
        return;
    }

    // Process response from VideoCore
    const fb_addr = mbox[5]; // Framebuffer address in mbox response (Tag 0x40001 response)
    const fb_pitch = mbox[10]; // Framebuffer pitch from Tag 0x40008

    if (fb_addr == 0 or fb_pitch == 0) {
        log.ERROR("Framebuffer initialization failed", .{});
        return;
    }

    // Assign framebuffer properties
    framebuffer = @ptrFromInt(fb_addr & ~@as(u32,MBOX_DATA_MASK));
    framebuffer_pitch = fb_pitch;

    log.INFO("Framebuffer initialized at {x} with pitch {d}", .{framebuffer, framebuffer_pitch});
}

