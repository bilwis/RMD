/*
 * Diagnostics.hpp
 *
 *  Created on: 14.12.2013
 *      Author: Clemens
 */

#ifndef DIAGNOSTICS_HPP_
#define DIAGNOSTICS_HPP_

#ifdef DEBUG
#define DEBUG_FLAG 1
#else
#define DEBUG_FLAG 0
#endif

/*#define debug_print(fmt, ...) \
	do { if (DEBUG_FLAG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
			__LINE__, __func__, __VA_ARGS__); } while (0)*/

#define debug_print(fmt, ...) \
            do { if (DEBUG_FLAG) fprintf(stdout, fmt, ##__VA_ARGS__); } while (0)

#endif /* DIAGNOSTICS_HPP_ */
