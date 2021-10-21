#include <string.h>

void* memcpy(void* dstptr, void* srcptr, uint64_t size) {
	uint8_t* dst = (uint8_t*) dstptr;
	uint8_t* src = (uint8_t*) srcptr;
	if (dst < src) {
		for (uint64_t i = 0; i < size; i++)
			dst[i] = src[i];
	} else {
		for (uint64_t i = size; i != 0; i--)
			dst[i-1] = src[i-1];
	}
	return dstptr;
}

void* memset(void* bufptr, uint8_t value, uint64_t size) {
	uint8_t* buf = (uint8_t*) bufptr;
	for (uint64_t i = 0; i < size; i++) {
		buf[i] = value;
	}	
	return bufptr;
}

size_t strlen(char* str){
    uint32_t len = 0;
	while (str[len])
		len++;
	return len;
}

bool strcmp(char* str1, char* str2) {
	size_t len = strlen(str1);
	if (strlen(str2) != len) {
		return false;
	}
	for (size_t i = 0; i < len; i++) {
		if (str1[i] != str2[i]) {
			return false;
		}
	}
	return true;
}

int strmatch(char* str1, char* str2) {
	int len1 = strlen(str1);
	int len2 = strlen(str2);
	int len = len1 < len2 ? len1 : len2;
	for (int i = 0; i < len; i++) {
		if (str1[i] != str2[i]) {
			return i;
		}
	}
	return len;
}

void itoa(int n, char* buf, int buflen) {
	uint32_t abs = n >= 0 ? n : -n;
    memset(buf, 0, buflen);
    int pos = buflen - 2;
    if (abs == 0) {
        buf[pos] = '0';
        pos--;
    }
    while (abs > 0) {
        buf[pos] = (abs % 10) + '0';
        pos--;
        abs /= 10;
    }
    if (n < 0) {
        buf[pos] = '-';
    }
    else {
        pos++;
    }
    char* str = &buf[pos];
	int ret_len = strlen(str);
	memcpy(&buf[0], &buf[pos], ret_len);
	buf[ret_len] = 0;
}