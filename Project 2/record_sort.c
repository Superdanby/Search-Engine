#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>
#include <string.h>
#include <math.h>
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include <regex.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#define _GNU_SOURCE
#include <pthread.h>

const int Max_files = 256;
const wchar_t start = 0x4E00;
const wchar_t stop = 0x9FBB;

typedef long long ll;
typedef unsigned long long ull;
typedef struct {
	wchar_t* delimiter; // record_delimiter(default:\n)
	wchar_t* subdel; // delimiter inside a record(default to the value of delimiter)
	PCRE2_SIZE erroffset;
	int errorcode;
	char* key; // key_pattern
	pcre2_code *k_re;
	pcre2_match_data *k_match_data;
	short casei; // case_insensitive
	short reverse; // reverse_record_begin_pattern
	ull chunk; // chunk size in bytes (default: 10Mb)
	ull memory; // chunk size in bytes (default: 1Gb)
	ull threads; // threads limited to use (default: available threads - 1)
	short numerical; // numerical comparison
	pcre2_code *n_re;
	pcre2_match_data *n_match_data;
	wchar_t* timep; // time_pattern
	short size;
	int zero; // number of digits for nameing
	char* preceding; // ignored preceding characters (better: regex)
	pcre2_code *p_re;
	pcre2_match_data *p_match_data;
	int filedigit; // number of digits of a chunk name, calculated by memory & chunk
	char* final; // final output name
	ull ulimit; // Total data size
	int mode; // external cpu reduction mode, 0: log2, 1: sqrt, 2: all -> 1, 3: 1
} arguments;
arguments args = {L"\n", NULL, 0, 0, NULL, NULL, NULL, 0, 0, 0, 4000000000, 1, 0, NULL, NULL, 0, 0, 3, NULL, NULL, NULL, 0, "result", 0, 3};

