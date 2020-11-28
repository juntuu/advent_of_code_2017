struct Stdin: Sequence, IteratorProtocol {
    func getIterator() -> Self {
        return self
    }
    func next() -> String? {
        return readLine()
    }
}

func validate<T: Hashable>(_ line: String, _ item: (Substring) -> T) -> Bool {
    var words = Set<T>()
    return line.split(separator: " ").map(item).allSatisfy {
        words.insert($0).inserted
    }
}

func good(_ line: String) -> Bool {
    return validate(line) { String($0) }
}

func better(_ line: String) -> Bool {
    return validate(line) { Array($0).sorted() }
}

func solve<T: Sequence>(_ lines: T) -> (Int, Int) where T.Element == String {
    var p1 = 0
    var p2 = 0
    for line in lines {
        if good(line) {
            p1 += 1
            if better(line) {
                p2 += 1
            }
        }
    }
    return (p1, p2)
}

let (p1, p2) = solve(Stdin())
print(p1)
print(p2)
