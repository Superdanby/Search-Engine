#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>
#include <string.h>
#include <assert.h>

const wchar_t start = 0x4E00;
const wchar_t stop = 0x9FBB;
const int Max_files = 256;

typedef long long ll;
typedef unsigned long long ull;
typedef struct {
	ull memory; // chunk size in kilobytes (default: 1Gb)
	// ull threads; // threads limited to use (default: available threads - 1)
	ull max_length;
	ull MaxHash;
	char * extend;
	FILE * external;
	char * opt;
} arguments;
arguments args = {1000000000, 100000, 1000000, "extend", NULL, "tcount.out"};

typedef struct {
	unsigned int next;
	long next_filepos;
	unsigned int count;
	wchar_t * key;
} node;
node * node_table = NULL;
unsigned int node_cnt = 1, node_size = 2;

typedef struct {
	unsigned int next; // next node
	long next_filepos; // next node
} head;
head * hash_table = NULL;

long write_to(unsigned int index, long filepos, unsigned int count, unsigned int next, long next_filepos, wchar_t * key)
{
	if(index == 0)
	{
		if(key == NULL)
			fseek(args.external, filepos, SEEK_SET);
		else
			fseek(args.external, 0, SEEK_END);
		long retpos = ftell(args.external);
		// printf("count: %ld\n", ftell(args.external));
		fwrite(&count, sizeof(unsigned int), 1, args.external);
		// printf("next_filepos: %ld\n", ftell(args.external));
		fwrite(&next_filepos, sizeof(long), 1, args.external);
		// printf("key: %ld\t%ls\n", ftell(args.external), key);
		if(key != NULL)
		{
			fwrite(key, sizeof(wchar_t), wcslen(key) + 1, args.external);
			// printf("stopped: %ld\n", ftell(args.external));
			free(key);
		}
		return retpos;
	}
	if(count != 0)
		node_table[index].count = count;
	node_table[index].next = next;
	node_table[index].next_filepos = next_filepos;
	if(key != NULL)
		node_table[index].key = key;
	return 0;
}

void read_from(unsigned int index, long filepos, unsigned int * count, unsigned int * next, long * next_filepos, wchar_t ** key)
{
	if(index == 0)
	{
		fseek(args.external, filepos, SEEK_SET);
		// printf("count: %ld\n", ftell(args.external));
		fread(count, sizeof(unsigned int), 1, args.external);
		*next = 0;
		// printf("next_filepos: %ld\n", ftell(args.external));
		fread(next_filepos, sizeof(long), 1, args.external);
		*key = (wchar_t *)malloc((args.max_length + 1) * sizeof(wchar_t));
		// printf("key: %ld\t", ftell(args.external));
		fread(*key, sizeof(wchar_t), args.max_length + 1, args.external);
		// printf("%ls\n", *key);
		// fgetws(*key, args.max_length, args.external);
		// printf("%ls\n", *key);
		return;
	}
	*next = node_table[index].next;
	*next_filepos = node_table[index].next_filepos;
	*count = node_table[index].count;
	*key = (wchar_t *)malloc((args.max_length + 1) * sizeof(wchar_t));
	wcscpy(*key, node_table[index].key);
	return;
}