int cmp (const void * a, const void * b)
{
	int ret = args.reverse == 1 ? 1 : 0; // increasing
	int rev = 1 - ret; // decreasing
	wchar_t** reca = *(wchar_t ***)a;
	wchar_t** recb = *(wchar_t ***)b;
	int idxa = 0, idxb = 0;

	// key_pattern
	if (args.key != NULL)
	{
		for (; reca[idxa][0] != L'\0'; idxa++)
		{
			// printf("idxain: %d, %ls\n", idxa, reca[idxa]);
			char* slima = (char*) malloc((wcslen(reca[idxa]) + 1)* sizeof(wchar_t));
			sprintf(slima, "%ls", reca[idxa]);
			if(pcre2_match(args.k_re, (PCRE2_SPTR)slima, -1, 0, 0, args.k_match_data, NULL) > 0)
			{
				free(slima);
				break;
			}
			free(slima);
		}
		if (reca[idxa][0] == L'\0')
			idxa = 0;

		for (; recb[idxb][0] != L'\0'; idxb++)
		{
			// printf("idxain: %d, %ls\n", idxb, recb[idxb]);
			char* slimb = (char*) malloc((wcslen(recb[idxb]) + 1)* sizeof(wchar_t));
			sprintf(slimb, "%ls", recb[idxb]);
			if(pcre2_match(args.k_re, (PCRE2_SPTR)slimb, -1, 0, 0, args.k_match_data, NULL) > 0)
			{
				free(slimb);
				break;
			}
			free(slimb);
		}
		if (recb[idxb][0] == L'\0')
			idxb = 0;
	}

	// length of unit to be compared
	int lena = wcslen(reca[idxa]);
	int lenb = wcslen(recb[idxb]);

	// unit to be compared
	wchar_t* loca = reca[idxa];
	wchar_t* locb = recb[idxb];
	wchar_t* tema = NULL;
	wchar_t* temb = NULL;

	// ignore preceding
	if (args.preceding != NULL)
	{
	    PCRE2_SIZE *ovector;
	    int rc;
		char* slima = (char*) malloc((wcslen(reca[idxa]) + 1)* sizeof(wchar_t));
		sprintf(slima, "%ls", reca[idxa]);
		char* slimb = (char*) malloc((wcslen(recb[idxb]) + 1)* sizeof(wchar_t));
		sprintf(slimb, "%ls", recb[idxb] );
	    PCRE2_SPTR value = (PCRE2_SPTR)slima;
	    rc = pcre2_match(args.p_re, value, -1, 0, 0, args.p_match_data, NULL);
	    if (rc <= 0)
	      printf("No preceding numbers: %s\n", slima);
	    else {
			ovector = pcre2_get_ovector_pointer(args.p_match_data);

			ull slimlen = strlen(slima);
			assert(slimlen > ovector[1]);
			tema = (wchar_t*)malloc((slimlen + 1) * sizeof(wchar_t));
			tema[0] = L'\0';
			if(swprintf(tema, slimlen, L"%hs", slima + ovector[1]) < 0)
			{
				printf("fail, slimlen: %lu\n", strlen(slima));
			}
			loca = tema;
	    }

		value = (PCRE2_SPTR)slimb;
	    rc = pcre2_match(args.p_re, value, -1, 0, 0, args.p_match_data, NULL);
	    if (rc <= 0)
	      printf("No preceding numbers: %s\n", slimb);
	    else {
			ovector = pcre2_get_ovector_pointer(args.p_match_data); /* Use ovector to get matched strings */

			ull slimlen = strlen(slimb);
			assert(slimlen > ovector[1]);
			temb = (wchar_t*)malloc((slimlen + 1) * sizeof(wchar_t));
			temb[0] = L'\0';
			if(swprintf(temb, slimlen, L"%hs", slimb + ovector[1]) < 0)
			{
				printf("fail");
			}
			locb = temb;
	    }
		free(slima);
		free(slimb);
	}

	// size
	if(args.size)
	{
		if(lena < lenb)
			return ret;
		else if(lenb < lena)
			return rev;
	}

	// numerical
	if (args.numerical)
	{
		int nua = 0, nub = 0;

	    PCRE2_SIZE *ovector;
	    int rc;
		char* slima = (char*) malloc((wcslen(reca[idxa]) + 1)* sizeof(wchar_t));
		sprintf(slima, "%ls", reca[idxa] );
		char* slimb = (char*) malloc((wcslen(recb[idxb]) + 1)* sizeof(wchar_t));
		sprintf(slimb, "%ls", recb[idxb] );
	    PCRE2_SPTR value = slima;
	    rc = pcre2_match(args.n_re, value, -1, 0, 0, args.n_match_data, NULL);
	    if (rc > 0)
		{
			ovector = pcre2_get_ovector_pointer(args.n_match_data); /* Use ovector to get matched strings */

			PCRE2_SIZE slen = ovector[1] - ovector[0];
			nua = slen;

		}

		value = slimb;
	    rc = pcre2_match(args.n_re, value, -1, 0, 0, args.n_match_data, NULL);
	    if (rc > 0)
		{
			ovector = pcre2_get_ovector_pointer(args.n_match_data); /* Use ovector to get matched strings */

			PCRE2_SIZE slen = ovector[1] - ovector[0];
			nub = slen;

		}
		free(slima);
		free(slimb);
		int difference = nua > nub ? nua - nub : nub - nua;
		if(nua > nub)
		{
			wchar_t* loc = locb;
			locb = (wchar_t*)malloc((lenb + difference + 2) * sizeof(wchar_t));
			for(int i = 0; i < difference; i++)
				locb[i] = L'0';
			locb[difference] = L'\0';
			wcscat(locb, loc);
		}

		if(nub > nua)
		{
			wchar_t* loc = loca;
			loca = (wchar_t*)malloc((lena + difference + 2) * sizeof(wchar_t));
			for(int i = 0; i < difference; i++)
				loca[i] = L'0';
			loca[difference] = L'\0';
			wcscat(loca, loc);
		}
	}

	int len = lena < lenb ? lena : lenb;

	wchar_t candia;
	wchar_t candib;

	for (int i = 0; i < len; i++)
	{
		candia = args.casei == 1 ? towlower(loca[i]) : loca[i];
		candib = args.casei == 1 ? towlower(locb[i]) : locb[i];
		if((ll)candia != (ll)candib && tema != NULL)
		{
			free(tema);
			free(temb);
		}
		if((ll)candia - (ll)candib < 0)
			return ret;
		else if((ll)candia - (ll)candib > 0)
			return rev;
	}
	ll localen = wcslen(loca);
	free(tema);
	free(temb);
	if ((ll)len == localen)
		return ret;
	return rev;
}

char* build_name(int num, int length)
{
	char* name = (char*)malloc((length + 2) * sizeof(char));
	char* fmt = (char*)malloc((2 + length + 1 + 2) * sizeof(char)); // sprintf
	strcpy(fmt, "%0");
	sprintf(fmt + 2, "%d", length);
	strcat(fmt, "d");
	sprintf(name, fmt, num);
	free(fmt);
	// printf("name%s\n", name);
	return name;
}

