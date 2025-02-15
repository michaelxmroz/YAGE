const std = @import("std");
const Builder = @import("std").Build;
const Target = @import("std").Target;
const CrossTarget = @import("std").zig.CrossTarget;
const Feature = @import("std").Target.Cpu.Feature;

pub fn build(b: *Builder) void 
{
	const target = b.resolveTargetQuery(.{
        .abi = .eabihf,
        .cpu_arch = .aarch64,
        .os_tag = std.Target.Os.Tag.freestanding,
        .cpu_model = std.Target.Query.CpuModel{ .explicit = &std.Target.arm.cpu.cortex_a72 },
    });

	const optimize = b.standardOptimizeOption(.{});
	
	const static_lib = b.addStaticLibrary(.{ .name = "staticlib", .target = target, .optimize = optimize });
	static_lib.addIncludePath(b.path("..\\src\\YAGECore\\Include"));
	static_lib.addIncludePath(b.path("..\\src\\YAGECore\\Source"));

    static_lib.addCSourceFile(.{ .file = b.path("..\\src\\YAGECore\\Source\\Emulator_C.cpp"), .flags = &.{"-std=c++14", "-ffreestanding", "-o2", "-fbuiltin", "--define-macro=FREESTANDING"} });
	static_lib.root_module.single_threaded = true;	
	b.installArtifact(static_lib);
	
	const kernel = b.addExecutable(.{
	.name = "kernel.elf",
	.root_source_file = b.path("..\\src\\YAGEOS\\main.zig"),
	.target = target,
	.optimize = optimize,
    });

    kernel.setLinkerScript(b.path("..\\src\\YAGEOS\\linker.ld"));

	kernel.linkLibrary(static_lib);
	kernel.root_module.single_threaded = true;
	kernel.addIncludePath(b.path("."));
	
	const install_kernel = b.addInstallArtifact(kernel, .{
			.dest_dir = .{ .override = .{ .custom = "..\\..\\bin\\ARM64\\Debug\\" } },
        });

    const kernel_step = b.step("kernel", "Build the kernel");
    kernel_step.dependOn(&install_kernel.step);
	
}
