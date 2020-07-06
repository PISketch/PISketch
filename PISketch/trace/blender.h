#ifndef _MIX_PACKETS_
#define _MIX_PACKETS_
#endif

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fstream>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <dirent.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <functional>
#include <time.h>

#include "genzipf.h"

#define CONSTANT_NUMBER 2654435761u

bool cmp(std::string const &arg_a, std::string const &arg_b) {
    return arg_a.size() < arg_b.size() || (arg_a.size() == arg_b.size() && arg_a < arg_b);
}

std::vector<std::string> getFilesNames(char const* dataDir)
{
	std::vector<std::string> files;
	DIR *dir;
	FILE *fp;
    struct dirent *ptr;
    dir = opendir(dataDir); ///open the dir
    int cnt = 0;
    int t_cnt = 0;
    char dirname[100] = {0};

    while((ptr = readdir(dir)) != NULL) ///read the list of this dir
    {   
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) 
        {
        	continue;
        }
        t_cnt++;

        memset(dirname,0,100);
        sprintf(dirname, "%s%s", dataDir, ptr->d_name);
       

        //fp =  fopen(dirname,"rb");
        //if( fgetc(fp) != -1)
        //{
       	files.push_back(dirname);
        
        //}
    	//else
    	//{
    	//	cnt++;
    	//}
    	//fclose(fp);
    }
    closedir(dir);
    //printf("%s\t",dataDir );
    //printf("Empty FILE(S):%d in %d\n", cnt, t_cnt);
    std::sort(files.begin(), files.end(), cmp);
    return files;
}

std::vector<std::string> getDirsNames(char const* dataDir)
{
	std::vector<std::string> directories;
	DIR *dir;
	FILE *fp;
    struct dirent *ptr;
    dir = opendir(dataDir); ///open the dir
    char dirname[100] = {0};


    while((ptr = readdir(dir)) != NULL) ///read the list of this dir
    {   
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) 
        {
        	continue;
        }
        if (ptr->d_type == 4)
        {
        	memset(dirname,0,100);
        	sprintf(dirname, "%s%s", dataDir, ptr->d_name);
        	printf("FILENAME:%s\n", ptr->d_name);
        	directories.push_back(dirname);
        }    
    }
    closedir(dir);
    std::sort(directories.begin(), directories.end(), cmp);
    return directories;
}

void moveFile(char const* src_file_path, char* dst_dir)
{
	FILE* fin = fopen(src_file_path, "rb");
	const char *deli = "/";
	char *file_name;
	char *tok;
	char file_path[100];
	sprintf(file_path,"%s",const_cast<char*>(src_file_path));
	tok = strtok(const_cast<char*>(src_file_path), deli);
	while(tok != NULL)
	{
		file_name = tok;
		tok = strtok(NULL, deli);
	}
	char dst_file_path[100];
	sprintf(dst_file_path,"%s/%s",dst_dir, file_name);
	FILE* fout = fopen(dst_file_path,"wb");
	uint8_t five_tuple[13];

	while(fread(five_tuple, sizeof(uint8_t), 13, fin))
	{
		fwrite(five_tuple, 1, 13, fout);
		memset(five_tuple, 0, 13);
	}

	fclose(fin);
	fclose(fout);

	remove(file_path);
	
	
}

void moveDirectory(char const* src_dir, char* dst_dir)
{
	std::vector<std::string> srcFiles = getFilesNames(src_dir);
	for (int i = 0; i < (int)srcFiles.size(); ++i)
	{
		moveFile(srcFiles[i].c_str(), dst_dir);
	}
}


