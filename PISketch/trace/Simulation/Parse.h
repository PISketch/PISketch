#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <cstring>
#include <algorithm>
#include <set>
//#include <fstream>
// 0xD4C3B2A1,默认是反向读的

#define PcapHeaderSize 24
#define MaxDataSize 100000

uint32_t ReverseUint32(uint8_t *str)
{
	uint32_t num = 0;
	num |= str[0];
	num |= str[1] << 8;
	num |= str[2] << 16;
	num |= str[3] << 24;
	return num;
}
uint32_t UnReverseUint32(uint8_t *str)
{
	uint32_t num = 0;
	num |= str[3];
	num |= str[2] << 8;
	num |= str[1] << 16;
	num |= str[0] << 24;
	return num;
}
uint16_t UnReverseUint16(uint8_t *str)
{
	uint16_t num = 0;
	num |= str[0];
	num |= str[1] << 8;
	return num;
}


std::set<uint32_t> Extract5Tuples(char const*src_file_name)
{
	printf("Start splitting...\n");
	printf("Dealing with %s\n", src_file_name);
	
	FILE *fin = fopen("../raw/attack/9002.pcap", "rb");
	//FILE *fin = fopen(src_file_name, "rb");
	FILE *fout = fopen("./sim/src_atk.dat", "wb");
	//FILE *fout = fopen("./sim/src.dat", "wb");


	int IPv4_cnt = 0;
	int IPv4_cnt_total = 0;
	uint8_t PcapFileHeader[PcapHeaderSize];
	uint8_t data[MaxDataSize];
	uint8_t five_tuple[13];
	uint8_t readIn = 0;
	int FileEndFlag = 0;
	readIn += fread((void*)PcapFileHeader, sizeof(uint8_t), PcapHeaderSize, fin);
	std::set<uint32_t> flowID;
	while(1)
	{
	/* 先读取packet header */
		uint8_t str[4];
		uint32_t num[4];
		for(int i = 0; i < 4; ++i)
		{
			int _left_bytes = 0;
			if((_left_bytes = fread((void*)str, sizeof(uint8_t), 4, fin)) != 4)
			{
				FileEndFlag = 1;
				if(_left_bytes != 0)
				{
					printf("Wrong left bytes!");
					exit(1);
				}
				break;
			}
			readIn += _left_bytes;
			num[i] = ReverseUint32(str);
		}
		if(FileEndFlag == 1)
			break;

		uint32_t timestamp = num[0];
		uint32_t caplen = num[2];
		uint32_t len = num[3];
		
	/* 然后读取data部分 */
		if(fread((void*)data, sizeof(uint8_t), caplen, fin) != caplen) {
			printf(">>> error on reading data\n");
			exit(1);
		}
		
		// if(!((data[14]>>4) == 4 && caplen >= 24)) //前面少读好多字节
		
		// if(!((data[0]>>4) == 4 && caplen >= 24))	// 如果不是IPv4包或者不合格，那么就跳过{
		// 	continue;
		// uint8_t Protocol_Type = data[9];
		// if(Protocol_Type != 6 && Protocol_Type != 17)
		// 	continue;
		// uint16_t SrcPort = UnReverseUint16(data + 20);
		// uint16_t DstPort = UnReverseUint16(data + 22);

		// five_tuple[0] = data[12];	five_tuple[1] = data[13];
		// five_tuple[2] = data[14];	five_tuple[3] = data[15];
		// five_tuple[4] = (SrcPort >> 8) & 0x00FF;
		// five_tuple[5] = SrcPort & 0x00FF;
		// five_tuple[6] = data[16];	five_tuple[7] = data[17];
		// five_tuple[8] = data[18];	five_tuple[9] = data[19];
		// five_tuple[10] = (DstPort >> 8) & 0x00FF;
		// five_tuple[11] = DstPort & 0x00FF;
		// five_tuple[12] = data[9];

		if(!((data[14]>>4) == 4 && caplen >= 24))	// 如果不是IPv4包或者不合格，那么就跳过{
			continue;
		uint8_t Protocol_Type = data[23];
		if(Protocol_Type != 6 && Protocol_Type != 17)
			continue;
		uint16_t SrcPort = UnReverseUint16(data + 34);
		uint16_t DstPort = UnReverseUint16(data + 36);

		five_tuple[0] = data[26];	five_tuple[1] = data[27];
		five_tuple[2] = data[28];	five_tuple[3] = data[29];
		five_tuple[4] = (SrcPort >> 8) & 0x00FF;
		five_tuple[5] = SrcPort & 0x00FF;
		five_tuple[6] = data[30];	five_tuple[7] = data[31];
		five_tuple[8] = data[32];	five_tuple[9] = data[33];
		five_tuple[10] = (DstPort >> 8) & 0x00FF;
		five_tuple[11] = DstPort & 0x00FF;
		five_tuple[12] = data[23];



		fwrite(five_tuple, 1, 13, fout);
		flowID.insert(*(uint32_t *)five_tuple);
		// printf("(%u.%u.%u.%u:%u, %u.%u.%u.%u:%u, protocol=%u)\n",
		//  	five_tuple[0], five_tuple[1], five_tuple[2], five_tuple[3],
		//  	*(uint16_t*)(five_tuple + 4),
		//  	five_tuple[6], five_tuple[7], five_tuple[8], five_tuple[9],
		//  	*(uint16_t*)(five_tuple + 10),
		//  	five_tuple[12]);
		// flowID.insert(*(uint32_t *)five_tuple);
		// if (flowID.size() >= 10000)
		// {
		// 	break;
		// }
		IPv4_cnt++;
	}
	
	IPv4_cnt_total += IPv4_cnt;
	fclose(fout);
	fclose(fin);
}
