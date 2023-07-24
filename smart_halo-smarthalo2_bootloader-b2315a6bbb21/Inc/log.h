#ifndef _LOG_H_
#define _LOG_H_

#ifdef DEBUG
#include <OSS/printf.h>

// Caller must use a format with at least one argument
#define ERR(fmt, ...) \
	do { \
		printf("%s:%d:%s " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
	} while (0)

#define LOG(...)	printf(__VA_ARGS__)

#else

#define ERR(...)
#define LOG(...)
#endif

#endif // _LOG_H_
