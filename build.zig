const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "CityBuilder",
        .target = target,
        .optimize = optimize,
    });
    exe.addIncludePath(.{ .path = "include" });
    exe.addIncludePath(.{ .path = "libs/include" });
    exe.addCSourceFiles(source_files, flags);
    exe.linkLibCpp();
    exe.linkSystemLibrary("shlwapi");
    exe.linkSystemLibrary("gdi32");
    exe.linkSystemLibrary("ole32");

    b.installArtifact(exe);

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }
    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}

const source_files = &[_][]const u8 {
    "src/Game.cpp",
    "src/Engine.cpp",
    "src/Graphics.cpp",
    "src/GUI.cpp",
    "src/Image.cpp",
    "src/Input.cpp",
    "src/Timer.cpp",
    "src/Audio.cpp",
    "src/Tile.cpp",
    "src/Grid.cpp",
    "src/Color.cpp",
    "src/Random.cpp",
    "src/UnitManager.cpp",
    "src/Serialization.cpp",
    "src/Platform.cpp",

    "src/imgui_draw.cpp",
    "src/imgui_tables.cpp",
    "src/imgui_widgets.cpp",
    "src/imgui.cpp",
};

const flags = &[_][]const u8 {
    "-std=c++20",
    "-DBAT_RUN",
    "-Wall",
    "-Wextra",
    "-Wno-c99-designator",
    "-Wno-reorder-init-list",
    "-Wno-microsoft-enum-forward-reference",
    "-Wno-unused-parameter",
    "-Wno-unused-variable",
    "-Wno-missing-field-initializers",
    "-Wno-switch",
    "-Wno-logical-op-parentheses",
    "-Wno-deprecated-declarations",
    "-Wno-missing-braces",
};