char* build_path(char* dirname, char* filename)
{
	char* fullpath = (char*) malloc((strlen(dirname) + 1 + strlen(filename) + 2) * sizeof(char));
	strcpy(fullpath, dirname);
	strcat(fullpath, "/");
	strcat(fullpath, filename);
	return fullpath;
}

ull parse_to (wchar_t **** recordsptr, wchar_t * unparsed)
{
	// all records
	wchar_t *** records = (wchar_t ***)malloc(sizeof(wchar_t **));
	ull rcnt = 0, rsc = 1;
	ll front = 0, back = 0, dlen = wcslen(args.delimiter);
	// assert(dlen < max_length);
	while(unparsed[front] != L'\0')
	{
		while(back - dlen + 1 < 0 || wcsncmp(unparsed + back - dlen + 1, args.delimiter, dlen) != 0)
		{
			if(unparsed[back + 1] == L'\0')
				break;
			++back;
		}

		// build records
		wchar_t ** strings = (wchar_t **)malloc(sizeof(wchar_t *));
		ull scnt = 0, ssc = 1;

		// read with unit delimiter
		ll ufront = front, uback = front, sdlen = wcslen(args.subdel);
		++back;
		front = back;

		while(ufront < back)
		{
			while(uback - ufront + 1 < sdlen || wcsncmp(unparsed + uback - sdlen + 1, args.subdel, sdlen) != 0)
			{
				// printf("record: %d %lc %d\n", uback, mystring[uback], sdlen);
				if(uback == back - 1)
					break;
				++uback;
			}
			// build unit
			wchar_t * unit = malloc((uback - ufront + 2) * sizeof(wchar_t));
			wcsncpy(unit, unparsed + ufront, uback - ufront + 1);
			unit[uback - ufront + 1] = L'\0';
			++uback;
			ufront = uback;
			if (scnt == ssc)
			{
				ssc <<= 1;
				// save unit to record
				strings = (wchar_t **)realloc(strings, ssc * sizeof(wchar_t *));
			}
			strings[scnt] = unit;
			++scnt;
		}

		strings = (wchar_t **)realloc(strings, (scnt + 1) * sizeof(wchar_t *));
		strings[scnt] = L"\0";

		if (rcnt == rsc)
		{
			rsc <<= 1;
			// save record to all data
			records = (wchar_t ***)realloc(records, rsc * sizeof(wchar_t **));
		}
		records[rcnt] = strings;
		++rcnt;
	}
	// data processing
	*recordsptr = records;
	free(unparsed);
	return rcnt;
}

int get_data (wchar_t ** unparsedptr, wchar_t ** buffer, FILE * inpFile)
{
	// buffers
	ull max_length = 10000000;
	max_length = args.memory / 3 < max_length ? args.memory / 3 : max_length;
	if(*buffer == 0)
	{
		*buffer = (wchar_t *)malloc(max_length * sizeof(wchar_t));
		*buffer[0] = L'\0';
	}
	wchar_t * mystring = (wchar_t *)malloc((max_length + 1) * sizeof(wchar_t));
	wchar_t * temp = (wchar_t *)malloc((max_length + 1) * sizeof(wchar_t));
	// wchar_t mystring [max_length + 1]; // record buffer
	// wchar_t temp [max_length + 1]; // record buffer
	mystring[0] = L'\0', mystring[max_length] = 0;

	// all records
	wchar_t * unparsed = (wchar_t *)malloc(args.memory * sizeof(wchar_t));
	*unparsedptr = unparsed;
	ull stop = args.memory - max_length * 2 - 2;
	unparsed[0] = L'\0', unparsed[stop] = L'\0';
	wcscpy(unparsed, *buffer);

	// data processing
	if (inpFile != NULL)
	{
		int go = 1;
		ll cont = 0, dlen = wcslen(args.delimiter);
		ll cnt = 0;
		// assert(dlen < max_length);
		while(unparsed[stop] == L'\0' && (go = fread(temp, 1, max_length, inpFile)))
		{
			mbstowcs(mystring, temp, go);
			wcscat(unparsed, mystring);
			// printf("go: %d, unparsed: %ls\n", go, unparsed);
			// printf("go: %d, unparsed: %ls\n", go, unparsed);
			// printf("go: %lld, target =%lld\n", cnt += go, args.memory - max_length - 1);
		}
		for(cont = wcslen(mystring) - dlen; go && cont >= 0; cont--)
		{
			if(wcsncmp(mystring + cont, args.delimiter, dlen) == 0)
			{
				// printf("cont %lld\n", cont);
				wcsncat(unparsed, mystring, cont + dlen);
				wcscpy(*buffer, mystring + cont + dlen);
				break;
			}
		}
		// printf("go: %d, unparsed: %ls\n", go, unparsed);
		free(mystring);
		free(temp);
		return go;
	}
	else
	{
		printf("Error opening file\n");
		exit(1);
	}
}

