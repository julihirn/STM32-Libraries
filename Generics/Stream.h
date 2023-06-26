#ifndef Stream_h
#define Stream_h

#include <inttypes.h>
#include "Print.h"
enum LookaheadMode {
  SKIP_ALL,       // All invalid characters are ignored.
  SKIP_NONE,      // Nothing is skipped, and the stream is not touched unless the first waiting character is valid.
  SKIP_WHITESPACE // Only tabs, spaces, line feeds & carriage returns are skipped.
};

#define NO_IGNORE_CHAR  '\x01' // a char not found in a valid ASCII numeric field
class Stream : public Print {
	protected:
		unsigned long _timeout;      // number of milliseconds to wait for the next char before aborting timed read
		unsigned long _startMillis;  // used for timeout measurement
		int timedRead();    // private method to read stream with timeout
		int timedPeek();    // private method to peek stream with timeout
		int peekNextDigit(LookaheadMode lookahead, bool detectDecimal);

	public:
		virtual int available() = 0;
		virtual int read() = 0;
		virtual int peek() = 0;
		Stream(){ _timeout = 1000;}
		void setTimeout(unsigned long timeout);
		unsigned long getTimeout(void){ return _timeout;}
		bool find(const char *target); 
		bool find(const uint8_t *target){
			return find((const char *)target);
		}
		bool find(const char *target, size_t length);
		bool find(const uint8_t *target, size_t length){
			return find((const char *)target, length);
		}
		bool find(char target){
			return find(&target, 1);
		}
		bool findUntil(const char *target, const char *terminator); 
		bool findUntil(const uint8_t *target, const char *terminator){
			return findUntil((const char *)target, terminator);
		}
		bool findUntil(const char *target, size_t targetLen, const char *terminate, size_t termLen); 
		bool findUntil(const uint8_t *target, size_t targetLen, const char *terminate, size_t termLen){
			return findUntil((const char *)target, targetLen, terminate, termLen);
		}
		long parseInt(LookaheadMode lookahead = SKIP_ALL, char ignore = NO_IGNORE_CHAR);
		float parseFloat(LookaheadMode lookahead = SKIP_ALL, char ignore = NO_IGNORE_CHAR);
		virtual size_t readBytes(char *buffer, size_t length);  // read chars from stream into buffer
		size_t readBytes(uint8_t *buffer, size_t length){
			return readBytes((char *)buffer, length);
		}
		virtual size_t readBytesUntil(char terminator, char *buffer, size_t length);
		size_t readBytesUntil(char terminator, uint8_t *buffer, size_t length){
			return readBytesUntil(terminator, (char *)buffer, length);
		}
		String readString();
		String readStringUntil(char terminator);

	protected:
		long parseInt(char ignore){
			return parseInt(SKIP_ALL, ignore);
		}
		float parseFloat(char ignore){
			return parseFloat(SKIP_ALL, ignore);
		}
		struct MultiTarget {
			const char *str;  // string you're searching for
			size_t len;       // length of string you're searching for
			size_t index;     // index used by the search routine.
		};
		int findMulti(struct MultiTarget *targets, int tCount);
};

#undef NO_IGNORE_CHAR
#endif
