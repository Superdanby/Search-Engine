#include <bits/stdc++.h>
#include <functional>

using namespace std;

typedef long long ll;
typedef unsigned long long ull;

struct arguments {
	string output = "out";
	string title = "\n@T:";
	string content = "\n@B:";
	string url = "\n@U:";
	string delimiter = "\n";
	ull memory = 4000000000;
} args;

struct Record {
	ll score;
	string title;
	// string time;
	string info;
	string url;
	string content;
};

inline ll hit_score(const string &A, const string &B, const int &tolerance)
{
	// cerr << A << "\t" << B  << "\n";
	vector<int> table(A.size() + 1);

	for(int i = 0; i <= A.size(); i++)
		table[i] = i;

	ll score = 0;

	int ubound = min((int)table.size(), tolerance + 2);
	for(int j = 0; j < B.size(); j++)
	{
		for(int i = 1; i < ubound; i++)
		{
			int prev = table[i];
			table[i] = min(table[0] + (A[i - 1] != B[j]), min(table[i] + 1, table[i - 1] + 1));
			table[0] = prev;
		}
		table[0] = 0;
		while(table[--ubound] > tolerance);
		ubound = min((int)table.size(), ubound + 2);
		score += table.back() <= tolerance ? tolerance - table.back() + 1 : 0;
	}
	return score;
}

void parse_to (vector<Record> &parsed, const string &unparsed, const int start, const int end, vector<string> &must, vector<string> &prefer, vector<string> &exclude)
{
	// all records
	int idx = start;
	while(idx < end)
	{
		Record inp;
		int uidx = unparsed.find(args.url, idx) + args.url.size();
		int tidx = unparsed.find(args.title, uidx);
		int cidx = unparsed.find(args.content, tidx + args.title.size());
		inp.url = unparsed.substr(uidx, tidx - uidx);
		tidx += args.title.size();
		inp.title = unparsed.substr(tidx, cidx - tidx);
		cidx += args.content.size() + 1;
		int endidx = unparsed.find(args.delimiter, cidx + args.content.size());
		inp.content = unparsed.substr(cidx, endidx - cidx);
		idx = endidx + 1;
		inp.score = 0;
		for(auto x: exclude)
		{
			inp.score += hit_score(x, inp.title, 0);
			inp.score += hit_score(x, inp.content, 0);
		}
		if(inp.score)
			continue;
		for(auto x: must)
		{
			inp.score += hit_score(x, inp.title, 0) * 100;
			inp.score += hit_score(x, inp.content, 0) * 20;
		}
		if(must.size() && !inp.score)
			continue;
		for(auto x: prefer)
		{
			inp.score += hit_score(x, inp.title, 2) * 5;
			inp.score += hit_score(x, inp.content, 2);
		}
		// cerr << inp.score << "\n";
		parsed.emplace_back(inp);
	}
	return;
}

bool cmp(const Record &A, const Record &B)
{
	return A.score > B.score;
}
void test(int a, int &b)
{
	cout << a << b << "\n";
}

int main(int argc, char ** argv) {
	// encoding
	setlocale(LC_ALL, "");
	ios_base::sync_with_stdio(false);
	cin.tie(0);
	cout.tie(0);

	vector<string> must, prefer, exclude, source;
	for(int i = 1; i < argc; i++)
	{
		if(!strcmp(argv[i], "-s"))
			source.emplace_back(string(argv[++i]));
		else if(!strcmp(argv[i], "+"))
			must.emplace_back(string(argv[++i]));
		else if(!strcmp(argv[i], "-"))
			exclude.emplace_back(string(argv[++i]));
		else if(!strcmp(argv[i], "-o"))
			args.output = string(argv[++i]);
		else
			prefer.emplace_back(string(argv[i]));
	}

	ifstream ifs;
	vector<Record> all;
	for(auto inp_name : source)
	{
		ifs.open(inp_name, ifstream::in);
		string buffer;
		stringstream buf;
		// cerr << "start reading\n";
		buf << ifs.rdbuf();
		// cerr << "read\n";
		// Number of threads available
		unsigned int nthreads = max(thread::hardware_concurrency() - 1, (unsigned int)1);
		vector<vector<Record>> parsed(nthreads);
		vector<thread> threads(nthreads);
		const string & unparsed = buf.str();
		int unit_len = unparsed.size() / nthreads, idx = 0, endidx;
		for(int i = 0; i < nthreads - 1; i++)
		{
			endidx = unparsed.find(args.url, idx + unit_len);
			vector<Record> &parsed_ref = parsed[i];
			threads[i] = thread(parse_to, ref(parsed_ref), ref(unparsed), idx, endidx, ref(must), ref(prefer), ref(exclude));
			// threads[i] = thread(test, 1, std::ref(3));
			idx = endidx + 1;
		}
		vector<Record> &parsed_back = parsed.back();
		threads.back() = thread(parse_to, ref(parsed_back), ref(unparsed), idx, unparsed.size(), ref(must), ref(prefer), ref(exclude));
		for (int i = 0; i < threads.size(); i++)
			threads[i].join();
		// for (auto x: threads)
		// 	x.join();
		for (auto x: parsed)
			all.insert(all.end(), make_move_iterator(x.begin()), make_move_iterator(x.end()));
		// cerr << "parsed\n";
		ifs.close();
	}
	sort(all.begin(), all.end(), [](const Record &A, const Record &B){return A.score > B.score;});
	ofstream ofs;
	ofs.open(args.output, ofstream::out);

	for(auto x: all)
	{
		// cout << "<h3><a href=\"" << x.url << "\">" << x.title << "</a></h3><br/><div align=\"right\">Score: " << x.score << "</div><br/><p>" << x.content << "</p><br/>\n";
		int strend = 1001;
		while((unsigned int)x.content[--strend] >= 128 && (unsigned int)x.content[strend] < 192);
		ofs << "<h3><a href=\"" << x.url << "\">" << x.title << "</a></h3><div align=\"right\">Score: " << x.score << "</div><p>" << x.content.substr(0, strend) << "</p><br/>\n";
	}
	ofs << all.size() << "\n";
	// cout << all.size() << "\n\n";
	return 0;
}
