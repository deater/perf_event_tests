#include <stdint.h>
#include <string.h>

/* FIXME: endian aware */
uint16_t get_uint16(unsigned char *data, int offset) {

	uint16_t temp;

	temp=data[offset] | (data[offset+1]<<8);

	return temp;
}

/* FIXME: endian aware */
uint32_t get_uint32(unsigned char *data, int offset) {

	uint32_t temp;

	temp=data[offset] | (data[offset+1]<<8)
		| (data[offset+2]<<16) | (data[offset+3]<<24);

	return temp;
}

/* FIXME: endian aware */
uint64_t get_uint64(unsigned char *data, int offset) {

	uint64_t temp;

	temp=	(((uint64_t)data[offset+0])<< 0) |
		(((uint64_t)data[offset+1])<< 8) |
		(((uint64_t)data[offset+2])<<16) |
		(((uint64_t)data[offset+3])<<24) |
		(((uint64_t)data[offset+4])<<32) |
		(((uint64_t)data[offset+5])<<40) |
		(((uint64_t)data[offset+6])<<48) |
		(((uint64_t)data[offset+7])<<56);
	return temp;
}


/* FIXME: check for overflow */
uint32_t get_string(unsigned char *data, int offset, char *string) {

	uint32_t len,total=0;

	len=get_uint32(data,offset);
	total+=4;

	memcpy(string,data+offset+4,len);
	total+=len;

	return total;

}