int update (wchar_t **** recordptr, int * winner, int idx, int * reccnt, const int empty)
{
	int ldx, rdx, pdx;
	while(idx)
	{
		pdx = (idx - 1) / 2;
		ldx = pdx * 2 + 1;
		rdx = ldx + 1;
		if(winner[ldx] == empty)
			winner[pdx] = winner[rdx];
		else if(winner[rdx] == empty)
			winner[pdx] = winner[ldx];
		else
		{
			if(!cmp(&(recordptr[winner[ldx]][reccnt[winner[ldx]]]), &(recordptr[winner[rdx]][reccnt[winner[rdx]]])))
				winner[pdx] = winner[ldx];
			else
				winner[pdx] = winner[rdx];
		}

		idx = pdx;
	}
	return 0;
}

typedef struct {
	wchar_t * unparsed;
	int ccnt; // stream count
} internal_args;

void internal (void * iargs)
{

	wchar_t * unparsed = (wchar_t *)(((internal_args *)iargs) -> unparsed);
	wchar_t *** records = NULL;

	ull rcnt = parse_to(&records, unparsed);
	int ccnt = (int)(((internal_args *)iargs) -> ccnt);
	free(iargs);
	qsort(records, rcnt, sizeof(wchar_t**), cmp);

	// build foldername
	char* dirname = build_name(ccnt, args.zero);

	struct stat st;
	// create folder
	if (stat(dirname, &st) == -1) {
		mkdir(dirname, 0755);
	}

	ull bcnt = 0; // byte count limited by size of chunks(args.chunk)
	int fcnt = 0; // chunk count

	// first output filename
	char* filename = build_name(fcnt++, args.filedigit);

	// build fullpath
	char* fullpath = build_path(dirname, filename);
	free(filename);

	FILE * outFile = fopen (fullpath, "w");
	free(fullpath);
	assert(outFile != 0);
	assert(rcnt != 0);

	for (ull k = 0; k < rcnt; k++)
	{
		assert(records[k][0][0] != L'\0');
		for (int j = 0; records[k][j][0] != L'\0'; j++)
		{
			bcnt += wcslen(records[k][j]);
			fprintf (outFile, "%ls", records[k][j]);
			free(records[k][j]);
		}
		assert(bcnt != 0);
		free(records[k]);
		if (bcnt >= args.chunk)
		{
			bcnt = 0;
			fclose (outFile);

			// update filename
			filename = build_name(fcnt++, args.filedigit);

			// rebuild fullpath
			fullpath = build_path(dirname, filename);
			free(filename);
			outFile = fopen (fullpath, "w");
			free(fullpath);
		}
	}
	free(dirname);
	dirname = NULL;
	free(records);
	records = NULL;
	fullpath = NULL;
	fclose (outFile);
	outFile = NULL;
	return;
}

typedef struct {
	int start; // input start folder
	int size;
	int target; // output foldername
	char* final; // final filename;
} external_args;