unsigned long long hash33(wchar_t *str)
{
    unsigned long long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

ull mem_in_use;
void hInsert(wchar_t *key) // need hdx, kdx
{
	unsigned int value = hash33(key) % args.MaxHash;
	unsigned int iterate = hash_table[value].next, previous = 0, count = 0;
	long filepos = hash_table[value].next_filepos, previous_filepos = 0;
	wchar_t * key_saved = NULL;

	while(iterate || filepos)
	{
		unsigned int next;
		long next_filepos;
		read_from(iterate, filepos, &count, &next, &next_filepos, &key_saved);
		if(!wcscmp(key, key_saved))
		{
			free(key);
			free(key_saved);
			write_to(iterate, filepos, ++count, next, next_filepos, NULL);
			return;
		}
		free(key_saved);
		previous = iterate;
		previous_filepos = filepos;
		iterate = next;
		filepos = next_filepos;
	}
	if(previous || previous_filepos)
	{
		if( mem_in_use >= args.memory)
		{
			long retpos = write_to(0, -1, 1, 0, 0, key);
			write_to(previous, previous_filepos, count, 0, retpos, NULL);
			return;
		}
		else
		{
			mem_in_use += wcslen(key);
			write_to(previous, 0, count, node_cnt, 0, NULL);
		}
	}
	else
	{
		if( mem_in_use >= args.memory)
		{
			hash_table[value].next = 0;
			hash_table[value].next_filepos = write_to(0, -1, 1, 0, 0, key);
			return;
		}
		else
		{
			mem_in_use += wcslen(key);
			hash_table[value].next = node_cnt;
			hash_table[value].next_filepos = 0;
		}
	}
	if (node_cnt == node_size)
	{
		mem_in_use += sizeof(node) * node_cnt;
		node_size <<= 1;
		node_table = (node *)realloc(node_table, node_size * sizeof(node));
	}
	write_to(node_cnt++, -1, 1, 0, 0, key);
	return;
}

int main(int argc, char** argv)
{
	setlocale(LC_ALL, "");

	char** files = (char**)malloc(Max_files * sizeof(char*));
	int filecnt = 0;
	char * extend = "extend";
	// args.threads = sysconf(_SC_NPROCESSORS_ONLN) - 1;
	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-m"))
		{
			char * pend;
			args.memory = strtoll(argv[++i], &pend, 10);
		}
		else if (!strcmp(argv[i], "-k"))
		{
			char * pend;
			args.max_length = strtoll(argv[++i], &pend, 10);
		}
		else if (!strcmp(argv[i], "-h"))
		{
			char * pend;
			args.MaxHash = strtoll(argv[++i], &pend, 10);
		}
		else if (!strcmp(argv[i], "-o"))
		{
			args.opt = argv[++i];
		}
		else if (!strcmp(argv[i], "-t"))
		{
			extend = argv[++i];
		}
		else
		{
			files[filecnt++] = argv[i];
		}
	}
	args.external = fopen(extend, "w+");
	mem_in_use = sizeof(head) * args.MaxHash;

	FILE * inpFile;

	hash_table = (head *)malloc(args.MaxHash * sizeof(head));
	memset(hash_table, 0, args.MaxHash * sizeof(head));
	node_table = (node *)malloc(node_size * sizeof(node));
	wchar_t * mystring = (wchar_t *)malloc((args.max_length + 2) * sizeof(wchar_t));

	for (int i = 0; i < filecnt; i++)
	{
		printf("%d\n", i);
		inpFile = fopen (files[i] , "r");
		printf("file %s\n", files[i]);
		if (inpFile != NULL)
		{
			while ( fgetws (mystring, args.max_length, inpFile) != NULL )
			{
				int length = wcslen(mystring);
				if (wcsncmp (mystring, L"@", 1) == 0)
					continue;
				for(int back = 0, front = 0; back < length; back++)
				{
					if (mystring[back] == L'，' || mystring[back] == L'。' || mystring[back] == L'\n' || mystring[back] == L'！' || mystring[back] == L'？')
					{
						while (front < back && (mystring[front] < start || mystring[front] > stop))
							++front;
						if (mystring[back] == L'\n')
							back -= 1;
						if (back - front <= 5)
						{
							front = back + 1;
							back = front;
							continue;
						}
						wchar_t * cut = (wchar_t *)malloc((back - front + 2) * sizeof(wchar_t));
						wcsncpy(cut, mystring + front, back - front + 1);
						cut[back - front + 1] = L'\0';
						front = back + 1;
						back = front;
						hInsert(cut);
					}
				}
			}
			fclose ( inpFile );
		}
		else
		{
			printf("Error opening file %s\n", files[i]);
		}
	}
	free(mystring);

	printf("start yo output\n");
	FILE * outFile = fopen(args.opt, "w");
	for(int i = 0; i < args.MaxHash; i++)
	{
		unsigned int iterate = hash_table[i].next, count = 0;
		long filepos = hash_table[i].next_filepos;
		wchar_t * key = NULL;
		while (iterate || filepos) {
			read_from(iterate, filepos, &count, &iterate, &filepos, &key);
			// printf("%u %ld\n", iterate, filepos);
			fprintf(outFile, "%u\n%ls\n\n", count, key);
			free(key);
		}
	}
	fclose (outFile);
	fclose(args.external);
	remove(args.extend);
	return 0;
}
