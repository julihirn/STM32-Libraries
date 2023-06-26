#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "Generics.h"

#include "Print.h"

#if defined (VIRTIO_LOG)
  #include "virtio_log.h"
#endif

// Public Methods //////////////////////////////////////////////////////////////

/* default implementation: may be overridden */
size_t Print::write(const uint8_t *buffer, size_t size){
	size_t n = 0;
	while (size--) {
		if (write(*buffer++)) {
			n++;
		} 
		else {
			break;
		}
	}
	return n;
}

size_t Print::print(const __FlashStringHelper *ifsh){
	return print(reinterpret_cast<const char *>(ifsh));
}

size_t Print::print(const String &s){
	return write(s.c_str(), s.length());
}

size_t Print::print(const char str[]){
	return write(str);
}

size_t Print::print(char c){
	return write(c);
}

size_t Print::print(unsigned char b, int base){
	return print((unsigned long) b, base);
}

size_t Print::print(int n, int base){
	return print((long) n, base);
}

size_t Print::print(unsigned int n, int base){
	return print((unsigned long) n, base);
}

size_t Print::print(long n, int base){
	if (base == 0) {
		return write(n);
	} 
	else if (base == 10) {
		if (n < 0) {
		  int t = print('-');
		  n = -n;
		  return printNumber(n, 10) + t;
		}
		return printNumber(n, 10);
	} 
	else {
		return printNumber(n, base);
	}
}

size_t Print::print(unsigned long n, int base){
	if (base == 0) {
	  	return write(n);
	} 
	else {
	  	return printNumber(n, base);
	}
}

size_t Print::print(long long n, int base){
	if (base == 0) {
		return write(n);
	} 
	else if (base == 10) {
		if (n < 0) {
			int t = print('-');
			n = -n;
			return printULLNumber(n, 10) + t;
		}
		return printULLNumber(n, 10);
	} 
	else {
	  return printULLNumber(n, base);
	}
}

size_t Print::print(unsigned long long n, int base){
	if (base == 0) {
		return write(n);
	} 
	else {
		return printULLNumber(n, base);
	}
}

size_t Print::print(double n, int digits){
	return printFloat(n, digits);
}

size_t Print::println(const __FlashStringHelper *ifsh){
	size_t n = print(ifsh);
	n += println();
	return n;
}

size_t Print::print(const Printable &x){
	return x.printTo(*this);
}

size_t Print::println(void){
	return write("\r\n");
}

size_t Print::println(const String &s){
	size_t n = print(s);
	n += println();
	return n;
}

size_t Print::println(const char c[]){
	size_t n = print(c);
	n += println();
	return n;
}

size_t Print::println(char c){
	size_t n = print(c);
	n += println();
	return n;
}

size_t Print::println(unsigned char b, int base){
	size_t n = print(b, base);
	n += println();
	return n;
}

size_t Print::println(int num, int base){
	size_t n = print(num, base);
	n += println();
	return n;
}

size_t Print::println(unsigned int num, int base){
	size_t n = print(num, base);
	n += println();
	return n;
}

size_t Print::println(long num, int base){
	size_t n = print(num, base);
	n += println();
	return n;
}

size_t Print::println(unsigned long num, int base){
	size_t n = print(num, base);
	n += println();
	return n;
}

size_t Print::println(long long num, int base){
	size_t n = print(num, base);
	n += println();
	return n;
}

size_t Print::println(unsigned long long num, int base){
	size_t n = print(num, base);
	n += println();
	return n;
}

size_t Print::println(double num, int digits){
	size_t n = print(num, digits);
	n += println();
	return n;
}

size_t Print::println(const Printable &x){
	size_t n = print(x);
	n += println();
	return n;
}

extern "C" {__attribute__((weak)) int _write(int file, char *ptr, int len){
    switch (file) {
    	case STDOUT_FILENO:
    	case STDERR_FILENO:
    	  /* Used for core_debug() */
#if defined (VIRTIO_LOG)
        virtio_log((uint8_t *)ptr, (uint32_t)len);
#elif defined(HAL_UART_MODULE_ENABLED) && !defined(HAL_UART_MODULE_ONLY)
        //uart_debug_write((uint8_t *)ptr, (uint32_t)len);
#endif
			break;
		case STDIN_FILENO:
		    break;
		default:
		    ((class Print *)file)->write((uint8_t *)ptr, len);
		    break;
		}
		return len;
	}
}
int Print::printf(const char *format, ...){
	va_list ap;
	va_start(ap, format);
	int retval = vdprintf((int)this, format, ap);
	va_end(ap);
	return retval;
}
int Print::printf(const __FlashStringHelper *format, ...){
	va_list ap;
	va_start(ap, format);
	int retval = vdprintf((int)this, (const char *)format, ap);
	va_end(ap);
	return retval;
}

// Private Methods /////////////////////////////////////////////////////////////

size_t Print::printNumber(unsigned long n, uint8_t base){
	char buf[8 * sizeof(long) + 1];
	char *str = &buf[sizeof(buf) - 1];
	*str = '\0';
	if (base < 2) {
	  base = 10;
	}
	do {
		unsigned long m = n;
		n /= base;
		char c = m - base * n;
		*--str = c < 10 ? c + '0' : c + 'A' - 10;
	} while (n);

	return write(str);
}
size_t Print::printULLNumber(unsigned long long n64, uint8_t base){
	char buf[64];
	uint8_t i = 0;
	uint8_t innerLoops = 0;
	if (base < 2) {
		base = 10;
	}
	uint16_t top = 0xFFFF / base;
	uint16_t th16 = 1;
	while (th16 < top) {
		th16 *= base;
		innerLoops++;
	}
	while (n64 > th16) {
		uint64_t q = n64 / th16;
		uint16_t r = n64 - q * th16;
		n64 = q;
		for (uint8_t j = 0; j < innerLoops; j++) {
		  uint16_t qq = r / base;
		  buf[i++] = r - qq * base;
		  r = qq;
		}
	}
	uint16_t n16 = n64;
	while (n16 > 0) {
		uint16_t qq = n16 / base;
		buf[i++] = n16 - qq * base;
		n16 = qq;
	}
	size_t bytes = i;
	for (; i > 0; i--) {
	  write((char)(buf[i - 1] < 10 ? '0' + buf[i - 1] : 'A' + buf[i - 1] - 10));
	}
	return bytes;
}
size_t Print::printFloat(double number, uint8_t digits){
	size_t n = 0;

	if (isnan(number)) {
		return print("nan");
	}
	if (isinf(number)) {
		return print("inf");
	}
	if (number > 4294967040.0) {
		return print("ovf");
	}
	if (number < -4294967040.0) {
		return print("ovf");
	}
	if (number < 0.0) {
		n += print('-');
		number = -number;
	}
	double rounding = 0.5;
	for (uint8_t i = 0; i < digits; ++i) {
		rounding /= 10.0;
	}
	number += rounding;
	unsigned long int_part = (unsigned long)number;
	double remainder = number - (double)int_part;
	n += print(int_part);
	if (digits > 0) {
	  n += print('.');
	}
	while (digits-- > 0) {
	  remainder *= 10.0;
	  unsigned int toPrint = (unsigned int)remainder;
	  n += print(toPrint);
	  remainder -= toPrint;
	}
	return n;
}
