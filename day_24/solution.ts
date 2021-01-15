const lines = () => {
	let text = "";
	const decoder = new TextDecoder();
	const buffer = new Uint8Array(1024);
	for (let read = null; (read = Deno.stdin.readSync(buffer));) {
		text += decoder.decode(buffer.slice(0, read));
	}
	return text.trim().split("\n");
}

let strong = -1;
let len = -1;
let long = -1;

const bridges = (xs: Set<number[]>, i=0, n=1, s=0) => {
	[...xs].filter(([a, b]) => i === a || i === b).forEach(x => {
		let [a, b] = x;
		a = a ^ b ^ i;
		b = s + a + i;
		if (b > strong) {
			strong = b;
		}
		if (n > len || (n == len && b > long)) {
			len = n;
			long = b;
		}
		xs.delete(x);
		bridges(xs, a, n+1, b);
		xs.add(x);
	});
};

bridges(new Set(lines().map(line => line.split("/").map(Number))));

Deno.stdout.writeSync(new TextEncoder().encode(`${strong}\n${long}\n`));
