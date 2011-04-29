/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * Copyright (C) 2009-2011 by Setuper
 * E-Mail: setuper at gmail dot com (setuper@gmail.com)

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "StringToArg.h"

#ifdef _WIN32

namespace utils {

#define strdup _strdup
#pragma warning(disable:4127) // Disable "conditional expression is constant"


int StringToArg::String2Arg(char const * str, int * argc_p, char *** argv_p) {
	int argc = 0, act = 10, err;
	char ** res = (char **)malloc(sizeof(char *) * 10);
	char ** argv = res;
	char *scan, *dest;
	while (isspace((unsigned char)*str)) {
		str++;
	}
	str = scan = strdup(str);
	while (true) {
		while (isspace((unsigned char)*scan)) {
			scan++;
		}
		if (*scan == '\0') {
			break;
		}
		if (++argc >= act) {
			act += act / 2;
			res = (char **) realloc(res, act * sizeof(char *));
			argv = res + (argc - 1);
		}
		*(argv++) = dest = scan;
		while (true) {
			char ch = *(scan++);
			switch (ch) {

				case '\0' :
					goto done;

				case '\\' :
					*(dest++) = ch;
					if ((*dest = *scan) == '\0') {
						goto done;
					}
					break;

				case '\'':
					err = CopyRawString(&dest, &scan);
					if (err != 0) {
						goto error_leave;
					}
					break;

				case '"':
					err = CopyCookedString(&dest, &scan);
					if (err != 0) {
						goto error_leave;
					}
					break;

				case ' ':
				case '\t':
				case '\n':
				case '\f':
				case '\r':
				case '\v':
				case '\b':
					goto token_done;

				default:
					*(dest++) = ch;

			}
		}

token_done:
		*dest = '\0';
	}

done:
	*argv_p = res;
	*argc_p = argc;
	*argv = NULL;
	if (argc == 0) {
		free((void *)str);
	}
	return 0;

error_leave:
	free(res);
	free((void *)str);
	return err;
}

int StringToArg::CopyRawString(char ** dest, char ** src) {

	while (true) {
		char ch = *((*src)++);

		switch (ch) {

			case '\0' :
				return -1;

			case '\'' :
				*(*dest) = '\0';
				return 0;

			case '\\' :
				ch = *((*src)++);

				switch (ch) {

					case '\0' :
						return -1;

					default:
						*((*dest)++) = '\\';
						break;

					case '\\' :
					case '\'' :
						break;

				}

			default :
				*((*dest)++) = ch;
				break;

		}
	}
}

int StringToArg::CopyCookedString(char ** dest, char ** src) {

	while (true) {
		char ch = *((*src)++);

		switch (ch) {

			case '\0' :
				return -1;

			case '"' :
				*(*dest) = '\0';
				return 0;

			default :
				*((*dest)++) = ch;
				break;

		}

	}
}

}; // namespace utils

#endif // _WIN32

/**
* $Id$
* $HeadURL$
*/