void external (void * eargs)
{
	int start = (int)(((external_args *)eargs) -> start);
	int ccnt = (int)(((external_args *)eargs) -> size);
	int target = (int)(((external_args *)eargs) -> target);
	char* final = (char*)(((external_args *)eargs) -> final);
	printf("start: %d, ccnt: %d, target: %d, final: %s\n", start, ccnt, target, final);
	free(eargs);
	ull bcnt = 0; // byte count, limited by memory usage(args.memory)

	// second input + ccnt-way merge sort
	wchar_t*** recordptr[ccnt]; // in-memory records
	int wincnt = 0;
	while((1 << (++wincnt)) < ccnt);
	int sdx = (1 << (wincnt)) - 1; // starting index
	wincnt = (sdx + 1) * 2; // sizeof winner tree
	int winner[wincnt]; // zero-based priority queue
	for(int i = 0; i < wincnt; i++)
		winner[i] = target; // impossible folder index
	int recmax[ccnt]; // record count of each chunk of the input file
	int reccnt[ccnt]; // record index of each stream
	memset(reccnt, 0, sizeof(reccnt));
	int* chunkcnt = (int*)malloc(ccnt * sizeof(int)); // index of the chunks inside a folder
	char* dirname = NULL;
	char* filename = NULL;
	char* fullpath = NULL;

	// file I/O
	FILE * inpFile = NULL;
	FILE * outFile = NULL;

	// initialize the winner tree
	for(int i = 0; i < ccnt; i++)
	{
		// initialize chunkcnt
		chunkcnt[i] = 0;

		// build foldername
		dirname = build_name(start + i, args.zero);

		// first input filename
		filename = build_name(chunkcnt[i]++, args.filedigit);

		// build fullpath
		fullpath = build_path(dirname, filename);
		free(dirname);
		free(filename);

		inpFile = fopen (fullpath, "r"); // Do not free fullpath
		wchar_t * unparsed = NULL;
		wchar_t * filebuffer = NULL;
		get_data(&unparsed, &filebuffer, inpFile);
		free(filebuffer);
		recmax[i] = parse_to(&recordptr[i], unparsed);
		winner[sdx + i] = i;
		fclose (inpFile);
		remove(fullpath);
		free(fullpath);
		assert(reccnt[i] == 0);
		update(recordptr, winner, sdx + i, reccnt, target);
	}
	// printf("Finished Winner tree initialization\n");
	int newchunk = 0;
	if(final != NULL)
		outFile = fopen(final, "w");
	else
	{
		dirname = build_name(target, args.zero);

		struct stat st;
		// create folder
		if (stat(dirname, &st) == -1) {
			mkdir(dirname, 0755);
		}

		filename = build_name(newchunk++, args.filedigit);
		fullpath = build_path(dirname, filename);
		free(dirname);
		free(filename);
		outFile = fopen(fullpath, "w");
	}

	assert(outFile != 0);
	// start merge O(n * ccnt), output
	while(winner[0] != target)
	{
		// output stream winner[0]
		assert(recordptr[winner[0]][reccnt[winner[0]]][0][0] != NULL);
		for (int i = 0; recordptr[winner[0]][reccnt[winner[0]]][i][0] != L'\0'; i++)
		{
			// printf ("%ls\0", recordptr[winner[0]][reccnt[winner[0]]][i]);
			assert(wcslen(recordptr[winner[0]][reccnt[winner[0]]][i]) != 0);
			fprintf (outFile, "%ls", recordptr[winner[0]][reccnt[winner[0]]][i]);
			bcnt += wcslen(recordptr[winner[0]][reccnt[winner[0]]][i]);
			free(recordptr[winner[0]][reccnt[winner[0]]][i]);
		}
		free(recordptr[winner[0]][reccnt[winner[0]]]);

		// write to next chunk
		if(bcnt > args.memory)
		{
			if(final == NULL)
			{
				fclose(outFile);
				dirname = build_name(target, args.zero);
				filename = build_name(newchunk++, args.filedigit);
				fullpath = build_path(dirname, filename);
				outFile = fopen(fullpath, "w");
				assert(outFile != NULL);
				free(dirname);
				free(filename);
				printf("output to: %s\n", fullpath);
			}
			bcnt = 0;
		}

		// check if stream winner[0] is empty
		if(++reccnt[winner[0]] == recmax[winner[0]])
		{
			// check if no chunks available
			DIR *dir = NULL;
			struct dirent *ent = NULL;
			dirname = build_name(start + winner[0], args.zero);
			if ((dir = opendir (dirname)) != NULL)
			{
				/* print all the files and directories within directory */
				while ((ent = readdir(dir)))
				{
					if (!strcmp(ent -> d_name, ".") || !(strcmp(ent -> d_name, "..")))
						continue;
					else
					{
						filename = build_name(chunkcnt[winner[0]]++, args.filedigit);
						fullpath = build_path(dirname, filename);
						free(filename);
						free(recordptr[winner[0]]);
						inpFile = fopen (fullpath, "r"); // Do not free fullpath
						wchar_t * unparsed = NULL;
						wchar_t * filebuffer = NULL;
						get_data(&unparsed, &filebuffer, inpFile);
						free(filebuffer);
						recmax[winner[0]] = parse_to(&recordptr[winner[0]], unparsed);
						printf("got %d records from: %s\n", recmax[winner[0]], fullpath);
						reccnt[winner[0]] = 0;
						fclose (inpFile);
						remove(fullpath);
						free(fullpath);
						if(recmax[winner[0]] != 0)
							break;
					}
				}
				if (ent == NULL)
				{
					winner[sdx + winner[0]] = target;
					remove(dirname);
				}
				free(dirname);
				closedir (dir);
				dir = NULL;
			}
			else
			{
				/* could not open directory */
				perror ("failed to open directory!\n");
				exit(2);
			}
		}
		update(recordptr, winner, sdx + winner[0], reccnt, target);
	}
	fclose (outFile);
	return;
}

