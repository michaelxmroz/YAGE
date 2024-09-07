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
    mmio.uartSendString("Test!!!\n");
    var buffer: [256]u8 = undefined;
    const prefix = switch (level) {
        .info => "INFO: ",
        .warning => "WARNING: ",
        .err => "ERROR: ",
    };
    mmio.uartSendString(prefix);

    _ = std.fmt.bufPrint(&buffer, "{s}{s}", .{"b", "a"}) catch 
    {
        mmio.uartSendString("Log formatting error\n");
        return;
    };
    mmio.uartSendString("Test2!!!\n");
    mmio.uartSendString(&buffer);
    _ = std.fmt.bufPrint(&buffer, format, args) catch 
    {
        mmio.uartSendString("Log formatting error\n");
        return;
    };
    mmio.uartSendString(&buffer);
}