void mergeTwoFiles(char const* src_file_name_main, char const* src_file_name_slave, char* dst_dir)
{
	FILE* src_file_main = fopen(src_file_name_main, "rb");
	FILE* src_file_slave = fopen(src_file_name_slave, "rb");
	const char *deli = "/";
	char *file_name;
	char *tok;
	char main_file_path[100];
	char slave_file_path[100];
	sprintf(main_file_path,"%s",const_cast<char*>(src_file_name_main));
	sprintf(slave_file_path,"%s",const_cast<char*>(src_file_name_slave));

	tok = strtok(const_cast<char*>(src_file_name_main), deli);
	while(tok != NULL)
	{
		file_name = tok;
		tok = strtok(NULL, deli);
	}
	char dst_file_path[100];
	sprintf(dst_file_path,"%s/%s", dst_dir, file_name);
	FILE* dst_file = fopen(dst_file_path, "wb");
	uint8_t five_tuple[13];
	srand((unsigned)time(NULL)); 
	int read = 0;

	while (!feof(src_file_main) && !feof(src_file_slave))
	{
		read = rand() % 2;
		if (read == 0)
		{
			fread(five_tuple, sizeof(uint8_t), 13, src_file_main);
		}
		else
		{
			fread(five_tuple, sizeof(uint8_t), 13, src_file_slave);
		}
		fwrite(five_tuple, 1, 13, dst_file);
		memset(five_tuple, 0 , 13);
	}

	while (!feof(src_file_main))
	{
		fread(five_tuple, sizeof(uint8_t), 13, src_file_main);
		fwrite(five_tuple, 1, 13, dst_file);
		memset(five_tuple, 0 , 13);
	}

	while (!feof(src_file_slave))
	{
		fread(five_tuple, sizeof(uint8_t), 13, src_file_slave);
		fwrite(five_tuple, 1, 13, dst_file);
		memset(five_tuple, 0 , 13);
	}

	fclose(src_file_main);
	fclose(src_file_slave);
	fclose(dst_file);
	
	remove(main_file_path);
	remove(slave_file_path);

}




void mergeTwoDirectories(char const* src_dir_name_a, char const* src_dir_name_b, char* dst_dir)
{

	std::vector<std::string> src_files_a = getFilesNames(src_dir_name_a);
	std::vector<std::string> src_files_b = getFilesNames(src_dir_name_b);
	int fileNo_a = (int)src_files_a.size();
	int fileNo_b = (int)src_files_b.size();
	
	int start_point = 0;
	int main = 0;
	int mergedFileNo = 0;
	int totalFileNo = 0;
	srand((unsigned)time(NULL)); 
	
	if (fileNo_a >= fileNo_b)
	{
		main = 0;
		mergedFileNo = fileNo_b;
		totalFileNo = fileNo_a;
		start_point = rand() % (fileNo_a - fileNo_b + 1);
	} 
	else
	{
		start_point = rand() % (fileNo_b - fileNo_a + 1);
		main = 1;
		mergedFileNo = fileNo_a;
		totalFileNo = fileNo_b;
	}

	char dst_file_path[100] = {0};

	for (int i = 0; i < totalFileNo; ++i)
	{
		if (i >= start_point && i <= (start_point + mergedFileNo - 1))
		{
			if (main == 0)
			{
				mergeTwoFiles(src_files_a[i].c_str(), src_files_b[i - start_point].c_str(), dst_dir);
			}
			else
			{
				mergeTwoFiles(src_files_b[i].c_str(), src_files_a[i - start_point].c_str(), dst_dir);
			}
		}
		else
		{
			if (main == 0)
			{
				moveFile(src_files_a[i].c_str(), dst_dir);
			}
			else
			{
				moveFile(src_files_b[i].c_str(), dst_dir);
			}
		}
	}

}

void packetsPadding(char* normal_traffic_dir, char* attack_traffic_dir, int padding)
{
	std::vector<std::string> nor_traffic = getFilesNames(normal_traffic_dir);
	std::vector<std::string> att_traffic = getFilesNames(attack_traffic_dir);
	int totalAttFileNo = (int)att_traffic.size();
	int totalNorFileNo = (int)nor_traffic.size();
	uint8_t five_tuple[13];
	int curNorFile = 0;
	char outputFileName[100];
	int curPadding = 0;

	FILE *norFin = fopen(nor_traffic[curNorFile].c_str(),"rb");
	srand((unsigned)time(NULL));
	int read = 0;


	for (int i = 0; i < totalAttFileNo; ++i)
	{
		memset(outputFileName, 0, 100);
		sprintf(outputFileName, "%s/%d.dat", attack_traffic_dir, i);

		FILE *attFin = fopen(att_traffic[i].c_str(),"rb");
		FILE *fout = fopen(outputFileName,"ab+");
		curPadding = 0;

		while (curPadding <= padding && !feof(attFin))
		{
			read = rand() % 2;
			if (read == 0)
			{
				fread(five_tuple, sizeof(uint8_t), 13, attFin);
				fwrite(five_tuple, 1, 13, fout);
				memset(five_tuple, 0 , 13);	
			}
			else
			{
				if (!feof(norFin))
				{
					fread(five_tuple, sizeof(uint8_t), 13, norFin);
					fwrite(five_tuple, 1, 13, fout);
					memset(five_tuple, 0 , 13);	
					curPadding++;
				}
				else
				{
					fclose(norFin);
					if (curNorFile >= totalNorFileNo - 1)
					{
						printf("More normal traffic is needed.(%d)\n", totalNorFileNo);
						return;
					}
					norFin = fopen(nor_traffic[++curNorFile].c_str(), "rb");

				}
			}
		}

		while (!feof(attFin))
		{
			fread(five_tuple, sizeof(uint8_t), 13, attFin);
			fwrite(five_tuple, 1, 13, fout);
			memset(five_tuple, 0 , 13);	
		}

		while (curPadding <= padding)
		{
			if (!feof(norFin))
			{
				fread(five_tuple, sizeof(uint8_t), 13, norFin);
				fwrite(five_tuple, 1, 13, fout);
				memset(five_tuple, 0 , 13);
				curPadding++;	
			}
			else
			{
				fclose(norFin);
				if (curNorFile >= totalNorFileNo - 1)
				{
					printf("More normal traffic is needed.(%d)\n", totalNorFileNo);
					return;
				}
				norFin = fopen(nor_traffic[++curNorFile].c_str(), "rb");
			}	
		}
		fclose(attFin);
		fclose(fout);
	}
	fclose(norFin);
}


