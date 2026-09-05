const std = @import("std");

pub fn main() !void {
    recalculateSpec("embrial/spec/format.txt") catch |e| switch (e) {
        error.FileNotFound => {
            std.debug.print("Warning: embrial/spec/format.txt not found\n", .{});
        },
        else => return e,
    };
//    const spec = try std.fs.cwd().openFile(
//        "Embrial/spec/format.txt",
//        .{ .mode = .read_only },
//    );
//    defer spec.close();
//
//    var buffer: [1024]u8 = undefined;
//    var spec_reader = spec.reader(&buffer);
//    var reader = &spec_reader.interface;
//
//    std.debug.print("POS: {}\n", .{try spec.getPos()});
//
//    while (try reader.takeDelimiter('\n')) |line| {
//        std.debug.print("LINE [{d}]: [{s}]\n", .{
//            line.len,
//            line,
//        });
//    }

    std.debug.print("DONE\n", .{});
}
const DirectiveKind = enum {
    NONE,
    BLOCK,
    INCLUDE,
    KEYWORDS
};
const Directive = struct {
    kind: DirectiveKind,
    line: usize,
    indent: usize,
    filepath: []const u8,
};
fn asciiLessThan(_: void, lhs: []const u8, rhs: []const u8) bool {
    return std.mem.order(u8, lhs, rhs) == .lt;
}

