const std = @import("std");
const Builder = @import("std").Build;
const Target = @import("std").Target;
const CrossTarget = @import("std").zig.CrossTarget;
const Feature = @import("std").Target.Cpu.Feature;
 
pub fn build(b: *Builder) void {
    //const features = Target.aarch64.Feature;
 
    //const disabled_features = Feature.Set.empty;
    //const enabled_features = Feature.Set.empty;
 
    //disabled_features.addFeature(features.mmx);
    //disabled_features.addFeature(features.sse);
    //disabled_features.addFeature(features.sse2);
    //disabled_features.addFeature(features.avx);
    //disabled_features.addFeature(features.avx2);
    //enabled_features.addFeature(features.soft_float);
 
	const target = b.resolveTargetQuery(.{
        .abi = .eabihf,
        .cpu_arch = .aarch64,
        .os_tag = std.Target.Os.Tag.freestanding,
        .cpu_model = std.Target.Query.CpuModel{ .explicit = &std.Target.arm.cpu.cortex_a72 },
    });
 
    //const mode = b.standardReleaseOptions();
 
    //const kernel = b.addExecutable("kernel.elf", "..\\src\\YAGEOS\\main.zig");
	
	const optimize = b.standardOptimizeOption(.{});
	
	const kernel = b.addExecutable(.{
	.name = "kernel.elf",
	.root_source_file = b.path("..\\src\\YAGEOS\\main.zig"),
	.target = target,
	.optimize = optimize,
    });
	
    //kernel.setTarget(target);
    //kernel.setBuildMode(mode);
    kernel.setLinkerScript(b.path("..\\src\\YAGEOS\\linker.ld"));
    //kernel.code_model = .kernel;
    //kernel.installArtifact();
	
	//const kernel_path_nofile = "C:\\Projects\\GameBoy\\bin\\ARM64\\Debug\\";
    //const kernel_path = b.getInstallPath(kernel_path_nofile, kernel.out_filename);
	
	    // Define a custom installation directory
    //const custom_install_dir = Builder.CustomInstallDir{
    //    .path = "C:\\Projects\\GameBoy\\bin\\ARM64\\Debug\\",
    //};
	
	const install_kernel = b.addInstallArtifact(kernel, .{
			.dest_dir = .{ .override = .{ .custom = "..\\..\\bin\\ARM64\\Debug\\" } },
            //.dest_dir =  { {b.path("..\\bin\\ARM64\\Debug\\");};},
        });
		
	//const wf = b.addWriteFiles();
    // Copies to a path relative to the package root.
    // Be careful with this because it updates source files. This should not be
    // used as part of the normal build process, but as a utility occasionally
    // run by a developer with intent to modify source files and then commit
    // those changes to version control.
    //wf.addCopyFileToSource(install_kernel.step.getOutput(), "C:\\Projects\\GameBoy\\bin\\ARM64\\Debug\\");
    //b.getInstallStep().dependOn(&install_kernel.step);
 
    const kernel_step = b.step("kernel", "Build the kernel");
    kernel_step.dependOn(&install_kernel.step);
 

}