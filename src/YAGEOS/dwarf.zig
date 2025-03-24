const std = @import("std");
const log = @import("log.zig");

extern const __debug_info_start: u8;
extern const __debug_info_end: u8;
extern const __debug_abbrev_start: u8;
extern const __debug_abbrev_end: u8;
extern const __debug_str_start: u8;
extern const __debug_str_end: u8;
extern const __debug_line_start: u8;
extern const __debug_line_end: u8;
extern const __debug_ranges_start: u8;
extern const __debug_ranges_end: u8;

pub fn getDwarfSections() DWARFParser {
    log.INFO("Getting DWARF sections...\n", .{});
    const debug_info = @as([*]const u8, &__debug_info_start)[0 .. @intFromPtr(&__debug_info_end) - @intFromPtr(&__debug_info_start)];
    const debug_abbrev = @as([*]const u8, &__debug_abbrev_start)[0 .. @intFromPtr(&__debug_abbrev_end) - @intFromPtr(&__debug_abbrev_start)];
    const debug_str = @as([*]const u8, &__debug_str_start)[0 .. @intFromPtr(&__debug_str_end) - @intFromPtr(&__debug_str_start)];
    const debug_line = @as([*]const u8, &__debug_line_start)[0 .. @intFromPtr(&__debug_line_end) - @intFromPtr(&__debug_line_start)];
    const debug_ranges = @as([*]const u8, &__debug_ranges_start)[0 .. @intFromPtr(&__debug_ranges_end) - @intFromPtr(&__debug_ranges_start)];

    log.INFO("DWARF sections loaded - info:{} bytes, abbrev:{} bytes, str:{} bytes, line:{} bytes, ranges:{} bytes\n", .{ debug_info.len, debug_abbrev.len, debug_str.len, debug_line.len, debug_ranges.len });

    return DWARFParser.init(debug_info, debug_abbrev, debug_str, debug_line, debug_ranges);
}

const DWARFParser = struct {
    const Self = @This();

    debug_info: []const u8,
    debug_abbrev: []const u8,
    debug_str: []const u8,
    debug_line: []const u8,
    debug_ranges: []const u8,

    pub fn init(
        debug_info: []const u8,
        debug_abbrev: []const u8,
        debug_str: []const u8,
        debug_line: []const u8,
        debug_ranges: []const u8,
    ) DWARFParser {
        return .{
            .debug_info = debug_info,
            .debug_abbrev = debug_abbrev,
            .debug_str = debug_str,
            .debug_line = debug_line,
            .debug_ranges = debug_ranges,
        };
    }

    pub fn parse(self: *Self) !void {
        log.INFO("Starting DWARF parsing...\n", .{});
        var info_stream = std.io.fixedBufferStream(self.debug_info);
        const reader = info_stream.reader();

        while (info_stream.pos < self.debug_info.len) {
            log.INFO("Parsing compilation unit at offset {}\n", .{info_stream.pos});
            try self.parseCompilationUnit(reader);
        }

        log.INFO("DWARF parsing completed\n", .{});
    }

    fn parseCompilationUnit(self: *Self, reader: anytype) !void {
        const unit_length = try reader.readInt(u32, .little);
        const version = try reader.readInt(u16, .little);
        const abbrev_offset = try reader.readInt(u32, .little);
        const address_size = try reader.readByte();

        log.INFO("Compilation unit: length={}, version={}, abbrev_offset={}, addr_size={}\n", .{ unit_length, version, abbrev_offset, address_size });

        if (version != 4) return error.UnsupportedDWARFVersion;

        // Track end of unit
        const unit_end: usize = reader.context.pos + unit_length - 7; // -7 = version(2) + offset(4) + addr_size(1)

        while (reader.context.pos < unit_end) {
            const abbrev_code = try reader.readULEB128();
            if (abbrev_code == 0) continue;

            log.INFO("DIE with abbrev_code={}\n", .{abbrev_code});
            // Lookup abbrev and decode DIE attributes here
            try self.handleDIEStub(abbrev_code);
        }
    }

    fn handleDIEStub(self: *Self, abbrev_code: u64) !void {
        // Placeholder: parse DIE attributes using abbrev_code later
        log.INFO("Handling DIE with abbrev_code={}\n", .{abbrev_code});
        // Don't discard parameters since they will be used in the future implementation
    }
};

// Test functions for kernel panic callstack testing
fn level3Function() void {
    log.INFO("In level3Function, about to trigger a panic\n", .{});
    @panic("Test kernel panic from level3Function");
}

fn level2Function() void {
    log.INFO("In level2Function, calling level3Function\n", .{});
    level3Function();
}

fn level1Function() void {
    log.INFO("In level1Function, calling level2Function\n", .{});
    level2Function();
}

pub fn testKernelPanic() void {
    log.INFO("Starting kernel panic test with nested calls\n", .{});
    level1Function();
}
