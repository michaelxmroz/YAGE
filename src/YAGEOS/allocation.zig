// Simple two-bucket allocator for emulator operation, plus a small linear buffer for run-time zig allocation

const log = @import("log.zig");
const utils = @import("utils.zig");
const std = @import("std");

extern const __heap_start: u8;

const BUCKET_SIZE = 0x2000000; // 32 mb for the emulator.
const BUCKET_COUNT = 2;
const SCRAP_BUFFER_SIZE = 0x1000000; // 16 mb for the scrapspace

const ALLOC_LOGS = false;

pub fn getHeapStart() usize {
    return @intFromPtr(&__heap_start);
}

pub fn getScrapBufferStart() usize {
    return getHeapStart() + BUCKET_COUNT * BUCKET_SIZE;
}

const Bucket = struct {
    start: u64,
    nextFree: u64,
    end: u64,
};

const AllocatorData = struct {
    currentActiveBucket: u64,
    buckets: [BUCKET_COUNT]Bucket,
    scrapNextFree: u64,
};

var allocatorData: AllocatorData = undefined;

pub fn init() void {
    const heapStart = getHeapStart();
    const scrapStart = getScrapBufferStart();

    allocatorData.currentActiveBucket = 0;
    allocatorData.scrapNextFree = scrapStart;

    for (&allocatorData.buckets, 0..) |*bucket, i| {
        bucket.start = heapStart + i * BUCKET_SIZE;
        bucket.nextFree = bucket.start;
        bucket.end = bucket.start + BUCKET_SIZE;
    }
}

pub fn activeBucketAlloc(size: u64) *u8 {
    const bucket = &allocatorData.buckets[allocatorData.currentActiveBucket];
    const retAddr = bucket.nextFree;

    bucket.nextFree += size;
    if (bucket.nextFree > bucket.end) {
        log.ERROR("Exceeded bucket capacity by {} bytes\n", .{bucket.nextFree - bucket.end});
        utils.hang();
    }

    if (ALLOC_LOGS) {
        log.INFO("Memory allocated: {} bytes at {} in bucket {}. Capacity left: {}\n", .{ size, retAddr, allocatorData.currentActiveBucket, (bucket.nextFree - bucket.end) });
        //log.INFO("Memory allocated: {} bytes at {} in bucket {}. Capacity left: {}\n", .{size, retAddr, allocatorData.currentActiveBucket, (bucket.nextFree - bucket.end) });
    }

    return @ptrFromInt(retAddr);
}

pub fn activeBucketFree() void {
    const bucket = &allocatorData.buckets[allocatorData.currentActiveBucket];
    bucket.nextFree = bucket.start;
}

pub fn scrapMemoryAlloc(size: u64) [*]u8 {
    const retAddr = allocatorData.scrapNextFree;

    allocatorData.scrapNextFree += size;
    if (allocatorData.scrapNextFree > getScrapBufferStart() + SCRAP_BUFFER_SIZE) {
        log.ERROR("Exceeded scrap space capacity by {} bytes\n", .{allocatorData.scrapNextFree - (getScrapBufferStart() + SCRAP_BUFFER_SIZE)});
        utils.hang();
    }

    return @ptrFromInt(retAddr);
}

pub fn getAllocator(size: u64) std.heap.FixedBufferAllocator {
    const mem = scrapMemoryAlloc(size);
    const slice: []u8 = mem[0..size];
    return std.heap.FixedBufferAllocator.init(slice);
}
