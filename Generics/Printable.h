
#ifndef Printable_h
#define Printable_h
#if PRINTABLE_ENABLE == 1
	#include <stdlib.h>

	class Print;
		class Printable {
			public:
			virtual size_t printTo(Print &p) const = 0;
		};

	#endif
#endif

