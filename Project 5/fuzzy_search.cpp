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
	double score;
	string title;
	// string time;
	string info;
	string url;
	string content;
};


ll boyer_moore(const string &A, const string &B)
{
	// Boyer Moore Initialization
	// bad bmInitocc();
	unordered_map<char, int> occurred;
    for (unsigned int i = 0; i < A.size(); i++)
    {
        occurred[A[i]] = i;
    }

    // bmPreprocess1();
	vector<int> border_front(A.size() + 1);
	vector<int> shift(A.size() + 1);
    size_t i = A.size(), j = A.size() + 1;
    border_front[i] = j;
    while (i > 0)
    {
        while (j <= A.size() && A[i-1] != A[j-1])
        {
            if (shift[j] == 0)
				shift[j] = j - i;
            j = border_front[j];
        }
        border_front[--i] = --j;
    }

    // bmPreprocess2();
	j = border_front[0];
    for(i = 0; i <= A.size(); i++)
    {
        if(shift[i] == 0)
			shift[i] = j;
        if(i == j)
			j = border_front[j];
    }

	// Boyer Moore Search
	ll cnt = 0;
    int Aidx = 0, Bidx;
    while (Aidx <= int(B.size()) - int(A.size()))
    {
        Bidx = A.size() - 1;
        while(Bidx >= 0 && A[Bidx] == B[Aidx + Bidx])
			--Bidx;
        if (Bidx < 0)
        {
            ++cnt;
            Aidx += shift[0];
        }
        else
            Aidx += max((shift[Bidx + 1]), Bidx - (occurred.find(B[Aidx + Bidx]) != occurred.end() ? occurred[B[Aidx + Bidx]] : -1));
    }
	return cnt;
}

inline ll hit_score(const string &A, const string &B, const int &tolerance)
{
	if(tolerance == 0)
		return boyer_moore(A, B);
	vector<int> table(A.size() + 1);

	for(size_t i = 0; i <= A.size(); i++)
		table[i] = i;

	double score = 0;

	int ubound = min((int)table.size(), tolerance + 2);
	for(size_t j = 0; j < B.size(); j++)
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
		score += table.back() <= tolerance ? double(tolerance - table.back() + 1) / double(table.back() + 1) : 0;
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
			double must_score = hit_score(x, inp.title, 0) * 100;
			must_score += hit_score(x, inp.content, 0) * 20;
			if(must_score)
				inp.score += must_score;
			else
			{
				inp.score = 0;
				break;
			}
		}
		if(must.size() && !inp.score)
			continue;
		for(auto x: prefer)
		{
			int tolerance = x.size() < 9 ? 1 : 2;
			tolerance = x.size() < 5 ? 0 : tolerance;
			inp.score += hit_score(x, inp.title, tolerance) * 5;
			inp.score += hit_score(x, inp.content, tolerance);
		}
		if(!inp.score)
			continue;
		parsed.emplace_back(inp);
	}
	return;
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
	if(exclude.empty() && must.empty() && prefer.empty())
	{
		ofstream ofs;
		ofs.open(args.output, ofstream::out);
		cout << "0\n";
		return 0;
	}

	ifstream ifs;
	vector<Record> all;
	for(auto inp_name : source)
	{
		ifs.open(inp_name, ifstream::in);
		string buffer;
		stringstream buf;
		buf << ifs.rdbuf();
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
		ifs.close();
	}
	sort(all.begin(), all.end(), [](const Record &A, const Record &B){return A.score > B.score;});
	ofstream ofs;
	ofs.open(args.output, ofstream::out);

	for(auto x: all)
	{
		ofs << "<h3><a href=\"" << x.url << "\">" << x.title << "</a></h3><div align=\"right\">Score: " << x.score << "</div><p>" << x.content << "</p>\n";
	}
	cout << all.size() << "\n";
	return 0;
}
