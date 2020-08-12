#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

struct timeSpot
{
		unsigned int minute;
		unsigned int second;
};

typedef unsigned long int timestamp;

struct classTime
{
		struct timeSpot tstart;
		struct timeSpot tend;
};

struct config
{
		char* title;
		struct timeSpot zero;
		char* mainFile;
		char* extFile;
		char* extName;
}CONFIG;

struct class
{
		bool valid;
		bool extValid;
		char* extName;
		struct timeSpot tstart;
		struct timeSpot tend;
}classes[100];

struct timeSpot str2ts(char str[1024])
{
//		printf("STRING=%s\n", str);
		unsigned int length = strlen(str);
		char minute[20];
		char second[20];
		unsigned int i = 0;
		for(; i<length; i++)
		{
				if(str[i] != ':')
				{
						minute[i] = str[i];
				}
				else
				{
						break;
				}
		}
		i++;
		for(unsigned int j=0; i<length; i++)
		{
//				puts(" In second FOR\n");
				second[j] = str[i];
//				printf("Now S [%c] [%c]\n", second[0], second[1]);
				j++;
		}
		struct timeSpot temp;
//		printf("min %d sec %d\n", atol(minute), atol(second));
		temp.minute = atol(minute);
		temp.second = atol(second);

		return temp;
}

int parseFile(xmlDocPtr doc, xmlNodePtr cur)
{
		xmlChar* main;
		xmlChar* extName;
		xmlChar* ext;
		cur = cur -> xmlChildrenNode;
		while(cur != NULL)
		{
				if((!xmlStrcmp(cur->name, (const xmlChar*) "main")))
				{
						main = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
						CONFIG.mainFile = main;
						printf("MAIN=%s\n", main);
				}
				if((!xmlStrcmp(cur->name, (const xmlChar*) "ext")))
				{
						ext = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
						extName = xmlGetProp(cur, "name");
						CONFIG.extFile = ext;
						CONFIG.extName = extName;
						printf("EXT[%s]=%s\n", extName, ext);
				}
				cur = cur -> next;
		}
		return 0;
}

int parseConfig(xmlDocPtr doc, xmlNodePtr cur)
{
		xmlChar* title;
//		xmlChar* zero;
		cur = cur -> xmlChildrenNode;
		while(cur != NULL)
		{
				if((!xmlStrcmp(cur->name, (const xmlChar*) "title")))
				{
						title = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
						CONFIG.title = title;
						printf("TITLE=%s\n", title);
				}
/*				if((!xmlStrcmp(cur->name, (const xmlChar*) "zero")))
				{
						zero = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
						CONFIG.zero = str2ts(zero);
						printf("ZERO=%s\n", zero);
				}
*/
				if((!xmlStrcmp(cur->name, (const xmlChar*) "file")))
				{
						parseFile(doc, cur);
				}
				cur = cur -> next;
		}
		return 0;
}

struct classTime parseTime(xmlDocPtr doc, xmlNodePtr cur)
{
		xmlChar* tstart;
		xmlChar* tend;
		cur = cur -> xmlChildrenNode;
		while(cur != NULL)
		{
				if((!xmlStrcmp(cur->name, (const xmlChar*) "tstart")))
				{
						tstart = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
						printf("TSTART=%s\n", tstart);
				}
				if((!xmlStrcmp(cur->name, (const xmlChar*) "tend")))
				{
						tend = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
						printf("TEND=%s\n", tend);
				}
				cur = cur -> next;
		}
		struct classTime temp;
		temp.tstart = str2ts(tstart);
		temp.tend = str2ts(tend);
		return temp;
}

int parseClass(xmlDocPtr doc, xmlNodePtr cur)
{
		static unsigned int a = 0;

		xmlChar* extplay;
		cur = cur -> xmlChildrenNode;
		while(cur != NULL)
		{
				if((!xmlStrcmp(cur->name, (const xmlChar*) "time")))
				{
						struct classTime tste = parseTime(doc, cur);
						classes[a].valid = true;
						classes[a].tstart = tste.tstart;
						classes[a].tend = tste.tend;
				}
				if((!xmlStrcmp(cur->name, (const xmlChar*) "extplay")))
				{
						extplay = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
						classes[a].valid = true;
						classes[a].extValid = true;
						classes[a].extName = extplay;
						printf("EXTPLAT=%s\n", extplay);
				}
				cur = cur -> next;
		}

		a++;
		return 0;
}

