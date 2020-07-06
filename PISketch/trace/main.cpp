#include "parser.h"
#include "blender.h"


using namespace std;

int main(int argc, char *argv[])
{
    //./a.out time_step the_number_of_packets_per_window
	if (argc != 4)
	{
		printf("./a.out  time_step(us)  max_pkt_num  max_win_num");
		return 0;
	}
    uint32_t time_step = (uint32_t)(atoi(argv[1]));
    uint32_t max_pkt_num = atoi(argv[2]);
    uint32_t max_win_num = atoi(argv[3]);

    char nor_traffic_dir[100] = "./raw/normal/";
    char att_traffic_dir[100] = "./raw/attack/";
    char test_traffic_dir[100] = "./test/";

    bool isCreated = false;

    //Create directories
    std::vector<std::string> attTrafficFiles = getFilesNames(att_traffic_dir);
    std::vector<std::string> norTrafficFiles = getFilesNames(nor_traffic_dir);
    // for (int i = 0; i < attTrafficFiles.size(); ++i)
    for (int i = 0; i < 1; ++i)
    {
    	// ./raw/attack/*.pcap
    	string str = norTrafficFiles[i].c_str();
    	str = str.substr(13, str.length() - 18);
    	sprintf(test_traffic_dir, "./test/%s", str.c_str());
		isCreated = mkdir(test_traffic_dir, S_IRWXU| S_IRWXG| S_IRWXO);
    }

    //Extract 5-tuples and generate test traffic 
    int fileNoBegin = 0;
    //for (int i = 0; i < attTrafficFiles.size(); ++i)
    for (int i = 0; i < 1; ++i)
    {
    	string str = norTrafficFiles[i].c_str();
    	str = str.substr(13, str.length() - 18);
    	sprintf(test_traffic_dir, "./test/%s", str.c_str());
    	fileNoBegin = 0;
    	printf("\n");
    	printf("Start Generating...\n");
    	fileNoBegin = Extract5Tuples(norTrafficFiles[i].c_str(), fileNoBegin, time_step, max_pkt_num, max_win_num, test_traffic_dir);
    	printf("%s: %d files.\n", norTrafficFiles[i].c_str(), fileNoBegin);
    	//GenTestData2(nor_traffic_dir, test_traffic_dir, padding);
    	printf("Complete.(%s)\n", str.c_str());

    }

	return 0;

    // ./a.out nameofAtttacker time_step(1/2/3/...) distribution(0:zipf/1:uniform) paddingNormalPacketsPerTimestep(10000/100000/...)
	// ./a.out numebr of attackers(1/2/3/...) time_step(1/2/3/...) distribution(0:zipf/1:uniform) paddingNormalPacketsPerTimestep(10000/100000/...)

    //Get path of attack traffic and normal traffic
	// uint8_t attackNo = atoi(argv[1]);
	// if (attackNo <= 0 || attackNo > 35)
	// {
	// 	printf("Number of attackers should be 1 ~ 35\n");
	// 	exit(1);
	// }
	// char normal_traffic_dir[100] = "./raw/normal/";
	// char attack_traffic_dir[100] = "./raw/attack";
	// char test_traffic_dir[100] = {0};
	// int isCreate= 0;
	// int fileNoBegin = 0;

	//Get all attack traffic files
	//std::vector<std::string> attTrafficFiles = getFilesNames(attack_traffic_dir);

	//Set timestep
	// uint32_t time_step = (uint32_t)(atoi(argv[2]));
	//Extract 5 tuples for all attack traffic files
	// for (int i = 0; i < attackNo; ++i)
	// {
	// 	sprintf(test_traffic_dir, "./testTraffic/%d", i);
	// 	isCreate = mkdir(test_traffic_dir, S_IRWXU| S_IRWXG| S_IRWXO);
	// 	if( !isCreate )
 //   			printf("Create path: %s\n", test_traffic_dir);
 //   		else
 //   			printf("Create path failed!: %s \n", test_traffic_dir);
 //   		fileNoBegin = 0;
	// 	//attTrafficFiles[i] = "./raw/attack/TrojanPage.pcap";
	// 	fileNoBegin =  Extract5Tuples(attTrafficFiles[i].c_str(), fileNoBegin, time_step, test_traffic_dir);
		
	// 	printf("%s\n",attTrafficFiles[i].c_str() );
	// }

	// /*
	// char attack_traffic_prefix[100] = "./raw/attack/";
	// char attack_traffic_suffix[100] = ".pcap";
	// int tmp;
	// char *attack_traffic_dir = new char[tmp = strlen(attack_traffic_prefix) + strlen(argv[1]) + strlen(attack_traffic_suffix)];
	// strcat(strcpy(attack_traffic_dir, attack_traffic_prefix), argv[1]);
	// strcat(attack_traffic_dir, attack_traffic_suffix);
	// */

	// //Configure distribution
	// int distribution = atoi(argv[3]);
	// int rand_seed;
	// double alpha;
	// rand_seed = 0;
	// alpha = 0;


	// //Set the number of packets that are filled in each timestep
	// int padding = atoi(argv[4]);

	// /*
	// //Set and create path for test traffic
	// char test_traffic_dir[100] = {0};
	// sprintf(test_traffic_dir, "./testTraffic/%s", argv[1]);
	// int isCreate = mkdir(test_traffic_dir, S_IRWXU| S_IRWXG| S_IRWXO);
	// if( !isCreate )
 //   		printf("Create path: %s\n", test_traffic_dir);
 //   	else
 //   		printf("Create path failed!: %s \n", test_traffic_dir);

 //   	//Split attack traffic into files with time_step
	// int fileNoBegin = 0;
	// fileNoBegin =  Extract5Tuples(attack_traffic_dir, fileNoBegin, time_step, test_traffic_dir);
	// */

	// //Generate test traffic
	// sprintf(test_traffic_dir, "./testTraffic/");
	// sprintf(attack_traffic_dir, "./testTraffic/");
	// GenTestData(normal_traffic_dir, attack_traffic_dir, test_traffic_dir, distribution, attackNo, padding, rand_seed, alpha);

	//return 0;
}
