#ifndef _ATT_PCAP_PARSER_
#define _ATT_PCAP_PARSER_

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
#define MAX_PKT_NUM 100000000
#define MAX_WIN_NUM 1000
#define file_num(time_step) ((int)(60 / time_step))

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


void GetFlowInformation(char *dataDir, std::vector<uint8_t *> &flows)
{
	FILE *fp = fopen(dataDir,"rb");
	char five_tuple[13];
	while (fread(five_tuple, 1, 13, fp) == 13)
	{
		flows.push_back((uint8_t *)five_tuple);
	}
	fclose(fp);

}


int Extract5Tuples(char const*src_file_name, int fileNoBegin, uint32_t time_step, uint32_t max_pkt_num, uint32_t max_win_num, char *dst_file_dir)
{
	printf("Dealing with %s\n", src_file_name);
	
	FILE *fin = fopen(src_file_name, "rb");
	char outFileName[100] = {0};
	sprintf(outFileName, "%s/%d.dat",dst_file_dir,fileNoBegin);
	FILE *fout = fopen(outFileName, "wb");

	int IPv4_cnt = 0;
	int IPv4_cnt_total = 0;

	uint8_t PcapFileHeader[PcapHeaderSize];
	uint8_t data[MaxDataSize];
	uint8_t five_tuple[13];
	uint8_t readIn = 0;
	int FileEndFlag = 0;

	readIn += fread((void*)PcapFileHeader, sizeof(uint8_t), PcapHeaderSize, fin);

	uint32_t start_time_stamp = 0;
	uint32_t file_cnt = 1;
	int last_time_stamp = 0;
	//int end_time_stamp = 0;
	//std::vector<uint32_t> real_attackers;
	std::set<uint32_t> real_attackers;
	while (IPv4_cnt_total + IPv4_cnt < max_pkt_num)
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


		uint32_t timestamp_s = num[0];
		uint32_t timestamp_us = num[1];
		uint32_t caplen = num[2];
		uint32_t len = num[3];



		if(start_time_stamp == 0)
			start_time_stamp = timestamp_s;

		uint32_t timestamp = (timestamp_s - start_time_stamp) * 1000000 + timestamp_us;
		if(timestamp > max_win_num * time_step)
			break;


		if(timestamp >= file_cnt * time_step)
		{
			while(timestamp >= file_cnt * time_step)
			{
				fclose(fout);
				memset(outFileName, 0, 100);
				sprintf(outFileName, "%s/%d.dat",dst_file_dir, fileNoBegin + file_cnt);
			
				fout = fopen(outFileName, "wb");
				file_cnt++;
				//printf("Valid Ipv4/6 packets: %d\n", IPv4_cnt);
				IPv4_cnt_total += IPv4_cnt;
				IPv4_cnt = 0;
				//printf(">>> filling %s\t", outFileName);
			}
		}

		if(last_time_stamp != timestamp)
		{
			last_time_stamp = timestamp;
		}
		
	/* 然后读取data部分 */
		if(fread((void*)data, sizeof(uint8_t), caplen, fin) != caplen) {
			printf(">>> error on reading data\n");
			exit(1);
		}


		//if(!((data[14]>>4) == 4 && caplen >= 24)) 前面少读好多字节
		
		if(!(((data[0]>>4) == 4 || (data[0]>>4) == 4) && caplen >= 24))	// 如果不是IPv4包或者不合格，那么就跳过{
			continue;
		uint8_t Protocol_Type = data[9];
		if(Protocol_Type != 6 && Protocol_Type != 17)
			continue;
		uint16_t SrcPort = UnReverseUint16(data + 20);
		uint16_t DstPort = UnReverseUint16(data + 22);

		five_tuple[0] = data[12];	five_tuple[1] = data[13];
		five_tuple[2] = data[14];	five_tuple[3] = data[15];
		five_tuple[4] = (SrcPort >> 8) & 0x00FF;
		five_tuple[5] = SrcPort & 0x00FF;
		five_tuple[6] = data[16];	five_tuple[7] = data[17];
		five_tuple[8] = data[18];	five_tuple[9] = data[19];
		five_tuple[10] = (DstPort >> 8) & 0x00FF;
		five_tuple[11] = DstPort & 0x00FF;
		five_tuple[12] = data[9];
		/*

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
		*/
		
		fwrite(five_tuple, 1, 13, fout);
		//real_attackers.push_back(*((uint32_t *)five_tuple));
		real_attackers.insert(*((uint32_t *)five_tuple));
		// printf("(%u.%u.%u.%u:%u, %u.%u.%u.%u:%u, protocol=%u)\n",
		 //  	five_tuple[0], five_tuple[1], five_tuple[2], five_tuple[3],
		 //  	*(uint16_t*)(five_tuple + 4),
		 //  	five_tuple[6], five_tuple[7], five_tuple[8], five_tuple[9],
		 // 	*(uint16_t*)(five_tuple + 10),
		 //  	five_tuple[12]);	 
		IPv4_cnt++;

	}
	IPv4_cnt_total += IPv4_cnt;

	/*
	while(++last_time_stamp < end_time_stamp)
	{
		if(last_time_stamp == file_cnt * time_step)
		{
			fclose(fout);
			memset(outFileName, 0, 100);
			//sprintf(outFileName, "%s%d.dat", dst_file_prefix, fileNoBegin + file_cnt);
			sprintf(outFileName, "%s/%d.dat",dst_file_dir, fileNoBegin + file_cnt);

			fout = fopen(outFileName, "wb");
			file_cnt++;
			printf("Valid Ipv4/6 packet: %d\n", IPv4_cnt);
			IPv4_cnt = 0;
			printf(">>> filling %s\t", outFileName);
		}
	}
	*/

	fclose(fout);
	fclose(fin);

	// int cup = 0;
	// int cup_label;
	// int bottle = 0;
	// int bottle_label;
	// int num;
	// if ((int)real_attackers.size() != 0)
	// {

	// 	for (int i = 0; i < (int)real_attackers.size(); i++)
	// 	{
	// 		num = count(real_attackers.begin(), real_attackers.end(), real_attackers[i]);
	// 		if (num > cup)
	// 		{
	// 			bottle = cup;
	// 			cup = num;
	// 			bottle_label = cup_label;
	// 			cup_label = i;
	// 		}
	// 		else if (num > bottle)
	// 		{
	// 			if (num != cup)
	// 			{
	// 				bottle = num;
	// 				bottle_label = i;
	// 			}
	// 		}
	// 	}
	// } 
	// else 
	// {
	// 	printf("No attacker exits.\n");
	// 	exit(1);
	// }

	/*
	char outFileDir [100] = {0}; 
	sprintf(outFileDir, "%s/list.dat", dst_file_dir);
	//FILE *ra = fopen("./test/list.dat", "ab+");
	FILE *ra = fopen(outFileDir, "ab+");
	uint32_t *tmp;
	tmp = (uint32_t *)malloc(sizeof(uint32_t));

	std::set<uint32_t>::iterator iter = real_attackers.begin();
    while(iter != real_attackers.end())
    {
        *tmp = *iter;
		fwrite(tmp, 1, 13, ra);
        ++iter;	
    }

   // printf("%d\n", cnt);
	// *tmp = real_attackers[cup_label];
	// fwrite(tmp, 1, 13, ra);
	// //fwrite(tmp, sizeof(uint32_t), 1, ra);
	// *tmp = real_attackers[bottle_label];
	// fwrite(tmp, 1, 13, ra);

	//fwrite(tmp, sizeof(uint32_t), 1, ra);
	//printf("%u\n", real_attackers[cup_label]);
	//printf("%u\n", real_attackers[bottle_label]);
	fclose(ra);
	*/
	printf("Attacker recording completed.\n");
	

	// size/2 : communication between attacker and victim
	printf("Total Valid Ipv4/6 packet:\t %d\n", IPv4_cnt_total);
	//printf("begin file no: %d,\tend file no: %d\n", fileNoBegin, fileNoBegin + file_cnt - 1);
	printf("Total File Amount:\t%d\n", fileNoBegin + file_cnt);
	printf("\n");
	
	return fileNoBegin + file_cnt;
}


