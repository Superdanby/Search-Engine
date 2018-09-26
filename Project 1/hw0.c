#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>
#include <string.h>

const int max_length = 100000;
const char* files[] = {"ettoday0.rec", "ettoday1.rec", "ettoday2.rec", "ettoday3.rec", "ettoday4.rec", "ettoday5.rec"};
const wchar_t start = 0x4E00;
const wchar_t stop = 0x9FBB;

int cmp (const void * a, const void * b)
{
	if (wcscmp (*(wchar_t **)a, *(wchar_t **)b) < 0)
		return 0;
	return 1;
}

int main()
{
	setlocale(LC_ALL, "");

	FILE * inpFile;
	FILE * outFile;
	wchar_t mystring [max_length + 2];
	outFile = fopen ("opt", "w");
	wchar_t ** strings = (wchar_t **)malloc(sizeof(wchar_t *));
	wchar_t ** grow = NULL;
	int cnt = 0, sc = 1;


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
						while (mystring[front] < start || mystring[front] > stop)
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
						front = back + 1;
						back = front;

						if (cnt == sc)
						{
							sc <<= 1;
							grow = (wchar_t **)realloc(strings, sc * sizeof(wchar_t *));
							strings = grow;
						}
						strings[cnt] = cut;
						++cnt;
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
	qsort(strings, cnt, sizeof(wchar_t*), cmp);
	for (int i = 0; i < cnt; i++)
		fprintf (outFile, "%ls\n", strings[i]);

	fclose (outFile);
	return 0;
}