fn recalculateSpec(directory: []const u8) !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();

    // create an arena
    var arena = std.heap.ArenaAllocator.init(gpa.allocator());
    defer arena.deinit();

    const allocator = arena.allocator();

    var directives: std.ArrayList(Directive) = .empty;
    var keywords: std.ArrayList([]const u8) = .empty;

    // parse the spec file
    const spec = try std.fs.cwd().openFile(directory, .{.mode = .read_only});
    defer spec.close();

    var buffer: [1024]u8 = undefined;
    var lineNumber: usize = 0;
    var specReader = spec.reader(&buffer);
    var reader = &specReader.interface;

    while (reader.takeDelimiter('\n')) |maybeLine| {
        const line = maybeLine orelse break;
        lineNumber += 1;
        const noCharachterReturn = std.mem.trimRight(u8, line, "\r");
        const cleanLine = std.mem.trimLeft(u8, noCharachterReturn, " \t");

        if (std.mem.startsWith(u8, cleanLine, "#keyword")) {
            const payload = std.mem.trimLeft(u8, cleanLine[8..], " \t");

            const permanentCopy = try allocator.dupe(u8, payload);
            try keywords.append(arena.allocator(), permanentCopy);

            _ = try reader.takeByte();
            continue;
        }

        const originalAddress = @intFromPtr(noCharachterReturn.ptr);
        const trimmedAddress = @intFromPtr(cleanLine.ptr);
        var directive: Directive = .{
            .kind = .NONE,
            .line = lineNumber,
            .filepath = "",
            .indent = trimmedAddress - originalAddress,
        };
        if (std.mem.startsWith(u8, cleanLine, "#block")) {
            directive.kind = .BLOCK;
            directive.filepath = cleanLine[6..];
        } else if (std.mem.startsWith(u8, cleanLine, "#include")) {
            directive.kind = .INCLUDE;
            directive.filepath = cleanLine[8..];
        } else if (std.mem.startsWith(u8, cleanLine, "#keywords")) {
            directive.kind = .KEYWORDS;
            directive.filepath = cleanLine[9..];
        } else continue;

        std.debug.print("DETECTED   DIRECTIVE on line {}: [{s}]\n", .{lineNumber, line});
        const filepath = std.mem.trimLeft(u8, directive.filepath, " \t");
        directive.filepath = try allocator.dupe(u8, filepath);

        try directives.append(arena.allocator(), directive);
        _ = try reader.takeByte();
    } else |err| {
        if (err != error.EndOfStream) return err;
    }

    std.mem.sort([]const u8, keywords.items, {}, asciiLessThan);

    const keywords_len = keywords.items.len;
    var columnCount = std.math.sqrt(keywords_len);

    var lineCount = if (columnCount > 0) keywords_len / columnCount else 0;
    var totalWidth: usize = 0;

    var columns: []usize = try allocator.alloc(usize, columnCount);

    while (columnCount > 0) {
        totalWidth = 4;
        @memset(columns, 0);
        for (0..columnCount) |i| {
            const offset = lineCount * i;
            const elementsInColumn = keywords_len - offset;
            var widestInColumn: usize = 0;

            // Ensure we don't read past lineCount for this column
            const loopLimit = @min(lineCount, elementsInColumn);

            for (0..loopLimit) |j| {
                const padding = if (i != columnCount - 1) @as(usize, 2) else 0;
                const keywordLen = keywords.items[offset + j].len + padding;

                if (keywordLen > widestInColumn) widestInColumn = keywordLen;
            }
            columns[i] = widestInColumn;
            totalWidth += widestInColumn;
        }
        if (totalWidth < 80) break;
        columnCount -= 1;
        if (columnCount > 0) lineCount = (keywords_len + columnCount - 1) / columnCount;
    }

    const wordSquareBufferCapacity = totalWidth * lineCount + lineCount;
    if (columnCount > 0) {
        for (1..columns.len) |i| {
            columns[i] += columns[i - 1] + 2;
        }
    }

    var wordSquareBuffer: []u8 = try allocator.alloc(u8, wordSquareBufferCapacity);
    @memset(wordSquareBuffer, 0);

    if (columnCount > 0) {
        for (0..columnCount) |i| {
            const offset = lineCount * i;
            const elementsInColumn = keywords_len - offset;
            const col = columns[i];

            // Ensure we don't read past lineCount for this column
            const loopLimit = @min(lineCount, elementsInColumn);

            for (0..loopLimit) |j| {
                const keyword = keywords.items[offset + j];
                const index = totalWidth * j + col;
                const destSlice = wordSquareBuffer[index .. index + keyword.len];
                @memcpy(destSlice, keyword);
                wordSquareBuffer[totalWidth * j] = '\n';
            }
        }
    }

    // output the result of processing all the directives
    try spec.seekTo(0);

    specReader = spec.reader(&buffer);
    reader = &specReader.interface;

    const manual = try std.fs.cwd().createFile("embrial/manual.txt", .{});
    defer manual.close();

    var manualFileWriterIdk = manual.writer(&.{});
    const manualFileWriter = &manualFileWriterIdk.interface;

    lineNumber = 0;
    var currentDirective: usize = 0;
    var directive = directives.items[0];

    var sourceFile: ?std.fs.File = null;
    var expectingBlockDirective = false;
    var codeBlockLine: usize = undefined;
    var padding: usize = undefined;
    var sourceBuffer: [1024]u8 = undefined;
    var sourceFileReader: std.fs.File.Reader = undefined;
    var sourceReader: *std.Io.Reader = undefined;

    defer if (sourceFile) |f| f.close();

    while (reader.takeDelimiter('\n')) |maybeLine| {
        const line = maybeLine orelse break;
        lineNumber += 1;
        if (currentDirective >= directives.items.len or directive.line != lineNumber) {
            try manualFileWriter.writeAll(line);
            try manualFileWriter.writeAll("\n");
            continue;
        }
        std.debug.print("PROCESSING DIRECTIVE on line {}: [{s}]\n", .{lineNumber, line});

        //std.debug.print("Processing directive: '{s}'\n", .{std.mem.trimLeft(u8, std.mem.trimRight(u8, line, "\r"), " \t")});
        switch (directive.kind) {
            .KEYWORDS => {
                try manualFileWriter.writeAll(wordSquareBuffer);
            },
            .INCLUDE => {
                sourceFile = std.fs.cwd().openFile(
                    directive.filepath,
                    .{.mode = .read_only}
                ) catch |err| switch(err) {
                    error.FileNotFound => {
                        std.debug.print("Warning: #include failed, File not found: {s}", .{directive.filepath});
                        continue;
                    },
                    else => return err,
                };

                // try manualFileWriter.writeFile(sourceFile);
                _ = try sourceReader.streamRemaining(manualFileWriter);
            },
            .BLOCK => {
                if (directive.filepath.len != 0) {
                    codeBlockLine = 1;
                    var codeBlockLineCount: usize = 0;

                    if (sourceFile) |f| {
                        f.close();
                        sourceFile = null;
                    }

                    sourceFile = std.fs.cwd().openFile(
                        directive.filepath,
                        .{.mode = .read_only}
                    ) catch |err| switch(err) {
                        error.FileNotFound => {
                            std.debug.print("Warning: #block failed, File not found: {s}", .{directive.filepath});
                            continue;
                        },
                        else => return err,
                    };
                    sourceFileReader = sourceFile.?.reader(&sourceBuffer);
                    sourceReader = &sourceFileReader.interface;

                    while (sourceReader.takeDelimiter('\n')) |maybeBlockLine| {
                        if (maybeBlockLine == null) break;
                        codeBlockLineCount += 1;
                    } else |err| {
                        if (err != error.EndOfStream) return err;
                    }

                    try sourceFile.?.seekTo(0);
                    sourceFileReader = sourceFile.?.reader(&sourceBuffer);
                    sourceReader = &sourceFileReader.interface;

                    const width = if (codeBlockLineCount == 0) 1 else std.math.log10(codeBlockLineCount);
                    padding = ((width / 4) + 1) * 4 + directive.indent;

                    try manualFileWriter.print("    {s}:\n", .{directive.filepath});
                } else codeBlockLine += 1;
                while (sourceReader.takeDelimiter('\n')) |maybeSourceLine| {
                    const sourceLine = maybeSourceLine orelse break;
                    defer codeBlockLine += 1;
                    const noCharachterReturn = std.mem.trimRight(u8, sourceLine, "\r");
                    const cleanLine = std.mem.trimLeft(u8, noCharachterReturn, " \t");

                    if (std.mem.startsWith(u8, cleanLine, "#comment")) {
                        expectingBlockDirective = true;
                        break;
                    }

                    const maxTextWidth = 80 - (padding + 4);
                    try manualFileWriter.print("{d: >[1]} |  ", .{
                        codeBlockLine,
                        padding
                    });
                    //try manualFileWriter.print("{:>} |  ", .{ codeBlockLine, padding });

                    var currentLineLen: usize = 0;
                    var index: usize = 0;

                    while (index < noCharachterReturn.len) {
                        var remaining = noCharachterReturn[index..];

                        var cursor: usize = 0;

                        while (cursor < remaining.len and (remaining[cursor] == ' ' or  remaining[cursor] == '\t')) {
                            // does the whitespace exceed the maxTextWidth?
                            if (currentLineLen + 1 > maxTextWidth or
                                (remaining[cursor] == '\t' and currentLineLen + 4 > maxTextWidth)
                            ) {
                                // it does so print the next lines prefix
                                try manualFileWriter.writeByte('\n');
                                for (0..padding) |_| {
                                    try manualFileWriter.writeByte(' ');
                                }
                                try manualFileWriter.writeAll("    ");
                                currentLineLen = 0;

                                // skip until the next word
                                while (cursor < remaining.len and
                                    (remaining[cursor] == ' ' or remaining[cursor] == '\t')) {
                                    cursor += 1;
                                }
                                break;
                            }

                            // the whitespace does not exceed the maxTextWidth

                            // print the whitespace, converting \t into 4 spaces
                            if (remaining[cursor] == '\t') {
                                try manualFileWriter.writeAll("    ");
                                currentLineLen += 4;
                            } else {
                                try manualFileWriter.writeByte(remaining[cursor]);
                                currentLineLen += 1;
                            }

                            cursor += 1;
                        }

                        if (cursor >= remaining.len) break;
                        index += cursor;

                        remaining = remaining[cursor..];
                        cursor = 0;

                        // the cursor is at the start of a word, determine the length of the word
                        while (cursor < remaining.len and remaining[cursor] != ' ' and remaining[cursor] != ' ') {
                            cursor += 1;
                        }

                        if (currentLineLen + cursor > maxTextWidth) {
                            try manualFileWriter.writeByte('\n');
                            for (0..padding) |_| {
                                try manualFileWriter.writeByte(' ');
                            }
                            try manualFileWriter.writeAll("    ");
                            currentLineLen = 0;
                        }

                        const token = remaining[0..cursor];
                        try manualFileWriter.writeAll(token);
                        currentLineLen += token.len;
                        index += token.len;
                    }

                    try manualFileWriter.writeByte('\n');
                } else |err| {
                    if (err != error.EndOfStream) return err;
                }
                try manualFileWriter.writeByte('\n');
            },
            else => unreachable,
        }

        currentDirective += 1;
        if (currentDirective < directives.items.len) {
            directive = directives.items[currentDirective];
        }
        _ = try reader.takeByte();
    } else |err| {
        if (err != error.EndOfStream) return err;
    }

    try manualFileWriter.flush();
}
