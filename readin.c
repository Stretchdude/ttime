#include <stdio.h>

#include <stdio.h>
#include <string.h>

#include <time.h>
#include <sys/time.h>
#include <malloc.h>
#include <stdlib.h>

#define ERR_BADTOKEN 1



#define main not_main
#include "main.c"
#undef main

struct my_date {
	int day;
	int mon;
	int year;
	struct my_time *times;
	struct my_date *next;
};


struct my_date *extract_date(int *olen, const char *in)
{
	const char *p = in;
	char date[16] = {0};
	struct my_date *ret = calloc(sizeof(struct my_date), 1);
	
	*olen = 0;
	while(p < in + 12) {
		if (*p == '-') break;
		(*olen)++;
		p++;
	}
	strncpy(date, in, *olen);
	printf("cdate: %s\n", date);
	char *ptr = strtok(date, ".");
	ret->day = atoi(ptr);
	ptr = strtok(NULL, ".");
	ret->mon = atoi(ptr);	
	ptr = strtok(NULL, ".");
	ret->year = atoi(ptr);	
	ret->times = NULL;
	ret->next = NULL;
}

void scan_options(char *ini)
{
	printf("sorry not implemented: %s\n", __func__);
}

void write_day_to_file(char *filename, struct my_date *day)
{
	int n = 0;
	int worked_a_day = 0;
	struct my_date *it = day;
	if (NULL == it) {
		return;
	}
	FILE *file = fopen(filename, "w+");
	if (NULL == file) printf("error opening out file\n");
	fprintf(file, "date;{come_time;leave_time;};;work_time;break_time;;accounted\n");
	printf("date;{come_time;leave_time;};;work_time;break_time;;accounted\n");
	while (it) {
		struct my_time *tit = it->times;
		struct work_day d = sum_up_times(it->times, get_now());
		int work_minutes = t2min(&d.workt);
		fprintf(file, "%02d.%02d.%04d;", it->day, it->mon, it->year);
		printf("%02d.%02d.%04d;", it->day, it->mon, it->year);
		while(tit){
			fprintf(file, "%02d:%02d;", tit->h, tit->m);
			printf("%02d:%02d;", tit->h, tit->m);
			tit = tit->next;
		}

		if (it->day == 1 && it->mon == 8 && it->year == 2048) {
			printf("options line. no accounting\n");
		} else {
			worked_a_day = work_minutes - target_workmin;
		}
		fprintf(file, ";%d;%d;;%d\n", work_minutes, t2min(&d.breakt), worked_a_day);
		printf(";%d;%d;;%d\n", work_minutes, t2min(&d.breakt), worked_a_day);
		it = it->next;
	}
	fprintf(file, "\n");
	printf("\n");
	fclose(file);
}


void print_days(struct my_date *d)
{
	int n = 0;
	struct my_date *it = d;
	if (NULL == it) {
		return;
	}
	while (it) {
		print_list(it->times);
		it = it->next;
	}
}

int main(int argc, char **argv)
{
	FILE *in = fopen("lala.csv", "r");
	char line[128]= {0};
	struct my_date *first_day = NULL;
	struct my_date *date_it = first_day;
	while (NULL != fgets(line, sizeof(line), in)) {
		char *p = line;		
		while (p < &line[sizeof(line) - 1]) {
			if (*p == ';') *p = '-';
			if (*p == ' ' || *p == '\t'){
				char *del = p;
				while (*del) {
					*del = *(del + 1);
					del++;
				}
				p++;
			}
			printf("%c", *p++);
		}
		int len = 0;
		if (NULL == first_day){
			first_day = extract_date(&len, line);
			date_it = first_day;
		} else {
			date_it->next = extract_date(&len, line);
			date_it = date_it->next;
		}
		printf("line {%d}: %s\n", len, line);

		printf("date{: %d . %d . %d}\n", date_it->day, date_it->mon, date_it->year);
		if (date_it->day == 1 && date_it->mon == 8 && date_it->year == 2048) {
			// the day i retire. on this day i (hopefully) don't care about time taking :-)
			scan_options(line + len);
			
		} else {
			printf("time input: %s\n", line + len);
			struct my_time *tt = scan_time(line + len);
			date_it->times = tt;
			print_list(tt);
		}
		memset(line, 0, sizeof(line));
		printf("---------------\n");
	}
	printf("------------------------------------\n");
	print_days(first_day);
	printf("------------------------------------\n");
	write_day_to_file("output.csv", first_day);
}
