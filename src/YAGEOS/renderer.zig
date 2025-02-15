const mmio = @import("mmio.zig");
const utils = @import("utils.zig");
const log = @import("log.zig");

// Mailbox buffer and framebuffer properties
var mbox: [36]u32 align(16) = undefined;
var framebuffer: *volatile u32 = undefined;
var framebuffer_width: u32 = 0;
var framebuffer_height: u32 = 0;
var framebuffer_pitch: u32 = 0;
var framebuffer_isrgb: u32 = 0;

// Mailbox registers and constants
const VIDEOCORE_MBOX = (mmio.MMIO_BASE + 0x0000B880);
const MBOX_READ = (VIDEOCORE_MBOX + 0x0);
const MBOX_STATUS = (VIDEOCORE_MBOX + 0x18);
const MBOX_WRITE = (VIDEOCORE_MBOX + 0x20);

const MBOX_POLL      = (VIDEOCORE_MBOX + 0x10);
const MBOX_SENDER    = (VIDEOCORE_MBOX + 0x14);
const MBOX_CONFIG    = (VIDEOCORE_MBOX + 0x1C);
const MBOX_DATA_MASK = 0xF;

const MBOX_FULL = 0x80000000;  // Mailbox full flag
const MBOX_EMPTY = 0x40000000; // Mailbox empty flag
const MBOX_VC_CHANNEL = 0x8;   // Mailbox VideoCore channel

// Mailbox tags
const MBOX_TAGS = enum (u32) {
    SETPHYWH   = 0x48003,
    SETVIRTWH  = 0x48004,
    SETVIRTOFF = 0x48009,
    SETDEPTH   = 0x48005,
    SETPXLORDR = 0x48006,
    ALLOCFB    = 0x40001,
    GETPITCH   = 0x40008,
    LAST       = 0,
};

// VideoCore request and response codes
const VIDEOCORE_REQUEST = 0x0;
const VIDEOCORE_RESPONSE_CODES = enum (u32) {
    SUCCESS = 0x80000000,
    ERROR = 0x80000001,
};

pub const Components = packed struct
{
    r : u8,
    g : u8,
    b : u8,
    a : u8,
};

pub const Color = union
{
    components: Components,

    fn toUnified(self: *const Color) u32 {
        // Reinterpret the packed struct as a u32
        return @bitCast(self.components);
    }

    fn fromUnified(self: *const Color, value: u32) void {
        // Reinterpret the u32 as a packed struct
        self.components = @bitCast(value);
    }
};



fn writeToVCMailbox(data : usize) void
{
    const msg = (data & ~@as(u32,MBOX_DATA_MASK)) | MBOX_VC_CHANNEL;
    while(mmio.mmioReadDirect(MBOX_STATUS) & MBOX_FULL != 0)
    {
        @call(.never_inline, utils.delay, .{4});
    }
    mmio.mmioWriteDirect(MBOX_WRITE, @intCast(msg));
}

fn readFromVCMailbox() u32
{
    while(mmio.mmioReadDirect(MBOX_STATUS) & MBOX_EMPTY != 0)
    {
        @call(.never_inline, utils.delay, .{4});
    }

    const resp = mmio.mmioReadDirect(MBOX_READ);
    return resp & ~@as(u32,MBOX_DATA_MASK);
}

fn addTag(buf: []u32, idx: *u32, tag: MBOX_TAGS, data: []const u32) void 
{
    const u32len = @as(u32, @intCast(data.len));
    buf[idx.*] = @intFromEnum(tag);                // Tag identifier
    buf[idx.* + 1] = u32len * 4;                // Value size in bytes
    buf[idx.* + 2] = 0;                           // Request code (0)
    @memcpy( buf[idx.* + 3..idx.* + 3 + u32len], data); // Copy tag data
    idx.* += 3 + u32len;                        // Move the index forward
}

