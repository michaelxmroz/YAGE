// Simple two-bucket allocator for emulator operation, plus a small linear buffer for run-time zig allocation

const log = @import("log.zig");
const utils = @import("utils.zig");
const std = @import("std");

const HEAP_START = 0x1000000; // 16 mb for the kernel + stack
const BUCKET_SIZE = 0x2000000; // 32 mb for the emulator.
const BUCKET_COUNT = 2;
const SCRAP_BUFFER_START = HEAP_START + BUCKET_COUNT * BUCKET_SIZE;
const SCRAP_BUFFER_SIZE = 0x1000000; // 16 mb for the scrapspace
const SCRAP_BUFFER_END = SCRAP_BUFFER_START + SCRAP_BUFFER_SIZE;

const ALLOC_LOGS = false;

const Bucket = struct
{
    start : u64,
    nextFree : u64,
    end : u64,
};

const AllocatorData = struct
{
    currentActiveBucket: u64,
    buckets : [BUCKET_COUNT]Bucket,
    scrapNextFree: u64,
};

var allocatorData = AllocatorData
{
    .currentActiveBucket = 0,
    .buckets = blk: {
        var temp: [BUCKET_COUNT]Bucket = undefined;
        for (&temp, 0..) |*value, i| {
            value.start = HEAP_START + i * BUCKET_SIZE;
            value.nextFree = value.start;
            value.end = HEAP_START + i * BUCKET_SIZE + BUCKET_SIZE;
        }
        break :blk temp;
    },
    .scrapNextFree = SCRAP_BUFFER_START,
};

pub fn activeBucketAlloc(size : u64) *u8
{
    const bucket = &allocatorData.buckets[allocatorData.currentActiveBucket];
    const retAddr = bucket.nextFree;
    
    bucket.nextFree += size;
    if(bucket.nextFree > bucket.end)
    {
        log.ERROR("Exceeded bucket capacity by {} bytes\n", .{ bucket.nextFree - bucket.end });
        utils.hang();
    }
    
    if(ALLOC_LOGS)
    {
        log.INFO("Memory allocated: {} bytes at {} in bucket {}. Capacity left: {}\n", .{size, retAddr, allocatorData.currentActiveBucket, (bucket.nextFree - bucket.end) });
        //log.INFO("Memory allocated: {} bytes at {} in bucket {}. Capacity left: {}\n", .{size, retAddr, allocatorData.currentActiveBucket, (bucket.nextFree - bucket.end) });
    }
    
    return @ptrFromInt(retAddr);
}

pub fn activeBucketFree() void
{
    const bucket = &allocatorData.buckets[allocatorData.currentActiveBucket];
    bucket.nextFree = bucket.start;
}

pub fn scrapMemoryAlloc(size : u64) [*]u8
{
    const retAddr = allocatorData.scrapNextFree;
    
    allocatorData.scrapNextFree += size;
    if(allocatorData.scrapNextFree > SCRAP_BUFFER_END)
    {
        log.ERROR("Exceeded scrap space capacity by {} bytes\n", .{ allocatorData.scrapNextFree - SCRAP_BUFFER_END });
        utils.hang();
    }

    return @ptrFromInt(retAddr);
}

pub fn getAllocator(size : u64) std.heap.FixedBufferAllocator
{
    const mem = scrapMemoryAlloc(size);
    const slice  : []u8 = mem[0..size];
    return std.heap.FixedBufferAllocator.init(slice);
}