int main(int argc, char** argv)
{
		char* filename;
		if(argc <= 1)
		{
				printf("Usage: autoplax [XML Schedule]\n");
				return -1;
		}
		filename = argv[1];
		xmlDocPtr doc;
		xmlNodePtr cur;
		doc = xmlParseFile(filename);
		if(doc == NULL)
		{
				printf("Error: Document not parsed successfully.\n");
				return -1;
		}
		cur = xmlDocGetRootElement(doc);
		if(cur == NULL)
		{
				printf("Error: No root element.\n");
				xmlFreeDoc(doc);
				return -1;
		}
		if(xmlStrcmp(cur->name, (const xmlChar*) "schedule"))
		{
				printf("Error: Not an AutoPlaX file, wrong root element.\n");
				return -1;
		}
		cur = cur -> xmlChildrenNode;
		while(cur != NULL)
		{
				if((!xmlStrcmp(cur->name, (const xmlChar *) "config")))
				{
						parseConfig(doc, cur);
				}
				if((!xmlStrcmp(cur->name, (const xmlChar*) "class")))
				{
						parseClass(doc, cur);
				}
				cur = cur -> next;
		}

		// Data structure stored OK, now start play stream!
		printf("Using title=%s\n", CONFIG.title);
		// If class[0].tstart = 7:20, now is 7:00, then we wait until it is 7:20

		unsigned int classCnt = 0;

		// Get zero timespot
		//CONFIG.zero.minute = hour(24)
		//           .second = minute(60)

		char tmp[64];
		time_t t = time(0);
		strftime(tmp, sizeof(tmp), "%H:%M", localtime(&t));
		CONFIG.zero.minute = str2ts(tmp).minute;
		CONFIG.zero.second = str2ts(tmp).second;
		while(true)
		{
				if(classes[classCnt].valid != true)
				{
						break;
				}
				// Let's wait until tstart!
				unsigned long int time0 = time(0);

				int delaySec;
				printf("ClassCNT=%d, ZERO[%d %d]\n", classCnt, CONFIG.zero.minute, CONFIG.zero.second);
/*				if(classCnt == 0)
				{
						printf("%ld-%ld\n",classes[classCnt].tstart.minute,CONFIG.zero.minute);
						int a1 = classes[classCnt].tstart.minute-CONFIG.zero.minute;
						printf("a1=%d\n", a1);
						delaySec = ((classes[classCnt].tstart.minute-CONFIG.zero.minute)*60+(classes[classCnt].tstart.second-CONFIG.zero.second))*60;
				}
				else
				{
						delaySec = ((classes[classCnt].tstart.minute-classes[classCnt-1].tend.minute)*60+(classes[classCnt].tstart.second-classes[classCnt-1].tstart.second))*60;
				}

				printf("Delay Sec=%ld\n", delaySec);
			//	printf("time0=%ld time=%ld\n", time0, time(0));
				while(time0+delaySec >= time(0)){
				}*/

				while(true)
				{
						time_t now = time(0);
						char tmp[64];
						strftime(tmp, sizeof(tmp), "%H:%M", localtime(&now));
						int h = str2ts(tmp).minute;
						int m = str2ts(tmp).second;
						if((h >= classes[classCnt].tstart.minute)&&(m >= classes[classCnt].tstart.second))
								break;
				}
				puts("out of while");
				char cmd[1024];
				sprintf(cmd, "ffplay -autoexit -nodisp %s", CONFIG.mainFile);
				printf("class[%d] TSTART\n", classCnt);
				//
				//return 0;
				//
				system(cmd);

				// Let's wait until tend!
/*				time0 = time(0);
				delaySec = ((classes[classCnt].tend.minute-classes[classCnt].tstart.minute)*60+(classes[classCnt].tend.second-classes[classCnt].tstart.second))*60;

				printf("Delay Sec=%ld\n", delaySec);

				while(time0+delaySec >= time(0)){
				}*/

				while(true)
				{
						time_t now = time(0);
						char tmp[64];
						strftime(tmp, sizeof(tmp), "%H:%M", localtime(&now));
						int h = str2ts(tmp).minute;
						int m = str2ts(tmp).second;
						if((h >= classes[classCnt].tend.minute)&&(m >= classes[classCnt].tend.second))
								break;
				}

				printf("class[%d] TEND\n", classCnt);
				system(cmd);	// Ring the MAIN file

				// Check if EXT is needed.
				if(classes[classCnt].extValid == true)
				{
						// Need extra one
						if((xmlStrcmp(CONFIG.extName, classes[classCnt].extName)))
						{
								printf("Error: Bad extname defined in CLASS element.\n");
								printf("Aborts\n");
								return -1;
						}
						char extCmd[1024];
						sprintf(extCmd, "ffplay -autoexit -nodisp %s", CONFIG.extFile);
						system(extCmd);
				}

				// OK.......
				classCnt++;
		}

		printf("It seems everything done...\n");
		return 0;
}
