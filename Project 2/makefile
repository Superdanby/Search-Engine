all: record_sort.c
	gcc -Wall -Wextra -std=c11 -pthread -o record_sort record_sort.c -lpcre2-8 -lm
test: all
	./record_sort -sd / -k ^.2 -p ^. -n num num2
	./record_sort -sd / -p ^. -n num num2
	./record_sort -sd / -n num num2
	./record_sort -sd / num2
	./record_sort num2
	./record_sort ../HW0/ettoday0.rec ../HW0/ettoday1.rec ../HW0/ettoday2.rec ../HW0/ettoday3.rec ../HW0/ettoday4.rec ../HW0/ettoday5.rec
	./record_sort 1501651417.rec -d $'\n'@$'\n' -sd $'\n' -k published
	./record_sort test.rec -d $'\n'@$'\n' -sd $'\n' -k published -m 10000 -c 1000
run: all
	time ./record_sort 1501651417.rec -d $'\n'@$'\n' -sd $'\n' -k published 1501937141.rec 1502275226.rec 1502462361.rec 1502660320.rec 1502864422.rec 1503161947.rec 1503740416.rec 1503863552.rec
