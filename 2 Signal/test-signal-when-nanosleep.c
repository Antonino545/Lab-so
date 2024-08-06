#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include <signal.h>
#include <time.h>

/* #define MUST_SLEEP_EXACT_TIME */

#define		SEC_TO_SLEEP	10

void handle_sig(int signum) {

	printf("Got signal %d when sleeping (waiting state)\n",
	       signum);
}

int main (int argc, char * argv[]) {
	struct sigaction sa;
	struct timespec this_time, next_time, diff_time;
	int ret_val;
	
	/* Setting the handler of Ctrl+C (SIGINT) */
	bzero(&sa, sizeof(sa));
	sa.sa_handler = handle_sig;
	sigaction(SIGINT, &sa, NULL);
	
	/* Setting the sleep time */
	this_time.tv_sec = SEC_TO_SLEEP;
	this_time.tv_nsec = 0;
	
	do {
		/* Try to sleep for this_time */
		ret_val = nanosleep(&this_time, &next_time);
		if (ret_val == 0) {
			/* nanosleep was NOT INTERRUPTED by a signal */
			diff_time = this_time;
		} else {
			/* nanosleep was INTERRUPTED by a signal */
			/* Computing the difference between the this_time and next_time */
			diff_time.tv_nsec = this_time.tv_nsec-next_time.tv_nsec;
			if (diff_time.tv_nsec < 0) {
				/* Carry 1e9 nanoseconds */
				diff_time.tv_nsec += 1000000000;
				diff_time.tv_sec = this_time.tv_sec-next_time.tv_sec-1;
			} else {
				diff_time.tv_sec = this_time.tv_sec-next_time.tv_sec;
			}
		}
		printf("Time spent in sleeping: %ld.%09ld\n",
			diff_time.tv_sec, diff_time.tv_nsec);
#ifndef	MUST_SLEEP_EXACT_TIME
	} while (0);
#else
		/* Sleep again is nanosleep was interrupted */
		this_time = next_time;
	} while (ret_val == -1 && errno == EINTR);
#endif
}

