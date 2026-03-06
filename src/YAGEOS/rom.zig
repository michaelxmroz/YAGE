const std = @import("std");

const embeddedrom = @import("romfile");

pub fn getRom() [*c]const u8 {
    return @ptrCast(embeddedrom.rombinary);
}

pub fn getRomSize() u32 {
    return embeddedrom.rombinary.len;
}