void packetsPadding2(char* normal_traffic_dir, char* attack_traffic_dir, int padding)
{
	std::vector<std::string> nor_traffic = getFilesNames(normal_traffic_dir);
	std::vector<std::string> att_traffic = getFilesNames(attack_traffic_dir);
	int totalAttFileNo = (int)att_traffic.size();
	int totalNorFileNo = (int)nor_traffic.size();
	uint8_t five_tuple[13];
	int curNorFile = 0;
	char outputFileName[100];
	int curPadding = 0;

	//FILE *norFin = fopen(nor_traffic[curNorFile].c_str(),"rb");
	FILE *norFin = fopen("./raw/normal/0.pcap","rb");
	int read = 0;


	for (int i = 0; i < totalAttFileNo - 1; ++i)
	{
		memset(outputFileName, 0, 100);
		sprintf(outputFileName, "%s/%d.dat", attack_traffic_dir, i);
		FILE *fout = fopen(outputFileName,"ab+");
		curPadding = 0;

		while (curPadding <= padding)
		{
			if (!feof(norFin)){

				fread(five_tuple, sizeof(uint8_t), 13, norFin);
				fwrite(five_tuple, sizeof(uint8_t), 13, fout);
				memset(five_tuple, 0 , 13);	
				curPadding++;
			}
			else
			{
				fclose(norFin);
				if (curNorFile >= totalNorFileNo - 1)
				{
					printf("More normal traffic is needed.(%d)\n", totalNorFileNo);
					return;
				}
				norFin = fopen(nor_traffic[++curNorFile].c_str(), "rb");
			}
			

		}
		fclose(fout);

	}
	fclose(norFin);

}

void GenTestData2(char* nor_traffic_dir, char* test_traffic_dir, int padding){
	printf("Generating...\n");
	packetsPadding2(nor_traffic_dir, test_traffic_dir, padding);

}