off_t fsize(const char *filename)
{
    struct stat st;

    if (stat(filename, &st) == 0)
        return st.st_size;

    perror("failed to determine filesize: ");
    perror(filename);
	exit(3);
}

int main(int argc, char** argv)
{
	// encoding
	setlocale(LC_ALL, "");

	// process arguments
	int sd_flag = 0;
	char** files = (char**)malloc(Max_files * sizeof(char*));
	int filecnt = 0;
	args.threads = sysconf(_SC_NPROCESSORS_ONLN) - 1;
	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-d"))
		{
			args.delimiter = (wchar_t*)malloc(strlen(argv[++i]) * sizeof(wchar_t));
			mbstowcs(args.delimiter, argv[i], strlen(argv[i]));
		}
		else if (!strcmp(argv[i], "-sd"))
		{
			args.subdel = (wchar_t*)malloc(strlen(argv[++i]) * sizeof(wchar_t));
			mbstowcs(args.subdel, argv[i], strlen(argv[i]));
			sd_flag = 1;
		}
		else if (!strcmp(argv[i], "-k"))
		{
			args.key = argv[++i];
			args.k_re = pcre2_compile((PCRE2_SPTR8)args.key, -1, 0, &args.errorcode, &args.erroffset, NULL);
			if (args.k_re == NULL) {
				PCRE2_UCHAR8 buffer[120];
				(void)pcre2_get_error_message(args.errorcode, buffer, 120);
				/* Handle error */
				exit (1);
			}
			args.k_match_data = pcre2_match_data_create_from_pattern(args.k_re, NULL);
		}
		else if (!strcmp(argv[i], "-i"))
			args.casei = 1;
		else if (!strcmp(argv[i], "-r"))
			args.reverse = 1;
		else if (!strcmp(argv[i], "-c"))
		{
			char * pend;
			args.chunk = strtol(argv[++i], &pend, 10);
		}
		else if (!strcmp(argv[i], "-m"))
		{
			char * pend;
			args.memory = strtoll(argv[++i], &pend, 10);
		}
		else if (!strcmp(argv[i], "-j"))
		{
			char * pend;
			args.threads = strtoll(argv[++i], &pend, 10);
		}
		else if (!strcmp(argv[i], "-J"))
		{
			char * pend;
			args.mode = strtoll(argv[++i], &pend, 10);
		}
		else if (!strcmp(argv[i], "-n"))
		{
			args.numerical = 1;
			args.n_re = pcre2_compile((PCRE2_SPTR8)"^[0-9]+", -1, 0, &args.errorcode, &args.erroffset, NULL);
			if (args.n_re == NULL) {
				PCRE2_UCHAR8 buffer[120];
				(void)pcre2_get_error_message(args.errorcode, buffer, 120);
				/* Handle error */
				exit (1);
			}
			args.n_match_data = pcre2_match_data_create_from_pattern(args.n_re, NULL);
		}
		else if (!strcmp(argv[i], "-t"))
		{
			args.timep = (wchar_t*)malloc(strlen(argv[++i]) * sizeof(wchar_t));
			mbstowcs(args.timep, argv[i], strlen(argv[i]));
		}
		else if (!strcmp(argv[i], "-s"))
			args.size = 1;
		else if (!strcmp(argv[i], "-p"))
		{
			args.preceding = argv[++i];
			args.p_re = pcre2_compile((PCRE2_SPTR8)args.preceding, -1, 0, &args.errorcode, &args.erroffset, NULL);
			if (args.p_re == NULL) {
				PCRE2_UCHAR8 buffer[120];
				(void)pcre2_get_error_message(args.errorcode, buffer, 120);
				/* Handle error */
				exit (1);
			}
			args.p_match_data = pcre2_match_data_create_from_pattern(args.p_re, NULL);
		}
		else if (!strcmp(argv[i], "-o"))
		{
			args.final = argv[++i];
		}
		else
		{
			args.ulimit += fsize(argv[i]);
			files[filecnt++] = argv[i];
		}
	}
	if (!sd_flag)
		args.subdel = args.delimiter;

	ull newmemory = args.memory / sizeof(wchar_t);
	ull new_chunk_count = args.ulimit / newmemory + 1;
	args.filedigit = 1;
	while(new_chunk_count /= 10)
		++args.filedigit;
	args.filedigit += 2;
	ull memsplit = args.threads > sysconf(_SC_NPROCESSORS_ONLN) ? sysconf(_SC_NPROCESSORS_ONLN) : args.threads;
	args.memory /= sizeof(wchar_t) * memsplit;
	args.chunk = (args.memory * args.memory * memsplit / args.ulimit ) / 2;
	args.chunk = args.chunk > args.memory ? args.memory : args.chunk;
	ull folder_max = args.ulimit / args.memory;
	args.zero = 1;
	while(folder_max /= 10)
		++args.zero;
	args.zero += 2;

	// file I/O
	FILE * inpFile = NULL;
	// FILE * outFile;

	// buffers
	int ccnt = 0; // ccnt is stream cnt
	pthread_t * thread = (pthread_t*)malloc(args.threads * sizeof(pthread_t)); // pthread of each stream
	int pcnt = args.threads; // thread count
	int thread_token = 0;
	int thread_max = 0;

	// first input + internal sort + split to output
	for (int i = 0; i < filecnt; i++)
	{
		printf("%d\n", i);
		printf("file: %s\n", files[i]);
		inpFile = fopen (files[i], "r"); // Do not free fullpath
		wchar_t * filebuffer = NULL; // hold data between memory limits in the same file, should initialize with NULL
		int go = 1;
		while(go)
		{
			internal_args* iargs = (internal_args*)malloc(sizeof(internal_args));
			iargs -> ccnt = ccnt++;
			iargs -> unparsed = NULL;
			go = get_data(&(iargs -> unparsed), &filebuffer, inpFile);
			while(!pcnt)
			{
				while(1)
				{
					thread_token %= args.threads;
					if(pthread_tryjoin_np(thread[thread_token], NULL) == 0)
					{
						++pcnt;
						break;
					}
					printf("thread_token %d in use\n", thread_token);
					++thread_token;
				}
			}
			--pcnt;
			printf("thread_token: %d\n", thread_token);
			pthread_create(thread + thread_token++, NULL, (void *)(internal), (void *)iargs);
			thread_max = thread_max > thread_token ? thread_max : thread_token;
		}
		free(filebuffer);
		printf("Finished reading file: %s\n", files[i]);
		fclose (inpFile);

	}
	for(int i = 0; i < thread_max; i++)
		pthread_join(thread[i], NULL);
	printf("Finished internal sort\n");

	// second input + ccnt-way merge sort
	int start = 0;
	if(args.mode == 3)
		thread_max = 1;
	while(thread_max)
	{
		int base = ccnt / thread_max, add = ccnt % thread_max;
		args.memory = newmemory / thread_max / 8;
		// CHECK THREAD MAX
		for(int i = 0, j = 0; i < thread_max; i++)
		{
			external_args * eargs = (external_args *)malloc(sizeof(external_args));
			eargs -> start = start + i * base + j;
			eargs -> size = base;
			if(add-- > 0)
			{
				++j;
				++(eargs -> size);
			}
			eargs -> target = start + ccnt + i;
			eargs -> final = NULL;
			if(thread_max == 1)
				eargs -> final = args.final;
			pthread_create(thread + i, NULL, (void *)(external), (void *)eargs);

		}
		start += ccnt;
		ccnt = thread_max;
		for(int i = 0; i < thread_max; i++)
			pthread_join(thread[i], NULL);
		if(thread_max != 1)
			switch (args.mode) {
				case 1:
					thread_max = (int)sqrt(thread_max);
					break;
				case 2:
					thread_max = 1;
					break;
				default:
					thread_max >>= 1;
			}
		else
			thread_max = 0;
	}
	printf("Bye~\n");
	return 0;
}
