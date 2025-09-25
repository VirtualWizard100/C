#include <asm/byteorder.h>


struct IP_Header {
#if defined(__BIG_ENDIAN_BITFIELD)
	uint8_t ip_version:4, ip_ihl:4;
	uint8_t ip_precedence:3, ip_delay:1, ip_throughput:1, ip_relibility:1, ip_padding:2;
#elif defined(__LITTLE_ENDIAN_BITFIELD)
	uint8_t ip_ihl:4, ip_version:4;
	uint8_t ip_padding:2, ip_relibility:1, ip_throughput:1, ip_delay:1, ip_precedence:3;
#endif
	uint16_t ip_total_len;	//Total Length of Packet in bytes
	uint16_t ip_ident;
//#if defined(__BIG_ENDIAN_BITFIELD)
//	uint16_t ip_reserved:1, ip_df:1, ip_mf:1, ip_frag_off:13;
//#elif defined(__LITTLE_ENDIAN_BITFIELD)
//	uint16_t ip_frag_off:13, ip_mf:1, ip_df:1, ip_reserved:1;
	uint16_t ip_frag_off_res_df_mf;
//#endif
	uint8_t ip_ttl;
	uint8_t ip_proto;
	uint16_t ip_chksum;
	uint32_t ip_src_addr;
	uint32_t ip_dst_addr;
	uint8_t ip_options[];
};

#define FRAG_OFF(frag_off_res_df_mf) ((frag_off_res_df_mf >> 8) & 0x1FFF)
#define DF(frag_off_res_df_mf) ((frag_off_res_df_mf & 0x0040) >> 6)
#define MF(frag_off_res_df_mf) ((frag_off_res_df_mf & 0x0020) >> 5)
#define RESERVED(frag_off_res_df_mf) ((frag_off_res_df_mf & 0x0010) >> 4)
/* Deprecated
//ip_v_ihl
#define VERSION(v_ihl) ((v_ihl & 0xF0) >> 4)
#define INTERNET_HEADER_LENGTH(v_ihl) (v_ihl & 0x0F)

//ip_tos
#define PRECEDENCE(tos) (tos & 0xE0)
#define DELAY(tos) (tos & 0x10)
#define THROUGHPUT(tos) (tos & 0x08)
#define RELIBILITY(tos) (tos & 0x04)

//ip_flgs_frag_off
#define FLAGS(flgs_frag_off) (flgs_frag_off & 0xE000)
#define FRAGMENT_OFFSET(flgs_frag_off) (flgs_frag_off & 0x1FFF)


*/
