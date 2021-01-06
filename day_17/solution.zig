const std = @import("std");

const List = struct {
    data: usize,
    next: *List,
    fn advance(self: List, n: usize) List {
        var p: *const List = &self;
        var i: usize = 0;
        while (i < n) : (i += 1) {
            p = p.next;
        }
        return p.*;
    }

    fn insert(self: List, buf: []List, data: usize) List {
        buf[data] = List{
            .data = data,
            .next = self.next,
        };
        buf[self.data].next = &buf[data];
        return buf[data];
    }
};

fn part1(step: usize) usize {
    const limit = 2018;
    var buf: [limit]List = undefined;
    buf[0] = List{
        .data = 0,
        .next = &buf[0],
    };
    var p = buf[0];
    var i: usize = 1;
    while (i < limit) : (i += 1) {
        p = p.advance(step % i).insert(&buf, i);
    }
    return p.next.*.data;
}

fn part2(step: usize) usize {
    var res: usize = 0;
    var i: usize = 1;
    var at: usize = 0;
    while (i <= 50000000) : (i += 1) {
        at = 1 + (at + step) % i;
        if (at == 1)
            res = i;
    }
    return res;
}

fn readNum(file: std.fs.File) !usize {
    var buf: [@sizeOf(usize)]u8 = undefined;
    const amt = try file.read(&buf);
    const line = std.mem.trimRight(u8, buf[0..amt], "\r\n");
    return std.fmt.parseUnsigned(usize, line, 10);
}

pub fn main() !void {
    const step = try readNum(std.io.getStdIn());
    const stdout = std.io.getStdOut().writer();
    try stdout.print("{}\n", .{part1(step)});
    try stdout.print("{}\n", .{part2(step)});
}
