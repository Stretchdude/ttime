#ifndef HEADER_H_INCLUDE_WATCHER__
#define HEADER_H_INCLUDE_WATCHER__

#include <time.h>
#include <sys/time.h>

#define TARGET_WORK_MIN (7*60+48)


#include <stdio.h>
#define HERE(...) do {printf("%s:%s:%d\n", __FILE__, __func__, __LINE__);} while(0)
enum ttime_err {
	ERR_NOERR = 0,
	ERR_BADTOKEN = 1,
};


//int target_workmin = 7*60+48;


struct my_time {
	int h;
	int m;
	int s;
	struct my_time *next;
};

struct work_day {
	struct my_time workt;
	struct my_time breakt;
};

struct work_day_mins {
	int workmin;
	int breakmin;
};

struct my_date {
	int day;
	int mon;
	int year;
	struct my_time *times;
	struct my_date *next;
};


int read_csv_file(char *filename, struct my_date **first_day);
struct work_day sum_up_times(struct my_time *times, struct tm *nowtm);
struct tm *get_now();
int t2min(const struct my_time *mt);
void print_list(struct my_time *first);
struct my_time *scan_time(char *argv);

void add_now_as_time(struct my_date *days);
void write_days_to_file(char *filename, struct my_date *date);
void print_days(struct my_date *days);

struct my_date *get_today();

;
#endif
