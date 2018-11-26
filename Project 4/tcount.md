# Project 0.1 External Hash
資工四 404410076 陳鐸元

## Arguments

`-m`: memory limit in bytes
`-h`: number of entries in the hash table
`-s`: largest key size

## Structure

- Input: read and parse all files
- hInsert: hash and store the record
	- After hitting memory limit, write to a temporary file
- Output: iterate through each list in each entry
- Sort: use HW1’s record sort to sort it
	- `../HW1/record_sort -d $'\n'$'\n' -sd $'\n' -r -n ../HW0/tcount.out -o ../HW0/tout`

## Benchmark

### Computer Hardware

- CPU: i7 - 6700HQ (4C8T)
- Memory: 16GB (2x8GB)
- Storage: 512Gb M.2 PCIE Gen 3 x4 SSD (Plextor M8pegn)

### Test Data

1 GB news records

## In-memory only

- 51s
- ![](in-memory-only.png)

## In-memory

- 4m7s
- ![](in-memory.png)

## Limited-memory

![](limited-memory.png)

## Validation

![](validation.png)

## Code

[https://github.com/Superdanby/Search-Engine/tree/master/Project%204](https://github.com/Superdanby/Search-Engine/tree/master/Project%204)
