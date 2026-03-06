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

    const allocator = b.allocator;

    const common_path_core = "../../src/YAGECore";
    const common_path_kernel = "../../src/YAGEOS";
    const common_path_core_source = std.fs.path.join(allocator, &.{ common_path_core, "Source" }) catch unreachable;
    const common_path_core_include = std.fs.path.join(allocator, &.{ common_path_core, "Include" }) catch unreachable;

    const static_lib = b.addLibrary(.{ .linkage = .static, .name = "staticlib", .root_module = b.createModule(.{
            .target = target,
            .optimize = .ReleaseFast,
        }),});
    static_lib.addIncludePath(b.path(common_path_core_include));
    static_lib.addIncludePath(b.path(common_path_core_source));

    var fileList = std.ArrayList([]const u8).initCapacity(b.allocator, 0) catch unreachable;
    var dir = std.fs.cwd().openDir(common_path_core_source, .{ .iterate = true }) catch unreachable;
    defer dir.close();

    var iter = dir.iterate();
    while (iter.next() catch unreachable) |entry| {
        if (entry.kind == .file) {
            if (std.mem.endsWith(u8, entry.name, ".cpp")) {
                const abs_path = std.fs.path.join(allocator, &[_][]const u8{ common_path_core_source, entry.name }) catch unreachable;
                fileList.append(b.allocator, abs_path) catch unreachable;
            }
        }
    }

    static_lib.addCSourceFiles(.{ .files = fileList.toOwnedSlice(b.allocator) catch unreachable, .flags = &.{ "-std=c++14", "-ffreestanding", "-o2", "-fbuiltin", "--define-macro=FREESTANDING","--define-macro=_LOGGING", "-fno-threadsafe-statics", "-fno-exceptions", "-fno-rtti" } });
    static_lib.root_module.single_threaded = true;
    //b.installArtifact(static_lib);

    var install_kernel = b.addInstallArtifact(static_lib, .{
        .dest_dir = .{ .override = .{ .custom = "../../../bin/ARM64/Release/" } },
    });

    const kernel = b.addExecutable(.{
        .name = "kernel.elf",
		.root_module = b.createModule(.{
            .root_source_file = b.path(std.fs.path.join(allocator, &.{ common_path_kernel, "main.zig" }) catch unreachable),
            .target = target,
            .optimize = optimize,
        }),
    });

    kernel.setLinkerScript(b.path(std.fs.path.join(allocator, &.{ common_path_kernel, "linker.ld" }) catch unreachable));

    kernel.linkLibrary(static_lib);
    kernel.root_module.single_threaded = true;

    kernel.addIncludePath(b.path(common_path_core_include));
	
	const rom_path = b.option([]const u8, "rom", "ROM path") orelse "../../splash.gb";

	const options = b.addOptions();
    options.addOption([]const u8, "rom_path", rom_path);

    kernel.root_module.addOptions("build_options", options);
	
	const gen = b.addWriteFiles();
	
	const rom_filename = "embeddedrom.gb";
	
	_ = gen.addCopyFile(.{ .cwd_relative = rom_path }, rom_filename);

    const rom_zig = gen.add("romfile.zig",
        b.fmt(
            \\pub const rombinary : []const u8 = @embedFile("{s}");
        , .{rom_filename})
    );

    const rom_mod = b.createModule(.{
        .root_source_file = rom_zig,
    });

    kernel.root_module.addImport("romfile", rom_mod);
	
	//kernel.step.dependOn(&copy_rom.step);

    install_kernel = b.addInstallArtifact(kernel, .{
        .dest_dir = .{ .override = .{ .custom = "../../../bin/ARM64/Release/" } },
    });

    const kernel_step = b.step("kernel", "Build the kernel");
    kernel_step.dependOn(&install_kernel.step);
}
