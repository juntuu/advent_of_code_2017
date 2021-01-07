#include <cstdio>
#include <vector>
#include <unordered_set>
#include <map>

int idiv(int a, int b)
{
	return (a % b == 0) ? a / b : -1;
}

int isqrt(int a)
{
	if (a < 0)
		return -1;
	int r = sqrt(a);
	return (a == r*r) ? r : -1;
}

std::unordered_set<int> solve(int a, int b, int c, int d, int e, int f)
{
	auto const A = c-f;
	if (A == 0)
		return {};
	auto const B = 2*(b-e) + A;
	auto const C = 2*(a-d);
	auto const det = isqrt(B*B - 4*A*C);
	if (det < 0)
		return {};
	std::unordered_set<int> res{};
	a = idiv(-B+det, 2*A);
	b = idiv(-B-det, 2*A);
	if (a >= 0)
		res.insert(a);
	if (b >= 0)
		res.insert(b);
	return res;
}

struct Point {
	int p[3];
	int v[3];
	int a[3];

	int acceleration() const
	{
		return abs(a[0]) + abs(a[1]) + abs(a[2]);
	}

	std::unordered_set<int> collision(Point const& other) const
	{
		auto const x = solve(p[0], v[0], a[0], other.p[0], other.v[0], other.a[0]);
		auto const y = solve(p[1], v[1], a[1], other.p[1], other.v[1], other.a[1]);
		auto const z = solve(p[2], v[2], a[2], other.p[2], other.v[2], other.a[2]);
		std::unordered_set<int> res{};
		for (auto const& i : x)
			if (y.count(i) && z.count(i))
				res.insert(i);
		return res;
	}
};

int main()
{
	std::vector<Point> points{};
	struct Point p{};

	constexpr char LINE[] = " p=<%d,%d,%d>, v=<%d,%d,%d>, a=<%d,%d,%d> ";
	while (9 == scanf(LINE, &p.p[0], &p.p[1], &p.p[2], &p.v[0], &p.v[1], &p.v[2], &p.a[0], &p.a[1], &p.a[2]))
		points.push_back(p);

	if (points.size() < 1)
		return 1;

	size_t min_i = 0;
	auto min_a = points[0].acceleration();
	std::map<int, std::unordered_set<int>> collisions{};
	int const n = points.size();
	for (int i = 0; i < n; i++) {
		auto const& p = points[i];
		auto a = p.acceleration();
		if (a < min_a) {
			min_a = a;
			min_i = i;
		}
		for (int j = i+1; j < n; j++)
			for (const auto t : p.collision(points[j]))
				collisions[t].insert({i, j});
	}

	printf("%ld\n", min_i);

	std::unordered_set<int> removed{};
	for (auto& [_, coll] : collisions) {
		for (auto i : removed)
			coll.erase(i);
		if (coll.size() > 1)
			removed.merge(coll);
	}

	printf("%ld\n", points.size() - removed.size());
}
