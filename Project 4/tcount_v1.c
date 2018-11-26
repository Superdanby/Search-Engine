#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>
#include <string.h>

const int max_length = 100000;
// const char* files[] = {"tin", "ettoday1.rec", "ettoday2.rec", "ettoday3.rec", "ettoday4.rec", "ettoday5.rec"};
const char* files[] = {"ettoday0.rec", "ettoday1.rec", "ettoday2.rec", "ettoday3.rec", "ettoday4.rec", "ettoday5.rec"};
const wchar_t start = 0x4E00;
const wchar_t stop = 0x9FBB;

#define MaxHash 1000000

unsigned int * hash_table = NULL;
typedef struct {
	unsigned int next;
	unsigned int count;
	wchar_t * key;
} node;
node * node_table = NULL;
unsigned int node_cnt = 1, node_size = 2;

unsigned long long hash33(wchar_t *str)
{
    unsigned long long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

void hInsert(wchar_t *key) // need hdx, kdx
{
	unsigned int value = hash33(key) % MaxHash;
	unsigned int iterate = hash_table[value], previous = 0;
	while(iterate)
	{
		if(!wcscmp(key, node_table[iterate].key))
		{
			++node_table[iterate].count;
			free(key);
			return;
		}
		previous = iterate;
		iterate = node_table[iterate].next;
	}
	if(previous)
		node_table[previous].next = node_cnt;
	else
		hash_table[value] = node_cnt;
	if (node_cnt == node_size)
	{
		node_size <<= 1;
		node_table = (node *)realloc(node_table, node_size * sizeof(node));
	}
	node_table[node_cnt].count = 1;
	node_table[node_cnt].next = 0;
	node_table[node_cnt++].key = key;
	return;
}

int cmp (const void * a, const void * b)
{
	if ((*(node *)a).count < (*(node *)b).count)
		return 1;
	return 0;
}

int main()
{
	setlocale(LC_ALL, "");

	FILE * inpFile;
	FILE * outFile;
	wchar_t mystring [max_length + 2];
	outFile = fopen ("opt", "w");

	hash_table = (unsigned int *)malloc(MaxHash * sizeof(unsigned int));
	memset(hash_table, 0, MaxHash * sizeof(unsigned int));
	node_table = (node *)malloc(node_size * sizeof(node));

	for (int i = 0; i < 6; i++)
	{
		printf("%d\n", i);
		inpFile = fopen (files[i] , "r");
		printf("file %s\n", files[i]);
		if (inpFile != NULL)
		{
			while ( fgetws (mystring, max_length, inpFile) != NULL )
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
	qsort(node_table, node_cnt, sizeof(node), cmp);
	for (int i = 0; i < node_cnt; i++)
		fprintf (outFile, "%u %ls\n", node_table[i].count, node_table[i].key);
	fclose (outFile);
	return 0;
}