void GenTestData(char* normal_traffic_dir, char* attack_traffic_dir, char* test_traffic_dir, int distribution, int attackNo, int padding, int rand_seed, double alpha)
{

	printf("Generating...\n");
	std::vector<std::string> dirs = getDirsNames(attack_traffic_dir);
	char a[100] = "./testTraffic/fog";
	char b[100] = "./testTraffic/haze";
	char c[100] = "./testTraffic/test";

	for (int i = 0; i < (attackNo); ++i)
	{
		if (i%2 == 0)
		{
			mergeTwoDirectories(dirs[i].c_str(), a, b);
		}
		else
		{
			mergeTwoDirectories(dirs[i].c_str(), b, a);
		}
	}
	
	
	if (attackNo % 2 == 0)
	{
		moveDirectory(a, c);
	}
	else
	{
		moveDirectory(b, c);
	}
	
	
	packetsPadding(normal_traffic_dir, c, padding);


	printf("Completed.\n");
	



	/*
	std::vector<int> srcDirs; // fileNO startpoint
	int maxFileNo = 0;
	int maxFileIndex = 0;
	int tmpFileNo = 0;
	for (int i = 0; i < attackNo; ++i)
	{
		sprintf(test_traffic_file, "%s%d",test_traffic_dir, i);
		tmpFileNo = (int)getFileNames(test_traffic_file).size();
		srcDirs.push_back(tmpFileNo);
		if (tmpFileNo > maxFileNo)
		{
			maxFileIndex = i;
		}
	}

	//Clculate start points
	srand((unsigned)time(NULL)); 
	std::vector<int> startPoints;
	int sp = 0;
	for (int j = 0; j < attackNo; ++j)
	{
		sp = (rand() % (maxFileNo - srcDirs[j]));
		startPoints.push_back(sp);
	}





	char* test_traffic_file[100] = {0};
	for (int j = 0; j < maxFileNo; ++j)
	{
		sprintf(test_traffic_file, "./testTraffic/%d.dat", j);
		FILE* fout = fopen(test_traffic_file,"rb+");


		fclose(fout)
	}
	
	//Get the number of files of attack and normal traffic and their names
	std::vector<std::string> attTrafficFiles = getFileNames(att_traffic_dir);
	std::vector<std::string> norTrafficFiles = getFileNames(normal_traffic_dir);
	int totalAttFileNo = (int)attTrafficFiles.size();
	int totalNorFileNo = (int)attTrafficFiles.size();

	//Set name and path of output file
	char outputFileName[100];
 
 	//Read the attack traffic and normal traffic and write in the test traffic directory 
 	uint8_t five_tuple[13];
 	uint8_t curNorFile = 0;
	FILE *norFin = fopen(norTrafficFiles[curNorFile].c_str(),"rb");

	for (int i = 0; i < totalAttFileNo; ++i)
	{
		memset(outputFileName, 0, 100);
		sprintf(outputFileName, "./testTraffic/%d.dat", i);
		FILE *attFin = fopen(attTrafficFiles[i].c_str(),"rb");
		FILE *fout = fopen(outputFileName,"ab+");

		while (fread(five_tuple, sizeof(uint8_t), 13, attFin))
		{
			fwrite(five_tuple, 1, 13, fout);
			memset(five_tuple, 0 , 13);	
		}

		for (int j = 0; j < padding; ++j)
		{
			if (fread(five_tuple, sizeof(uint8_t), 13, norFin))
			{
				fwrite(five_tuple, 1, 13, fout);
				memset(five_tuple, 0 , 13);	
			} 
			else
			{
				fclose(norFin);
				if (curNorFile >= totalNorFileNo)
				{
					printf("More normal traffic is needed.(%d)\n", totalNorFileNo);
					return;
				}
				FILE *norFin = fopen(norTrafficFiles[++curNorFile].c_str(), "rb");
				j--;
			}

		}

		fclose(attFin);
		fclose(fout);

	}

	fclose(norFin);
	printf("Completed.(%d Files)\n", totalAttFileNo);

	

	int pos = 0;
	int spos = 0;
	
	char outputFileName[100];
	
	std::vector<std::string> norDataFiles = getFileNames(norDataDir);
	std::vector<std::string> attDataFiles = getFileNames(attDataDir);
	int norData = (int)norDataFiles.size();
	int attData = (int)attDataFiles.size();
	printf("Valid normal files:%d\n", norData);
	printf("Valid attack files:%d\n", attData);


	// Sort and MIX 
	if (distribution == 2)
	{
		std::sort(attDataFiles.begin(), attDataFiles.end(), cmp);
		std::sort(norDataFiles.begin(), norDataFiles.end(), cmp);
		if (norData >= attData)
		{
			pos = norData - attData;
			srand((unsigned)time(NULL));
			pos = rand()%pos;
			pos--;
		} else 
		{
			printf("Need more normal data.\n");
		}
	}
	

	int zipf_pos[attData];
	std::hash<std::string> hash_fn;

	if (distribution == 0)
	{
		Genzipf(rand_seed, alpha, norData, attData, zipf_pos);
	}

	for (int i = 0; i < attData; i++)
	{
		memset(outputFileName, 0, 100);
		if (distribution == 1)
		{
			pos = ((uint8_t)hash_fn(attDataFiles[i])) % norData;
		}
		else if (distribution == 0)
		{
			pos = zipf_pos[i] - 1;
		}
		else if (distribution == 2)
		{
			pos++;
		}
		sprintf(outputFileName, "./norData/%d.dat", pos);
		FILE *fin = fopen(attDataFiles[i].c_str(),"rb");
		FILE *fout = fopen(outputFileName,"ab+");

		while (fread(five_tuple, sizeof(uint8_t), 13, fin) > 0)
		{
			fwrite(five_tuple, 1, 13, fout);
			memset(five_tuple, 0 , 13);	
		}
		fclose(fin);
		fclose(fout);
		
	}
	printf("Completed.\n");
	*/
}
