/*
 * ================================================================================
 *
 *       Filename:  printk.c
 *
 *    Description:  内核的打印函数
 *
 *        Version:  1.0
 *        Created:  2013年11月06日 12时06分00秒
 *         Author:  Hurley (LiuHuan), liuhuan1992@gmail.com
 *        Company:  Class 1107 of Computer Science and Technology
 *
 *         Update: wangguohao.2009@gmail.com
 *    Update time: Thu Apr 28 10:35:58 CST 2016
 * ================================================================================
 */
#include "printk.h"

static int vsprintf(char *buff, const char *format, va_list args);

void printk(const char *format, ...)
{
	static char buff[1024];
	va_list args;
	int i;

	va_start(args, format);
	i = vsprintf(buff, format, args);
	va_end(args);

	buff[i] = '\0';
	console_write(buff);
}

void printk_color(real_color_t back, real_color_t fore, const char *format, ...)
{
	static char buff[1024];
	va_list args;
	int i;

	va_start(args, format);
	i = vsprintf(buff, format, args);
	va_end(args);

	buff[i] = '\0';
	console_write_color(buff, back, fore);
}

static int skip_atoi(const char **s)
{
	int i = 0;
	while (is_digit(**s)) {
		i = i * 10 + *((*s)++) - '0';
	}
  return i;
}

static char *number(char *str, int num, int base, int size, int precision, int type)
{
	char c, sign, tmp[36];
	const char *digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int i;

	if (type & SMALL) {
		digits ="0123456789abcdefghijklmnopqrstuvwxyz";
	}
	if (type & LEFT) {
		type &= ~ZEROPAD;
	}
	if (base < 2 || base > 36) {
		return 0;
	}

	c = (type & ZEROPAD) ? '0' : ' ' ;

	if (type & SIGN && num < 0) {
		sign = '-';
		num = -num;
	} else {
		sign = (type&PLUS) ? '+' : ((type&SPACE) ? ' ' : 0);
	}

	if (sign) {
	      size--;
	}
	if (type & SPECIAL) {
		if (base == 16) {
			size -= 2;
		} else if (base == 8) {
			size--;
		}
	}
	i = 0;
	if (num == 0) {
		tmp[i++] = '0';
	} else {
		while (num != 0) {
			tmp[i++] = digits[do_div(num,base)];
		}
	}

	if (i > precision) {
		precision = i;
	}
	size -= precision;

	if (!(type&(ZEROPAD+LEFT))) {
		while (size-- > 0) {
			*str++ = ' ';
		}
	}
	if (sign) {
		*str++ = sign;
	}
	if (type & SPECIAL) {
		if (base == 8) {
			*str++ = '0';
		} else if (base == 16) {
			*str++ = '0';
			*str++ = digits[33];
		}
	}
	if (!(type&LEFT)) {
		while (size-- > 0) {
			*str++ = c;
		}
	}
	while (i < precision--) {
		*str++ = '0';
	}
	while (i-- > 0) {
		*str++ = tmp[i];
	}
	while (size-- > 0) {
		*str++ = ' ';
	}
	return str;
}

static int vsprintf(char *buff, const char *format, va_list args)
{
	int len = 0;
  int i = 0;
	char *str = NULL;
	char *s = NULL;
	int *ip = NULL;
	int flags = 0x0000;		// flags to number()

	int field_width = 0;	// width of output field
	int precision = 0;		// min. # of digits for integers; max number of chars for from string

	for (str = buff ; *format ; ++format) {
		if (*format != '%') {
			*str++ = *format;
			continue;
		}

		flags = 0;
		repeat:
			++format;		// this also skips first '%'
			switch (*format) {
				case '-': flags |= LEFT;
					  goto repeat;

				case '+': flags |= PLUS;
					  goto repeat;

				case ' ': flags |= SPACE;
					  goto repeat;

				case '#': flags |= SPECIAL;
					  goto repeat;

				case '0': flags |= ZEROPAD;
					  goto repeat;
			}

		// get field width
		field_width = -1;
		if (is_digit(*format)) {
			field_width = skip_atoi(&format);
		} else if (*format == '*') {
			// it's the next argument
			field_width = va_arg(args, int);
			if (field_width < 0) {
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		// get the precision
		precision = -1;
		if (*format == '.') {
			++format;
			if (is_digit(*format)) {
				precision = skip_atoi(&format);
			} else if (*format == '*') {
				// it's the next argument
				precision = va_arg(args, int);
			}
			if (precision < 0) {
				precision = 0;
			}
		}

		// get the conversion qualifier
		if (*format == 'h' || *format == 'l' || *format == 'L') {
					++format;
		}

		switch (*format) {

		case 'c':
			if (!(flags & LEFT)) {
				while (--field_width > 0) {
					*str++ = ' ';
				}
			}
			*str++ = (unsigned char) va_arg(args, int);
			while (--field_width > 0) {
				*str++ = ' ';
			}
			break;

		case 's':
			s = va_arg(args, char *);
			len = strlen(s);

			if (precision < 0) {
				precision = len;
			} else if (len > precision) {
				len = precision;
			}
			if (!(flags & LEFT)) {
				while (len < field_width--) {
					*str++ = ' ';
				}
			}
			for (i = 0; i < len; ++i) {
				*str++ = *s++;
			}
			while (len < field_width--) {
				*str++ = ' ';
			}
			break;

		case 'o':
			str = number(str, va_arg(args, unsigned long), 8,
				field_width, precision, flags);
			break;

		case 'p':
			if (field_width == -1) {
				field_width = 8;
				flags |= ZEROPAD;
			}
			str = number(str, (unsigned long) va_arg(args, void *), 16,
				field_width, precision, flags);
			break;

		case 'x':
			flags |= SMALL;

		case 'X':
			str = number(str, va_arg(args, unsigned long), 16,
				field_width, precision, flags);
			break;

		case 'd':

		case 'i':
			flags |= SIGN;

		case 'u':
			str = number(str, va_arg(args, unsigned long), 10,
				field_width, precision, flags);
			break;

		case 'b':
			str = number(str, va_arg(args, unsigned long), 2,
				field_width, precision, flags);
			break;

		case 'n':
			ip = va_arg(args, int *);
			*ip = (str - buff);
			break;

		default:
			if (*format != '%')
				*str++ = '%';
			if (*format) {
				*str++ = *format;
			} else {
				--format;
			}
			break;
		}
	}
	*str = '\0';
	return (str -buff);
}