int Extract5Tuples(char const*src_file_name, char *dst_file_dir)
{
	printf("Start splitting...\n");
	printf("Dealing with %s\n", src_file_name);
	
	FILE *fin = fopen(src_file_name, "rb");
	char outFileName[100] = {0};
	sprintf(outFileName, "%s",dst_file_dir);
	FILE *fout = fopen(outFileName, "wb");

	int IPv4_cnt = 0;
	int IPv4_cnt_total = 0;

	uint8_t PcapFileHeader[PcapHeaderSize];
	uint8_t data[MaxDataSize];
	uint8_t five_tuple[13];
	uint8_t readIn = 0;
	int FileEndFlag = 0;

	readIn += fread((void*)PcapFileHeader, sizeof(uint8_t), PcapHeaderSize, fin);

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
		
		//if(!((data[14]>>4) == 4 && caplen >= 24)) 前面少读好多字节
		
		if(!((data[0]>>4) == 4 && caplen >= 24))	// 如果不是IPv4包或者不合格，那么就跳过{
			continue;
		uint8_t Protocol_Type = data[9];
		if(Protocol_Type != 6 && Protocol_Type != 17)
			continue;
		uint16_t SrcPort = UnReverseUint16(data + 20);
		uint16_t DstPort = UnReverseUint16(data + 22);

		five_tuple[0] = data[12];	five_tuple[1] = data[13];
		five_tuple[2] = data[14];	five_tuple[3] = data[15];
		five_tuple[4] = (SrcPort >> 8) & 0x00FF;
		five_tuple[5] = SrcPort & 0x00FF;
		five_tuple[6] = data[16];	five_tuple[7] = data[17];
		five_tuple[8] = data[18];	five_tuple[9] = data[19];
		five_tuple[10] = (DstPort >> 8) & 0x00FF;
		five_tuple[11] = DstPort & 0x00FF;
		five_tuple[12] = data[9];

		// if(!((data[14]>>4) == 4 && caplen >= 24))	// 如果不是IPv4包或者不合格，那么就跳过{
		// 	continue;
		// uint8_t Protocol_Type = data[23];
		// if(Protocol_Type != 6 && Protocol_Type != 17)
		// 	continue;
		// uint16_t SrcPort = UnReverseUint16(data + 34);
		// uint16_t DstPort = UnReverseUint16(data + 36);
		// five_tuple[0] = data[26];	five_tuple[1] = data[27];
		// five_tuple[2] = data[28];	five_tuple[3] = data[29];
		// five_tuple[4] = (SrcPort >> 8) & 0x00FF;
		// five_tuple[5] = SrcPort & 0x00FF;
		// five_tuple[6] = data[30];	five_tuple[7] = data[31];
		// five_tuple[8] = data[32];	five_tuple[9] = data[33];
		// five_tuple[10] = (DstPort >> 8) & 0x00FF;
		// five_tuple[11] = DstPort & 0x00FF;
		// five_tuple[12] = data[23];
		
		// printf("(%u.%u.%u.%u:%u, %u.%u.%u.%u:%u, protocol=%u)\n",
		//  	five_tuple[0], five_tuple[1], five_tuple[2], five_tuple[3],
		//  	*(uint16_t*)(five_tuple + 4),
		//  	five_tuple[6], five_tuple[7], five_tuple[8], five_tuple[9],
		//  	*(uint16_t*)(five_tuple + 10),
		//  	five_tuple[12]);
		fwrite(five_tuple, 1, 13, fout);

		IPv4_cnt++;
	}
	
	IPv4_cnt_total += IPv4_cnt;
	
	fclose(fout);
	fclose(fin);
	return IPv4_cnt_total;
}

#endif