// Utility function to find a tag and retrieve its data from the mailbox response
fn findTag(buf: []const u32, tag: MBOX_TAGS) ?[]const u32 {
    var idx: usize = 2; // Start after the size and request code
    while (idx < buf.len) {
        const current_tag = @intFromEnum(tag);
        if (buf[idx] == current_tag) {
            const data_len = buf[idx + 1] / 4; // Length in words (u32)
            return buf[idx + 3..idx + 3 + data_len];
        }
        // Skip to the next tag: tag + size (2 words) + data length
        idx += 3 + (buf[idx + 1] / 4);
    }
    return null; // Tag not found
}


// Initialize the framebuffer
pub fn initFramebuffer() void 
{
    const width = 1920;  // Desired framebuffer width
    const height = 1080; // Desired framebuffer height
    const depth = 32;    // Bits per pixel (color depth)

    @memset(&mbox, 0);

    var idx: u32 = 0;

    // Add tags to the mailbox buffer
    mbox[idx] = 0; // Placeholder for the total message size
    idx += 1;
    mbox[idx] = VIDEOCORE_REQUEST; // VideoCore request code
    idx += 1;

    addTag(&mbox, &idx, MBOX_TAGS.SETPHYWH, &.{width, height});
    addTag(&mbox, &idx, MBOX_TAGS.SETVIRTWH, &.{width, height});
    addTag(&mbox, &idx, MBOX_TAGS.SETVIRTOFF, &.{0, 0});
    addTag(&mbox, &idx, MBOX_TAGS.SETDEPTH, &.{depth});
    addTag(&mbox, &idx, MBOX_TAGS.SETPXLORDR, &.{1});
    addTag(&mbox, &idx, MBOX_TAGS.ALLOCFB, &.{4096, 0});
    addTag(&mbox, &idx, MBOX_TAGS.GETPITCH, &.{0});

    // Mark the end of the message
    mbox[idx] = @intFromEnum(MBOX_TAGS.LAST);
    idx += 1;

    // Set the total message size
    mbox[0] = idx * 4;

    // Send the mailbox message to the VideoCore
    writeToVCMailbox(@intFromPtr(&mbox));

    // Await VideoCore response
    const vcResponse = readFromVCMailbox();

    // Check the response
    if (vcResponse != @intFromPtr(&mbox) or mbox[1] != @intFromEnum(VIDEOCORE_RESPONSE_CODES.SUCCESS)) {
        log.ERROR("Framebuffer request failed: {}", .{vcResponse});
        return;
    }

    log.INFO("Framebuffer request succeeded\n", .{});

    // Process response from VideoCore
    if (findTag(&mbox, MBOX_TAGS.ALLOCFB)) |allocFB| {
        framebuffer = @ptrFromInt(allocFB[0] & 0x3FFFFFFF); // Convert GPU address to ARM address
    } else {
        log.ERROR("Failed to retrieve framebuffer address", .{});
        return;
    }

    framebuffer_width = findTag(&mbox, MBOX_TAGS.SETVIRTWH).?[0];
    framebuffer_height = findTag(&mbox, MBOX_TAGS.SETVIRTWH).?[1];
    framebuffer_pitch = findTag(&mbox, MBOX_TAGS.GETPITCH).?[0];
    framebuffer_isrgb = findTag(&mbox, MBOX_TAGS.SETPXLORDR).?[0];

    // Check for initialization errors
    if (mbox[28] == 0 or framebuffer_pitch == 0) {
        log.ERROR("Framebuffer initialization failed", .{});
        return;
    }

    log.INFO(
        "Framebuffer initialized at {x} with width {d}, height {d}, and pitch {d}\n",
        .{mbox[28], framebuffer_width, framebuffer_height, framebuffer_pitch}
    );
}


fn drawPixel(x: usize, y: usize, col: Color) void
{
    const offs = (y * framebuffer_pitch) + (x * 4);
    //const u32off = @as(u32, @intCast(offs));
    const pos : *volatile u32 = @ptrFromInt( @intFromPtr(framebuffer) + offs);
    pos.* = col.toUnified();
}

pub fn drawRect(x1: usize, y1: usize, x2: usize, y2: usize, col: Color) void
{
    for(y1..y2) |y|
    {
        for(x1..x2) |x|
        {
            drawPixel(x, y, col);
        }
    }
}