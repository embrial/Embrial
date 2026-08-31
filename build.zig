const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{
        .default_target = .{
            .cpu_arch = .x86_64,
            .os_tag = .linux,
            .abi = .gnu,
            .cpu_features_add = std.Target.x86.featureSet(&.{
                .bmi2,
            }),
        },
    });
    const optimize = b.standardOptimizeOption(.{});

    // Compilers:
    const exe = b.addExecutable(.{
        .name = "compiler",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
        }),
    });

    exe.linkLibC();

    exe.addCSourceFiles(.{
        .files = &.{
            "src/main.c",
            "src/simd.c",
        },
        .flags = &.{
            "-Wall",
            "-Wextra",
            "-std=gnu99",
        },
    });

    exe.addIncludePath(b.path("include"));
    b.installArtifact(exe);

    // Tests:
    const tests = b.addExecutable(.{
        .name = "compilerTests",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
        }),
    });

    tests.linkLibC();

    tests.addCSourceFiles(.{
        .files = &.{
            "tests/main.c"
        },
        .flags = &.{
            "-Wall",
            "-Wextra",
            "-std=gnu99",
        },
    });

    tests.addIncludePath(b.path("include"));

    // Zig steps:

    const run_step = b.step("run", "Run the app");

    const run_cmd = b.addRunArtifact(exe);
    run_step.dependOn(&run_cmd.step);

    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

   const test_step = b.step("test", "Run all tests");

   const test_cmd = b.addRunArtifact(tests);
   test_step.dependOn(&test_cmd.step);
}
