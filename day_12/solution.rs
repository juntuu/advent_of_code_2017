use std::io::{self, BufRead};
use std::collections::{HashMap, HashSet};


fn pop(set: &mut HashSet<u32>) -> u32 {
    let e = set.iter().next().cloned().unwrap();
    set.remove(&e);
    e
}

fn pop_group(from: u32, connections: &mut HashMap<u32, Vec<u32>>) -> usize {
    let mut visited = HashSet::new();
    let mut todo = HashSet::new();
    todo.insert(from);
    while !todo.is_empty() {
        let i = pop(&mut todo);
        if visited.insert(i) {
            if let Some(v) = connections.get(&i) {
                todo.extend(v);
                connections.remove(&i);
            }
        }
    }
    visited.len()
}

fn count_groups(connections: &mut HashMap<u32, Vec<u32>>) -> usize {
    let mut total = 0;
    while !connections.is_empty() {
        let i = connections.keys().next().unwrap();
        pop_group(*i, connections);
        total += 1;
    }
    total
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let input = io::stdin();
    let mut connections = HashMap::new();
    for line in input.lock().lines() {
        let line = line?;
        let mut it = line.split("<->");
        let from: u32 = it.next().expect("bad input").trim().parse()?;
        let out: Vec<u32> = it.next().expect("bad input")
            .split(",")
            .map(|p| p.trim().parse().expect("bad input"))
            .collect();
        connections.insert(from, out);
    }
    let p1 = pop_group(0, &mut connections);
    println!("{}", p1);
    let p2 = 1 + count_groups(&mut connections);
    println!("{}", p2);
    Ok(())
}
