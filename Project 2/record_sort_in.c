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

const int Max_files = 256;
const int max_length = 300000;
const wchar_t start = 0x4E00;
const wchar_t stop = 0x9FBB;

typedef long long ll;
typedef struct {
	wchar_t* delimiter; // record_delimiter(default:\n)
	wchar_t* subdel; // delimiter inside a record(default to the value of delimiter)
	char* key; // key_pattern
	// wchar_t* key; // key_pattern
	short casei; // case_insensitive
	short reverse; // reverse_record_begin_pattern
	short numerical; // numerical comparison
	wchar_t* timep; // time_pattern
	short size;
	char* preceding; // ignored preceding characters (better: regex)
} arguments;
arguments args = {L"\n", NULL, NULL, 0, 0, 0, 0, 0, NULL};

int cmp (const void * a, const void * b)
{
	int ret = args.reverse ? 1 : 0; // increasing
	int rev = 1 - ret; // decreasing

	wchar_t** reca = *(wchar_t ***)a;
	wchar_t** recb = *(wchar_t ***)b;
	int idxa = 0, idxb = 0;

	// key_pattern
	if (args.key != NULL)
	{
		pcre2_code *re;
		pcre2_match_data *match_data;
		PCRE2_SIZE erroffset;
		int errorcode;
		PCRE2_SPTR pattern = args.key;
		// printf("key: %s\n", args.key);
		re = pcre2_compile(pattern, -1, 0, &errorcode, &erroffset, NULL);
		if (re == NULL) {
			PCRE2_UCHAR8 buffer[120];
			(void)pcre2_get_error_message(errorcode, buffer, 120);
			/* Handle error */
			exit (1);
		}
		match_data = pcre2_match_data_create_from_pattern(re, NULL);
		for (; reca[idxa][0] != L'\0'; idxa++)
		{
			// printf("idxain: %d, %ls\n", idxa, reca[idxa]);
			char* slima = (char*) malloc((wcslen(reca[idxa]) + 1)* sizeof(wchar_t));
			sprintf(slima, "%ls", reca[idxa]);
			if(pcre2_match(re, (PCRE2_SPTR)slima, -1, 0, 0, match_data, NULL) > 0)
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
			if(pcre2_match(re, (PCRE2_SPTR)slimb, -1, 0, 0, match_data, NULL) > 0)
			{
				free(slimb);
				break;
			}
			free(slimb);
		}
		if (recb[idxb][0] == L'\0')
			idxb = 0;
		pcre2_match_data_free(match_data);
		pcre2_code_free(re);
	}

	// length of unit to be compared
	int lena = wcslen(reca[idxa]);
	int lenb = wcslen(recb[idxb]);

	// unit to be compared
	wchar_t* loca = reca[idxa];
	wchar_t* locb = recb[idxb];
	wchar_t* tema = NULL;
	wchar_t* temb = NULL;
	// printf("loca: %ls\n", loca);
	// printf("locb: %ls\n", locb);

	// ignore preceding
	if (args.preceding != NULL)
	{
		pcre2_code *re;
	    pcre2_match_data *match_data;
	    PCRE2_SIZE erroffset, *ovector;
	    int errorcode;
	    int rc;
	    PCRE2_SPTR pattern = args.preceding;
	    // PCRE2_SPTR pattern = strcat("^", args.preceding);
		char* slima = (char*) malloc((wcslen(reca[idxa]) + 1)* sizeof(wchar_t));
		sprintf(slima, "%ls\0", reca[idxa]);
		char* slimb = (char*) malloc((wcslen(recb[idxb]) + 1)* sizeof(wchar_t));
		sprintf(slimb, "%ls\0", recb[idxb] );
	    // PCRE2_SPTR value = "waaaye";
	    PCRE2_SPTR value = (PCRE2_SPTR)slima;
	    re = pcre2_compile(pattern, -1, 0, &errorcode, &erroffset, NULL);
	    if (re == NULL) {
			PCRE2_UCHAR8 buffer[120];
			(void)pcre2_get_error_message(errorcode, buffer, 120);
			/* Handle error */
			exit(1);
	    }
	    match_data = pcre2_match_data_create_from_pattern(re, NULL);
	    rc = pcre2_match(re, value, -1, 0, 0, match_data, NULL);
		// printf("startslima: %s\n", slima);
		// printf("hi");
	    if (rc <= 0)
	      printf("No preceding numbers: %s\n", slima);
	    else {
			ovector = pcre2_get_ovector_pointer(match_data);
			// printf( "Match succeeded at offset %zu\n", ovector[0] );
			/* Use ovector to get matched strings */

			// printf("%s %d %d\n", slima + 3000, strlen(slima), ovector[1]);
			assert(strlen(slima) > ovector[1]);
			// assert(strlen(slima) - ovector[1] < 3000);
			tema = (wchar_t*)malloc(max_length);
			memset(tema, 0, max_length);
			if(swprintf(tema, max_length / sizeof(wchar_t) - 1, L"%hs\0", slima + ovector[1]) < 0)
			{
				printf("fail, slimlen%d\n", strlen(slima));
			}
			loca = tema;
			// printf("locaw %ls\n", loca);
	    }

		// value = "waaaye";
		value = (PCRE2_SPTR)slimb;
	    rc = pcre2_match(re, value, -1, 0, 0, match_data, NULL);
	    if (rc <= 0)
	      printf("No preceding numbers: %s\n", slimb);
	    else {
			ovector = pcre2_get_ovector_pointer(match_data);
			// printf( "Match succeeded at offset %zu\n", ovector[0] );
			/* Use ovector to get matched strings */

			temb = (wchar_t*)malloc(max_length);
			if(swprintf(temb, max_length / sizeof(wchar_t) - 1, L"%hs\0", slimb + ovector[1]) < 0)
			{
				printf("fail");
			}
			locb = temb;
			// printf("locbw %ls\n", locb);
	    }
		free(slima);
		free(slimb);
		// pcre2_match_data_free(match_data);
		// pcre2_code_free(re);
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
		// regex_t re;

		pcre2_code *re;
	    pcre2_match_data *match_data;
	    PCRE2_SIZE erroffset, *ovector;
	    int errorcode;
	    int rc;
	    PCRE2_SPTR pattern = "^[0-9]+";
		char* slima = (char*) malloc((wcslen(reca[idxa]) + 1)* sizeof(wchar_t));
		sprintf(slima, "%ls", reca[idxa] );
		char* slimb = (char*) malloc((wcslen(recb[idxb]) + 1)* sizeof(wchar_t));
		sprintf(slimb, "%ls", recb[idxb] );
	    PCRE2_SPTR value = slima;
	    re = pcre2_compile(pattern, -1, 0, &errorcode, &erroffset, NULL);
	    if (re == NULL) {
			PCRE2_UCHAR8 buffer[120];
			(void)pcre2_get_error_message(errorcode, buffer, 120);
			/* Handle error */
			exit(1);
	    }
	    match_data = pcre2_match_data_create_from_pattern(re, NULL);
	    rc = pcre2_match(re, value, -1, 0, 0, match_data, NULL);
	    if (rc > 0)
		{
			ovector = pcre2_get_ovector_pointer(match_data);
			// printf( "Match succeeded at offset %zu\n", ovector[0] );
			/* Use ovector to get matched strings */

			PCRE2_SIZE slen = ovector[1] - ovector[0];
			// printf( "%.*s\n", (int)slen, (char *)start );
			nua = slen;

		}
	    // else {
		// 	printf("No preceding numbers: %s\n", slima);
	    // }

		value = slimb;
	    rc = pcre2_match(re, value, -1, 0, 0, match_data, NULL);
	    if (rc > 0)
		{
			ovector = pcre2_get_ovector_pointer(match_data);
			// printf( "Match succeeded at offset %zu\n", ovector[0] );
			/* Use ovector to get matched strings */

			PCRE2_SIZE slen = ovector[1] - ovector[0];
			// printf( "%.*s\n", (int)slen, (char *)start );
			nub = slen;

		}
		free(slima);
		free(slimb);
	    // else {
		// 	printf("No preceding numbers: %s\n", slimb);
	    // }
		pcre2_match_data_free(match_data);
		pcre2_code_free(re);
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

int main(int argc, char** argv)
{
	// encoding
	setlocale(LC_ALL, "");

	// process arguments
	int sd_flag = 0;
	char** files = (char**)malloc(Max_files * sizeof(char*));
	int filecnt = 0;
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
			// args.key = (wchar_t*)malloc(strlen(argv[++i]) * sizeof(wchar_t));
			// mbstowcs(args.key, argv[i], strlen(argv[i]));
			args.key = argv[++i];
		}
		else if (!strcmp(argv[i], "-i"))
			args.casei = 1;
		else if (!strcmp(argv[i], "-r"))
			args.reverse = 1;
		else if (!strcmp(argv[i], "-n"))
			args.numerical = 1;
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
		}
		else
			files[filecnt++] = argv[i];
	}
	if (!sd_flag)
		args.subdel = args.delimiter;

	// file I/O
	FILE * inpFile;
	FILE * outFile;
	outFile = fopen ("opt", "w");

	// buffers
	wchar_t charbuf [2];
	charbuf[1] = L'\0';
	wchar_t mystring [max_length + 2]; // record buffer
	mystring[0] = L'\0';

	// all records
	wchar_t *** records = (wchar_t ***)malloc(sizeof(wchar_t **));
	int rcnt = 0, rsc = 1;

	// data processing
	for (int i = 0; i < filecnt; i++)
	{
		printf("%d\n", i);
		inpFile = fopen (files[i], "r");
		printf("file: %s\n", files[i]);
		if (inpFile != NULL)
		{
			// read with record delimiter
			int go = 1;
			int back = 0, dlen = wcslen(args.delimiter);
			wchar_t * pwc;
			wchar_t* buffer;
			while ( go )
			{
				charbuf[0] = fgetwc(inpFile);
				charbuf[1] = L'\0';
				if (charbuf[0] == WEOF)
				{
					charbuf[0] = L'\0';
					go = 0;
				}
				assert(charbuf[1] == L'\0');
				mystring[back] = charbuf[0];
				mystring[back + 1] = L'\0';
				// wcscat(mystring, charbuf);
				// printf("rec: %ls\n", mystring);
				assert(back < max_length);

				if(back - dlen + 1 < 0 || wcsncmp(mystring + back - dlen + 1, args.delimiter, dlen) != 0)
				{
					++back;
					continue;
				}

				// build records
				wchar_t ** strings = (wchar_t **)malloc(sizeof(wchar_t *));
				int scnt = 0, ssc = 1;

				// read with unit delimiter
				// printf("record: %ls\n", mystring);

				int ufront = 0, uback = 0, sdlen = wcslen(args.subdel);
				int sgo = 1;
				while (sgo)
				{
					if (uback - ufront + 1 < sdlen || wcsncmp(mystring + uback - sdlen + 1, args.subdel, sdlen) != 0)
					{
						// printf("record: %d %lc %d\n", uback, mystring[uback], sdlen);
						if(uback < back)
						{
							++uback;
							continue;
						}
					}

					// build unit
					wchar_t * unit = malloc((uback - ufront + 2) * sizeof(wchar_t));
					wcsncpy(unit, mystring + ufront, uback - ufront + 1);
					unit[uback - ufront + 1] = L'\0';
					++uback;
					if(uback > back)
					{
						sgo = 0;
					}
					ufront = uback;
					if (scnt == ssc)
					{
						ssc <<= 1;
						// save unit to record
						strings = (wchar_t **)realloc(strings, ssc * sizeof(wchar_t *));
					}
					strings[scnt] = unit;
					// printf("unit: %ls\n", strings[scnt]);
					++scnt;
				}
				back = 0;
				mystring[0] = L'\0';
				// printf("hello\n");
				// return 0;

				strings = (wchar_t **)realloc(strings, (scnt + 1) * sizeof(wchar_t *));
				strings[scnt] = L"\0";

				if (rcnt == rsc)
				{
					rsc <<= 1;
					// save record to all data
					records = (wchar_t ***)realloc(records, rsc * sizeof(wchar_t **));
				}
				records[rcnt] = strings;
				// for (int j = 0; records[rcnt][j][0] != L'\0'; j++)
				// 	printf ("%d: %ls\n", j, records[rcnt][j]);
				++rcnt;
			}
			fclose ( inpFile );
		}
		else
		{
			printf("Error opening file %s\n", files[i]);
		}
		// for (int k = 0; k < rcnt; k++)
		// 	for (int j = 0; records[k][j][0] != L'\0'; j++)
		// 		printf ("%d: %ls\n", k, records[k][j]);
	}
	printf("Finish reading!\n");

	printf ("%ls\n", records[0][0]);
	qsort(records, rcnt, sizeof(wchar_t**), cmp);
	printf("Finished sorting\n");

	for (int i = 0; i < rcnt; i++)
	{
		for (int j = 0; records[i][j][0] != L'\0'; j++)
			fprintf (outFile, "%ls\0", records[i][j]);
		fprintf (outFile, "Record: %d\n", i);
	}

	fclose (outFile);
	return 0;
}
