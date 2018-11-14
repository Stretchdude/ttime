#include "header.h"

#include <stdio.h>

#include <time.h>
#include <sys/time.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>


#define FILE_HEADER  "date;{come_time;leave_time;};;work_time;break_time;;accounted\n"

struct my_date *extract_date(int *olen, const char *in)
{
	const char *p = in;
	char date[16] = {0};
	struct my_date *ret = calloc(sizeof(struct my_date), 1);

	char has_date = 0;
	int count = 0;
	while (*p != '-' && p < (in + 12)) {
		if (*p == '.') {
			has_date = 1;	
			break;
		}
		p++;
	}	
	p = in;

	if (12 > strlen(in) || !has_date){
		return NULL;	
	}	

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

void write_days_to_file(char *filename, struct my_date *day)
{
	int n = 0;
	int worked_a_day = 0;
	struct my_date *it = day;
	if (NULL == it) {
		return;
	}

	FILE *file = fopen(filename, "w+");
	if (NULL == file){
		 printf("error opening out file\n");
		return;
	}
	fprintf(file, FILE_HEADER);
//	printf(FILE_HEADER);

	while (it) {
		struct my_time *tit = it->times;
		struct work_day d = sum_up_times(it->times, get_now());
		int work_minutes = t2min(&d.workt);
		fprintf(file, "%02d.%02d.%04d;", it->day, it->mon, it->year);
//		printf( "# %02d.%02d.%04d;", it->day, it->mon, it->year);
		while(tit){
			fprintf(file, "%02d:%02d;", tit->h, tit->m);
//			printf("%02d:%02d;", tit->h, tit->m);
			tit = tit->next;

		}

		if (it->day == 1 && it->mon == 8 && it->year == 2048) {
			printf("options line. no accounting\n");
		} else {
			worked_a_day = work_minutes - TARGET_WORK_MIN;
		}
		fprintf(file, ";%d;%d;;%d\n", work_minutes, t2min(&d.breakt), worked_a_day);
//		printf(";%d;%d;;%d\n", work_minutes, t2min(&d.breakt), worked_a_day);
		it = it->next;

	}

	fprintf(file, "\n");
	fclose(file);
}

struct my_date *get_last_day(struct my_date *first)
{
	struct my_date *it = first;
	while (NULL != it->next){
		it = it->next;
	}
	return it;
}

void append_day(struct my_date *days, struct my_date *day)
{
	struct my_date *last = get_last_day(days);
	last->next = day;
}

struct my_date *get_today()
{
	struct tm *now = get_now();
	struct my_date *today = malloc(sizeof(*today));
	today->day = now->tm_mday;
	today->mon = now->tm_mon + 1;
	today->year = now->tm_year+1900;
	today->next =	NULL;
	today->times = NULL;
	return today;
}
struct my_time *get_now_time()
{
	struct tm *now = get_now();
	struct my_time *nowt = malloc(sizeof(*nowt));
	nowt->h = now->tm_hour;
	nowt->m = now->tm_min;
	nowt->s = now->tm_sec;
	nowt->next = NULL;
	return nowt;
}

void append_times(struct my_date *day, struct my_time *time)
{
	if (NULL == day){
		day = get_today();
	}
	struct my_time *it = day->times;

	if (NULL == it){
		day->times = time;
		return;
	}
HERE();
	while (it->next)
		it = it->next;
	
	it->next = time;
}

void add_today_with_times(struct my_date *days, struct my_time *times)
{
	struct my_date *today = get_today();
	today->times = times;
	
	append_day(days, today);
}

void add_now_as_time(struct my_date *days)
{
	struct my_date *last = get_last_day(days);
	struct my_date *today = get_today();
	struct my_date *day = NULL;
	
//	printf("%d vs. %d\n", today->year, last->year);
//	printf("%d vs. %d\n", today->mon, last->mon);
//	printf("%d vs. %d\n", today->day, last->day);
	if (today->year != last->year || today->mon != last->mon || today->day != last->day) {
		append_times(today, get_now_time());
		append_day(days, today);
HERE();
	} else {
HERE();
		append_times(last, get_now_time());
//		add_today_with_times(days, get_now_time());
	}
}


void print_days(struct my_date *d)
{
	int n = 0;
	struct my_date *it = d;
	if (NULL == it) {
		return;
	}
	while (it) {
		printf("[%d.%d.%d]:\n", it->day, it->mon, it->year);
		print_times(it->times);
		it = it->next;
	}
	printf("--------\n");
}

int read_csv_file(char *filename, struct my_date **first_day)
{
	FILE *in = fopen(filename, "r");
	char line[128] = {0};

	struct my_date *date_it = *first_day;
	if (NULL == in){
		printf("file doesn't exist");
		return 1;
	}
	while (NULL != fgets(line, sizeof(line), in)) {
		char *p = line;	
		printf("LINE: %s\n", line);
		if (0 == strcmp(line, FILE_HEADER)){
			continue;
		}	
		while (p < &line[sizeof(line) - 1]) {
//			printf("-%c.%c-", p[0], p[1]);
			if (';' == p[0] && ';' == p[1]){
				p[0] = '\n';
				char *del = &p[1];
				while (*del) {
					*del = 0;
					del++;
				}

				break;
			}
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
		struct my_date *tmp = extract_date(&len, line);
		if (NULL == tmp) 
			goto GOTO_CONTINUE;
		if (tmp->day == 1 && tmp->mon == 8 && tmp->year == 2048){
		// the day i retire. on this day i (hopefully) don't care about time taking :-)
			scan_options(line + len);
			goto GOTO_CONTINUE;
		}
		if (NULL == *first_day){
//			*first_day = extract_date(&len, line);
			*first_day = tmp;
			date_it = *first_day;
		} else {
//			date_it->next = extract_date(&len, line);
			date_it->next = tmp;
			date_it = date_it->next;
		}
		printf("line {%d}: %s\n", len, line);
		printf("date{%d.%d.%d}\n", date_it->day, date_it->mon, date_it->year);
		printf("time input: %s\n", line + len);
		struct my_time *tt = scan_time(line + len);
		date_it->times = tt;
		print_times(tt);
		memset(line, 0, sizeof(line));
		printf("---------------\n");
GOTO_CONTINUE:
;
	}
	printf("------------------------------------\n");
	//print_days(*first_day);
//	printf("------------------------------------\n");
//	write_day_to_file("output.csv", *first_day);
}
