const std = @import("std");

pub fn main(init: std.process.Init) !void {
    recalculateSpec(init.io, "spec/format.txt") catch |e| switch (e) {
        error.FileNotFound => {
            std.debug.print("Warning: spec/format.txt not found: {}\n", .{e});
        },
        else => return e,
    };

    std.debug.print("DONE\n", .{});
}
const DirectiveKind = enum {
    NONE,
    BLOCK,
    INCLUDE,
    KEYWORDS,
    KEYWORD,
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

fn recalculateSpec(io: std.Io, directory: []const u8) !void {
    // create an arena
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();

    const allocator = arena.allocator();

    var directives: std.ArrayList(Directive) = .empty;
    var keywords: std.ArrayList([]const u8) = .empty;

    // parse the spec file
    const cwd = std.Io.Dir.cwd();
    const spec = try cwd.openFile(io, directory, .{.mode = .read_only});
    defer spec.close(io);

    var buffer: [1024]u8 = undefined;
    var lineNumber: usize = 0;
    var specReader = spec.reader(io, &buffer);
    var reader = &specReader.interface;

    // discover directives:
    while (reader.takeDelimiter('\n')) |maybeLine| {
        const line = maybeLine orelse break;
        lineNumber += 1;
        const noCharachterReturn = std.mem.trimEnd(u8, line, "\r");
        const cleanLine = std.mem.trimStart(u8, noCharachterReturn, " \t");

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
        } else if (std.mem.startsWith(u8, cleanLine, "#keyword")) {
            std.debug.print("DETECTED   DIRECTIVE on line {d: >8}: {s}\n", .{lineNumber, cleanLine});

            const permanentCopy = try allocator.dupe(u8, cleanLine[9..]);
            try keywords.append(arena.allocator(), permanentCopy);

            directive.kind = .KEYWORD;
            try directives.append(arena.allocator(), directive);
            continue;
        } else continue;

        std.debug.print("DETECTED   DIRECTIVE on line {d: >8}: {s}\n", .{lineNumber, cleanLine});
        const filepath = std.mem.trimStart(u8, directive.filepath, " \t");
        directive.filepath = try allocator.dupe(u8, filepath);

        try directives.append(arena.allocator(), directive);
        _ = try reader.takeByte();
    } else |err| {
        if (err != error.EndOfStream) return err;
    }

    // calculate word square for keywords:
    std.mem.sort([]const u8, keywords.items, {}, asciiLessThan);

    const keywords_len = keywords.items.len;
    var columnCount = std.math.sqrt(keywords_len);

    var lineCount = if (columnCount > 0) (keywords_len + columnCount - 1) / columnCount else 0;
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
            columns[i] = totalWidth;
            totalWidth += widestInColumn;
        }
        if (totalWidth < 80) break;
        columnCount -= 1;
        if (columnCount > 0) lineCount = (keywords_len + columnCount - 1) / columnCount;
    }

    const rowWidth = totalWidth;
    const rowStride = rowWidth + 1;
    const wordSquareBufferCapacity = rowStride * lineCount;

    var wordSquareBuffer: []u8 = try allocator.alloc(u8, wordSquareBufferCapacity);
    @memset(wordSquareBuffer, 0x20);

    if (columnCount > 0) {
        for (0..lineCount) |i| {
            wordSquareBuffer[i * rowStride + rowWidth] = '\n';
        }
        for (0..columnCount) |i| {
            const offset = lineCount * i;
            const elementsInColumn = keywords_len - offset;
            const col = columns[i];

            // Ensure we don't read past lineCount for this column
            const loopLimit = @min(lineCount, elementsInColumn);

            for (0..loopLimit) |j| {
                const keyword = keywords.items[offset + j];
                const index = rowStride * j + col;
                const destSlice = wordSquareBuffer[index .. index + keyword.len];
                @memcpy(destSlice, keyword);
            }
        }
    }

    std.debug.print("GENERATED KEYWORD TABLE:\n{s}", .{wordSquareBuffer});

    // output the result of processing all the directives
    specReader = spec.reader(io, &buffer);
    try specReader.seekTo(0);
    reader = &specReader.interface;

    const manual = try cwd.createFile(io, "manual.txt", .{});
    defer manual.close(io);

    var manualFileWriterIdk = manual.writer(io, &.{});
    const manualFileWriter = &manualFileWriterIdk.interface;

    lineNumber = 0;
    var currentDirective: usize = 0;
    var directive = directives.items[0];

    var sourceFile: ?std.Io.File = null;
    var expectingBlockDirective = false;
    var codeBlockLine: usize = undefined;
    var padding: usize = undefined;
    var sourceBuffer: [1024]u8 = undefined;
    var sourceFileReader: std.Io.File.Reader = undefined;
    var sourceReader: *std.Io.Reader = undefined;

    defer if (sourceFile) |f| f.close(io);

    while (reader.takeDelimiter('\n')) |maybeLine| {
        const line = maybeLine orelse break;
        lineNumber += 1;
        if (currentDirective >= directives.items.len or directive.line != lineNumber) {
            try manualFileWriter.writeAll(line);
            try manualFileWriter.writeAll("\n");
            continue;
        }

        if (directive.kind != .KEYWORD) {
            std.debug.print("PROCESSING DIRECTIVE on line {d: >8}: {s}\n", .{lineNumber, std.mem.trimStart(u8, line, " \t")});
        }

        //std.debug.print("Processing directive: '{s}'\n", .{std.mem.trimStart(u8, std.mem.trimEnd(u8, line, "\r"), " \t")});
        switch (directive.kind) {
            .KEYWORDS => {
                try manualFileWriter.writeAll(wordSquareBuffer);
            },
            .INCLUDE => {
                sourceFile = cwd.openFile(
                    io,
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
                    codeBlockLine = 0;
                    var codeBlockLineCount: usize = 0;

                    if (sourceFile) |f| {
                        f.close(io);
                        sourceFile = null;
                    }

                    sourceFile = cwd.openFile(
                        io,
                        directive.filepath,
                        .{.mode = .read_only}
                    ) catch |err| switch(err) {
                        error.FileNotFound => {
                            std.debug.print("Warning: #block failed, File not found: {s}", .{directive.filepath});
                            continue;
                        },
                        else => return err,
                    };
                    sourceFileReader = sourceFile.?.reader(io, &sourceBuffer);
                    sourceReader = &sourceFileReader.interface;

                    while (sourceReader.takeDelimiter('\n')) |maybeBlockLine| {
                        if (maybeBlockLine == null) break;
                        codeBlockLineCount += 1;
                    } else |err| {
                        if (err != error.EndOfStream) return err;
                    }

                    // try sourceFile.?.seekTo(io, 0);
                    sourceFileReader = sourceFile.?.reader(io, &sourceBuffer);
                    try sourceFileReader.seekTo(0);
                    sourceReader = &sourceFileReader.interface;

                    const width = if (codeBlockLineCount == 0) 1 else std.math.log10(codeBlockLineCount);
                    padding = ((width / 4) + 1) * 4 + directive.indent;

                    try manualFileWriter.print("    {s}:\n", .{directive.filepath});
                }
                while (sourceReader.takeDelimiter('\n')) |maybeSourceLine| {
                    const sourceLine = maybeSourceLine orelse break;
                    const noCharachterReturn = std.mem.trimEnd(u8, sourceLine, "\r");
                    const cleanLine = std.mem.trimStart(u8, noCharachterReturn, " \t");

                    if (std.mem.startsWith(u8, cleanLine, "#comment")) {
                        expectingBlockDirective = true;
                        break;
                    }

                    codeBlockLine += 1;
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
            .KEYWORD => {
                currentDirective += 1;
                if (currentDirective < directives.items.len) {
                    directive = directives.items[currentDirective];
                }
                continue;
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
