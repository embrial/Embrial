import math

with open("keywords.txt", "r") as file:
    lines = file.readlines()

lines = sorted([line.strip().lower() for line in lines if line.strip()])
sideLen = math.ceil(math.sqrt(len(lines)))

with open("keywords.txt", "w") as file:
    file.writelines(f"{line}\n" for line in lines)

columns = [lines[i: i + sideLen] for i in range(0, len(lines), sideLen)]

columnWidths = []
for i in range(len(columns)):
    longestInCol = 0
    for j in range(len(columns[i])):
        cLen = len(columns[i][j])
        # 4. Corrected logic assignment (longestInCol = cLen)
        if cLen > longestInCol:
        longestInCol = cLen
    columnWidths.append(longestInCol + 2)

for j in range(sideLen):
    for i in range(len(columns)):
        if j >= len(columns[i]):
            continue
    print(f"{columns[i][j]:<{columnWidths[i]}}", end="")
print()

const std = @import("std");

fn asciiLessThan(_: void, lhs: []const u8, rhs: []const u8) bool {
    return std.mem.order(u8, lhs, rhs) == .lt;
}

pub fn main() !void {
    // Legacy managed style setup assumed from your snippet
    var keywords = std.ArrayList([]const u8).init(allocator);
    defer keywords.deinit();

    // Sort the internal slice in-place
    std.mem.sort([]const u8, keywords.items, {}, asciiLessThan);
}

