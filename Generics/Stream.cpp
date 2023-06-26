#include "Generics.h"
#include "Stream.h"

#define PARSE_TIMEOUT 1000  // default number of milli-seconds to wait
int Stream::timedRead(){
	int c;
	_startMillis = HAL_GetTick();
	do {
		c = read();
		if (c >= 0) {
			return c;
		}
	} while (HAL_GetTick() - _startMillis < _timeout);
	return -1;     // -1 indicates timeout
}
int Stream::timedPeek(){
	int c;
	_startMillis = HAL_GetTick();
	do {
		c = peek();
		if (c >= 0) {
			return c;
		}
	} while (HAL_GetTick() - _startMillis < _timeout);
	return -1;     // -1 indicates timeout
}
int Stream::peekNextDigit(LookaheadMode lookahead, bool detectDecimal){
	int c;
	while (1) {
		c = timedPeek();

		if (c < 0 ||
			c == '-' ||
			(c >= '0' && c <= '9') ||
			(detectDecimal && c == '.')) {
			return c;
		}
		switch (lookahead) {
			case SKIP_NONE: return -1; // Fail code.
		  	case SKIP_WHITESPACE:
		    	switch (c) {
		      		case ' ':
		      		case '\t':
		      		case '\r':
		      		case '\n': break;
		      		default: return -1; // Fail code.
		    	}
		  	case SKIP_ALL:
		    	break;
		}
	  	read();
	}
}

// Public Methods
//////////////////////////////////////////////////////////////

void Stream::setTimeout(unsigned long timeout){
	_timeout = timeout;
}
bool  Stream::find(const char *target){
	return findUntil(target, strlen(target), NULL, 0);
}
bool Stream::find(const char *target, size_t length){
	return findUntil(target, length, NULL, 0);
}
bool  Stream::findUntil(const char *target, const char *terminator){
	return findUntil(target, strlen(target), terminator, strlen(terminator));
}
bool Stream::findUntil(const char *target, size_t targetLen, const char *terminator, size_t termLen){
	if (terminator == NULL) {
		MultiTarget t[1] = {{target, targetLen, 0}};
		return findMulti(t, 1) == 0 ? true : false;
	} 
	else {
		MultiTarget t[2] = {{target, targetLen, 0}, {terminator, termLen, 0}};
		return findMulti(t, 2) == 0 ? true : false;
	}
}
long Stream::parseInt(LookaheadMode lookahead, char ignore){
	bool isNegative = false;
	long value = 0;
	int c;
	c = peekNextDigit(lookahead, false);
	if (c < 0) {
		return 0; 
	}
	do {
		if (c == ignore){}
		else if (c == '-') {
			isNegative = true;
		}
		else if (c >= '0' && c <= '9') {
			value = value * 10 + c - '0';
		}
		read();
		c = timedPeek();
	} while ((c >= '0' && c <= '9') || c == ignore);
	if (isNegative) {
	  value = -value;
	}
	return value;
}
float Stream::parseFloat(LookaheadMode lookahead, char ignore){
	bool isNegative = false;
	bool isFraction = false;
	long value = 0;
	int c;
	float fraction = 1.0;
	c = peekNextDigit(lookahead, true);
	if (c < 0) {
		return 0; 
	}
	do {
		if (c == ignore){}
		else if (c == '-') {
			isNegative = true;
		} 
		else if (c == '.') {
			isFraction = true;
		} 
		else if (c >= '0' && c <= '9'){
			value = value * 10 + c - '0';
			if (isFraction) {
				fraction *= 0.1;
			}
		}
		read(); 
		c = timedPeek();
	} while ((c >= '0' && c <= '9')  || (c == '.' && !isFraction) || c == ignore);
	if (isNegative) {
		value = -value;
	}
	if (isFraction) {
		return value * fraction;
	} 
	else {
		return value;
	}
}
size_t Stream::readBytes(char *buffer, size_t length){
	size_t count = 0;
	while (count < length) {
		int c = timedRead();
		if (c < 0) {
			break;
		}
		*buffer++ = (char)c;
		count++;
	}
	return count;
}
size_t Stream::readBytesUntil(char terminator, char *buffer, size_t length){
	if (length < 1) {
		return 0;
	}
	size_t index = 0;
	while (index < length) {
		int c = timedRead();
		if (c < 0 || c == terminator) {
			break;
		}
		*buffer++ = (char)c;
		index++;
	}
	return index;
}
String Stream::readString(){
	String ret;
	int c = timedRead();
	while (c >= 0) {
		ret += (char)c;
		c = timedRead();
	}
	return ret;
}
String Stream::readStringUntil(char terminator){
	String ret;
	int c = timedRead();
	while (c >= 0 && c != terminator) {
		ret += (char)c;
		c = timedRead();
	}
	return ret;
}

int Stream::findMulti(struct Stream::MultiTarget *targets, int tCount){
	for (struct MultiTarget *t = targets; t < targets + tCount; ++t) {
		if (t->len <= 0) {
			return t - targets;
		}
	}
	while (1) {
		int c = timedRead();
		if (c < 0) {
			return -1;
		}
		for (struct MultiTarget *t = targets; t < targets + tCount; ++t) {
			if (c == t->str[t->index]) {
				if (++t->index == t->len) {
					return t - targets;
				} 
				else {
					continue;
				}
			}
			if (t->index == 0) {
				continue;
			}
			int origIndex = t->index;
			do {
				--t->index;
				if (c != t->str[t->index]) {
					continue;
				}
				if (t->index == 0) {
					t->index++;
					break;
				}
				int diff = origIndex - t->index;
				size_t i;
				for (i = 0; i < t->index; ++i) {
					if (t->str[i] != t->str[i + diff]) {
					  break;
					}
				}
				if (i == t->index) {
					t->index++;
					break;
				}
			} while (t->index);
		}
	}
	return -1;
}
