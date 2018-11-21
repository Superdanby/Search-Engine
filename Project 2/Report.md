# Project 1
資工四 404410076 陳鐸元

## Features

`-d`: delimiter(defaults to ‘\n’)
`-sd`: in-record delimiter(defaults to delimiter)
`-k`: sub-record selection key(regex)
`-p`: ignore preceding(regex)
`-n`: numerical sort
`-i`: ignore case
`-s`: sort by size
`-m`: memory usage(defaults to 4Gb)
`-j`: number of cpus(defaults to all cpus - 1)
`-J`: external sort cpu reduction mode(default: use 1 cpu only)

## Structure

- Input: read at most 10Mb of data at a time (single thread)
	- After hitting memory limit, create a LWP to parse, sort and output (multi-thread)
- Internal sort: qsort
- External sort: merge sort with winner tree, `O(log(k) * cmp * N)`
	- `-J = 3`: use 1 cpu
	- `-J = 0`: reduce half of cpus each time
	- `-J = 1`: recude cpus by square root
	- `-J = 2`: reduce cpus to 1 after the first merge
- Compare function
	- locate key -> ignore preceding -> sort by size -> numericalize -> ignore case
	- Perl Compatible Regular Expression: key, preceding, numerical

## Example

![regexsource](./regexsource.png)
![regex](./regex.png)

## Compile

`gcc -Wall -Wextra -std=c11 -pthread -o record_sort record_sort.c -lpcre2-8 -lm`

## Benchmark

### Computer Hardware

- CPU: i7 - 6700HQ (4C8T)
- Memory: 16GB (2x8GB)
- Storage: 512Gb M.2 PCIE Gen 3 x4 SSD (Plextor M8pegn)

### Test Data

18 Gb Youtube data

## `-d $'\n'@$'\n' -s`

- `./record_sort 1501651417.rec -d $'\n'@$'\n' -s 1501937141.rec 1502275226.rec 1502462361.rec 1502660320.rec 1502864422.rec 1503161947.rec 1503740416.rec 1503863552.rec -J = 0`: 13m6.023s

## `-d $'\n'@$'\n' -sd $'\n' -k published -p @ -n`

- `./record_sort 1501651417.rec -d $'\n'@$'\n' -sd $'\n' -k published -p @ -n 1501937141.rec 1502275226.rec 1502462361.rec 1502660320.rec 1502864422.rec 1503161947.rec 1503740416.rec 1503863552.rec -J = 0`: 21m21.992s

## Code(907 lines)

[https://github.com/Superdanby/Search-Engine/blob/master/Project%202/record_sort.c](https://github.com/Superdanby/Search-Engine/blob/master/Project%202/record_sort.c)
