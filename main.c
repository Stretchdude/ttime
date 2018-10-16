#include <stdio.h>
#include <string.h>

#include <time.h>
#include <sys/time.h>
#include <malloc.h>
#include <stdlib.h>

#define ERR_BADTOKEN 1

int target_workmin = 7*60+48;

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

struct my_time *scan_time(char *argv)
{
	struct my_time *mt = calloc(sizeof(struct my_time), 1);
	struct my_time *it = mt;	
	int numnum = 0;
	int i = 0;
	char s[256];

	strcpy(s, argv);
	char* token = strtok(s, "-");
	while (token) {
		//	printf("token: %s\n", token);

		numnum += sscanf(token, "%2d:%2d", &it->h, &it->m);
		if (0 != numnum % 2){
			printf("error with token: %s\n", token);
			exit(-ERR_BADTOKEN);
		}
		token = strtok(NULL, "-");
		if (token){
			it->next = calloc(sizeof(struct my_time), 1);
			it = it->next;
		}
	}

	return mt;
}

int t2min(const struct my_time *mt)
{
	int ret = mt->h * 60;
	ret += mt->m;
	return ret;
}

struct my_time min2t(const int mins)
{
	struct my_time t;
	t.m = mins % 60;
	t.h = mins / 60;
	t.s = 0;
	return t;
}


void just_go(const time_t *now)
{
	time_t gotime = *now + target_workmin * 60;
	struct tm *gotm = localtime(&gotime);
	char buff[128] = {0};

	strftime(buff, sizeof(buff), "%H:%M:%S", gotm);
	printf("%s\n", buff);
}

struct work_day_mins sum_up_hours(struct my_time *time, struct tm *nowtm)
{
	int i = 0;
	int minutes = 0 ;
	int breaks = 0;
	struct my_time *it = time;
	/* accumulate all times to work time or to break time in a loop */
	while (it) {
		// printf("my time: %d %d %d -> %d\n", it->h, it->m, it->s, t2min(it));
		if (i % 2 == 0){
			minutes += -1 * t2min(it);
			if (i > 1 && it->next) {
				breaks += t2min(it);
			}
		} else {
			minutes += t2min(it);
			if (it->next){
				breaks += -1 * t2min(it);
			}
		}
		it = it->next;				
		i++;
	//	printf("workm: %d - breakmin: %d\n", minutes, breaks);
	}
	/* if a time is missing assume work end time is now */
	if (i % 2 && NULL == it) {
		struct my_time mnow = { .h = nowtm->tm_hour, .m = nowtm->tm_min, .s = 0};
		printf("assume \"now\" for last time value: %02d:%02d\n", mnow.h, mnow.m);
		minutes += t2min(&mnow);
	}

	/* keep an eye on the correct break times */
	struct my_time w = min2t(minutes);
	if (breaks < 45 && minutes > 9 * 60) {
		w = min2t(minutes-45);
		printf("forcing 45 minutes break for %d minutes work!\n", minutes); 
		minutes -= 45;
	} else if (breaks < 30 && minutes > 6 * 60) {
		w = min2t(minutes-30);
		printf("forcing 30 minutes break for %d minutes work!\n", minutes); 
		minutes -= 30;
	}
	
	struct work_day_mins day = {minutes, breaks};

	return day;

}


struct work_day mins2times(int workmin, int breakmin)
{
	struct work_day day;
	day.workt  = min2t(workmin);
	day.breakt = min2t(breakmin);

	return day;
}

struct work_day_mins times2mins(const struct work_day *day)
{
	struct work_day_mins m;
	m.workmin = t2min(&day->workt);
	m.breakmin = t2min(&day->breakt);
	return m;
}

struct work_day sum_up_times(struct my_time *times, struct tm *nowtm)
{
	struct work_day_mins mins = sum_up_hours(times, nowtm);
	return mins2times(mins.workmin, mins.breakmin);
}

void go(int argc, char **argv)
{
	struct timeval time; ;
	time_t now;
	struct tm *nowtm;

	gettimeofday(&time, NULL);
	now =  time.tv_sec;
	nowtm = localtime(&now);

	if (2 == argc){
		just_go(&now);
	}
	if (3 == argc) {
		struct my_time *mt = scan_time(argv[2]);
		struct work_day_mins daym =  sum_up_hours(mt, nowtm);
		struct work_day w = mins2times(daym.workmin, daym.breakmin);
		printf ("worktime so far in min: %d -> %d:%d:%d\n", 
			daym.workmin, w.workt.h, w.workt.m, w.workt.s );
		printf("breaktime so far in min: %d -> %d:%d:%d\n",
			daym.breakmin, w.breakt.h, w.breakt.m, w.breakt.s);

		int result = daym.workmin - target_workmin;
		if (0 < result) {
			printf("worked enough today - +%d minutes\n", result);
		 } else {
			printf("still %d minutes to work\n", result);
			printf("has: %d :: must %d\n", daym.workmin, target_workmin);
		 }
	}
}

void print_help(char *me)
{
	printf("tool to collect your work hours\n");
	printf("usage: %s [cmd] [times]\n", me);
	printf(" where cmd is one of:\n");
	printf("\thelp\tprints this help msg\n");
	printf("\tgo\tcalculates time to leave for today from [times]\n");
	printf("\tadd\tadds [times] to csv sheat\n");
	printf(" where times is in the format:\n");
	printf("\t<empty>\t\t\tassumes the only time is \"now\" - taken from system time\n");
	printf("\tHH:MM\t\t\ttime of arival\n");
	printf("\tHH:MM-HH:MM\t\twork period - arival to leave\n");
	printf("\tHH-MM-HH:MM-HH:MM\tone work period and one started e.g. after lunch break\n");
	printf("\t...and so forth\t\tarival and leaving times can be concatenated at will.\n");
	printf("\t\t\t\tmissing last time will be assumed as now\n");
}

int main(int argc, char **argv)
{	
	switch (argc) {
		case 1:
			print_help(argv[0]);
			break;
		case 2:
			if (0 == strcasecmp(argv[1], "go")){
				printf("go now:\n");	
				go(argc, NULL);
			}
			if (0 == strcasecmp(argv[1], "add")){
				printf("add now\n");
			}
			if (0 == strcasecmp(argv[1], "help")){
				print_help(argv[0]);
			}
			break;
		case 3:
			if (0 == strcasecmp(argv[1], "go")){
				go(argc, argv);
			}
			break;
	}

	return 0;
}
