#include <stdint.h>
#include "SEGGER_RTT.h"
#include "nrf_delay.h"

#define ND_UTIL_SUCCEED                 0x00
#define ND_UTIL_FAILED                  0xFF

#define POLARSSL_ERR_BASE64_BUFFER_TOO_SMALL               -0x002A  /**< Output buffer too small. */
#define POLARSSL_ERR_BASE64_INVALID_CHARACTER              -0x002C  /**< Invalid character in input. */

extern uint16_t swap_uint16( uint16_t val );
extern int16_t swap_int16( int16_t val );
extern uint32_t swap_uint32( uint32_t val );
extern int32_t swap_int32( int32_t val );
extern float compensation(float data, float dt);
extern int base64_encode( unsigned char *dst, uint16_t *dlen, const unsigned char *src, uint16_t slen );
extern int base64_decode( unsigned char *dst, uint16_t *dlen, const unsigned char *src, uint16_t slen );

