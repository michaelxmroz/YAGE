const std = @import("std");
const Builder = @import("std").Build;
const Target = @import("std").Target;
const CrossTarget = @import("std").zig.CrossTarget;
const Feature = @import("std").Target.Cpu.Feature;

pub fn build(b: *Builder) void {
    const target = b.resolveTargetQuery(.{
        .abi = .eabihf,
        .cpu_arch = .aarch64,
        .os_tag = std.Target.Os.Tag.freestanding,
        .cpu_model = std.Target.Query.CpuModel{ .explicit = &std.Target.arm.cpu.cortex_a72 },
    });

    const optimize = b.standardOptimizeOption(.{});

    const static_lib = b.addStaticLibrary(.{ .name = "staticlib", .target = target, .optimize = .ReleaseFast });
    static_lib.addIncludePath(b.path("..\\src\\YAGECore\\Include"));
    static_lib.addIncludePath(b.path("..\\src\\YAGECore\\Source"));

    static_lib.addCSourceFiles(.{ .files = &.{
        "..\\src\\YAGECore\\Source\\Allocator.cpp",
        "..\\src\\YAGECore\\Source\\APU.cpp",
        "..\\src\\YAGECore\\Source\\AudioChannel.cpp",
        "..\\src\\YAGECore\\Source\\CPU.cpp",
        "..\\src\\YAGECore\\Source\\Emulator.cpp",
        "..\\src\\YAGECore\\Source\\Emulator_C.cpp",
        "..\\src\\YAGECore\\Source\\Helpers.cpp",
        "..\\src\\YAGECore\\Source\\InstructionFunctions.cpp",
        "..\\src\\YAGECore\\Source\\Interrupts.cpp",
        "..\\src\\YAGECore\\Source\\Joypad.cpp",
        "..\\src\\YAGECore\\Source\\Logging.cpp",
        "..\\src\\YAGECore\\Source\\MBC.cpp",
        "..\\src\\YAGECore\\Source\\Memory.cpp",
        "..\\src\\YAGECore\\Source\\PixelFetcher.cpp",
        "..\\src\\YAGECore\\Source\\PixelFIFO.cpp",
        "..\\src\\YAGECore\\Source\\PPU.cpp",
        "..\\src\\YAGECore\\Source\\Registers.cpp",
        "..\\src\\YAGECore\\Source\\Serial.cpp",
        "..\\src\\YAGECore\\Source\\Serialization.cpp",
        "..\\src\\YAGECore\\Source\\Timer.cpp",
        "..\\src\\YAGECore\\Source\\VirtualMachine.cpp",
    }, .flags = &.{ "-std=c++14", "-ffreestanding", "-o2", "-fbuiltin", "--define-macro=FREESTANDING", "-fno-threadsafe-statics", "-fno-exceptions", "-fno-rtti" } });
    static_lib.root_module.single_threaded = true;
    //b.installArtifact(static_lib);

    var install_kernel = b.addInstallArtifact(static_lib, .{
        .dest_dir = .{ .override = .{ .custom = "..\\..\\bin\\ARM64\\Release\\" } },
    });

    const kernel = b.addExecutable(.{
        .name = "kernel.elf",
        .root_source_file = b.path("..\\src\\YAGEOS\\main.zig"),
        .target = target,
        .optimize = optimize,
    });

    kernel.setLinkerScript(b.path("..\\src\\YAGEOS\\linker.ld"));

    kernel.linkLibrary(static_lib);
    kernel.root_module.single_threaded = true;

    kernel.addIncludePath(b.path("..\\src\\YAGECore\\Include"));
    //kernel.addCMacro("FREESTANDING");

    install_kernel = b.addInstallArtifact(kernel, .{
        .dest_dir = .{ .override = .{ .custom = "..\\..\\bin\\ARM64\\Release\\" } },
    });

    const kernel_step = b.step("kernel", "Build the kernel");
    kernel_step.dependOn(&install_kernel.step);
}
