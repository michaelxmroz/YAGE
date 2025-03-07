// Logging helpers

const mmio = @import("mmio.zig");
const std = @import("std");

const LogLevel = enum {
    info,
    warning,
    err,
};

pub fn ERROR(comptime format: []const u8, args: anytype) void
{
    LOG(LogLevel.err, format, args);
}

pub fn WARNING(comptime format: []const u8, args: anytype) void
{
    LOG(LogLevel.warning, format, args);
}

pub fn INFO(comptime format: []const u8, args: anytype) void
{
    LOG(LogLevel.info, format, args);
}

pub fn LOG(level: LogLevel, comptime format: []const u8, args: anytype) void 
{
    var buffer: [4096]u8 = undefined;
    const prefix = switch (level) {
        .info => "INFO: ",
        .warning => "WARNING: ",
        .err => "ERROR: ",
    };
    mmio.uartSendString(prefix);

    const bufSlice = std.fmt.bufPrint(&buffer, format, args) catch 
    {
        mmio.uartSendString("Log formatting error\n");
        return;
    };
    mmio.uartSendString(bufSlice);
}