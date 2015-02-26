#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <unistd.h>
#include <fcntl.h>
#include "liblouis.h"
#include "louis.h"

#define BUF_MAX  27720

static void trimLine(char *line)
{
	char *crs = line;
	while(*crs)
	if(*crs == '\n' || *crs == '\r')
	{
		*crs = 0;
		crs--;
		while(crs > line && (*crs == ' ' || *crs == '\t'))
		{
			*crs = 0;
			crs--;
		}
		return;
	}
	else
		crs++;
}

static void addSlashes(char *line)
{
	char *sft, *crs = line;
	while(*crs)
	{
		if(*crs == '\\')
		{
			sft = crs;
			while(*sft)
				sft++;
			sft[1] = 0;
			sft--;
			while(sft > crs)
			{
				sft[1] = sft[0];
				sft--;
			}
			sft[1] = '\\';
			crs++;
		}
		crs++;
	}	
}

int main(int argn, char **args)
{
	widechar inputText[BUF_MAX], output1Text[BUF_MAX], output2Text[BUF_MAX],
	         expectText[BUF_MAX], expectDots[BUF_MAX],
	         empText[BUF_MAX], etnText[BUF_MAX], tmpText[BUF_MAX];
	int inputLen, output1Len, output2Len, expectLen, empLen, tmpLen, etnLen = 0, etnHave;
	formtype emphasis[BUF_MAX], emp1[BUF_MAX], emp2[BUF_MAX];
	char *charText, inputLine[BUF_MAX], origInput[BUF_MAX], origEmp[BUF_MAX], origEtn[BUF_MAX];
	FILE *input, *passFile;
	int failFile, outFile;
	int pass_cnt = 0, fail_cnt = 0;
	int blank_out = 0, blank_pass = 1, blank_fail = 0;
	int out_more = 0, line = 0, i;
	
	unsigned short uni = 0xfeff, space = 0x0020, dash = 0x002d,
	               bar = 0x007c, plus = 0x002b, tab = 0x0009;
	unsigned int nl = 0x000a000d;
	
	input = stdin;
	for(i = 1; args[i]; i++)
	if(args[i][0] == '-' && args[i][1] == 'f')
	{
		i++;
		input = fopen(args[i], "r");
		if(!input)
		{
			fprintf(stderr, "ERROR:  cannot open input file %s\n", input);
			return 1;
		}
	}
	else if(args[i][0] == '-' && args[i][1] == 'm')
		out_more = 1;
	
	passFile = fopen("pass.txt", "w");
	outFile = open("output.txt", O_TRUNC | O_CREAT | O_RDWR);
	write(outFile, &uni, 2);
	//write(outFile, &nl, 4);
	failFile = open("fail.txt", O_TRUNC | O_CREAT | O_RDWR);
	write(failFile, &uni, 2);
	//write(failFile, &nl, 4);
	
	memset(emphasis, 0, BUF_MAX);
	
	while(fgets(inputLine, BUF_MAX - 97, input))
	{
		line++;
		trimLine(inputLine);
		if(!inputLine[0])
		{
//			if(blank_pass)
//			{
//				fprintf(passFile, "\n");
//				blank_pass = 0;
//			}
			if(!blank_fail)
			{
				write(failFile, &nl, 4);
				blank_fail = 1;
			}
			if(!blank_out)
			{
				write(outFile, &nl, 4);
				blank_out = 1;
			}
			continue;
		}
		
		if(inputLine[0] == '#')
		{
			if(inputLine[1] == '#')
				continue;
			addSlashes(inputLine);	
			inputLen = extParseChars(inputLine, inputText);
			if(inputLine[1] != '~')
			{
				if(!blank_pass)
				{
					blank_pass = 1;
					fprintf(passFile, "\n", inputLine);
				}
				fprintf(passFile, "%s\n", inputLine);	
				write(outFile, inputText, inputLen * 2);
				write(outFile, &nl, 4);
				blank_out = 0;
			}
			write(failFile, inputText, inputLen * 2);
			write(failFile, &nl, 4);
			blank_fail = 0;
			continue;
		}
			
		if(!strncmp("~end", inputLine, 4))
			break;

		if(!strncmp("~emp", inputLine, 4))
		if(fgets(inputLine, BUF_MAX - 97, input))
		{
			line++;
			trimLine(inputLine);
			strcpy(origEmp, inputLine);
			memset(emphasis, 0, BUF_MAX);
			for(i = 0; inputLine[i]; i++)
				emphasis[i] = inputLine[i] - '0';
			emphasis[i] = 0;
			continue;
		}
		else
		{
			fprintf(stderr, "ERROR:  unexpected on of file, #%d\n", line);
			return 1;
		}
		

		if(!strncmp("~etn", inputLine, 4))
		if(fgets(inputLine, BUF_MAX - 97, input))
		{
			line++;
			trimLine(inputLine);
			strcpy(origEtn, inputLine);
			//memset(emphasis, 0, BUF_MAX);
			etnLen = etnHave = 0;
			for(i = 0; inputLine[i]; i++)
			{
				etnLen++;
				if(inputLine[i] - '0')
				{
					etnHave = 1;
					emphasis[i] |= (inputLine[i] - '0') << 8;
				}
			}
			//emphasis[i] = 0;
			if(etnHave)
				empLen = extParseChars(origEtn, etnText);
			else
				etnLen = 0;
			continue;
		}
		else
		{
			fprintf(stderr, "ERROR:  unexpected on of file, #%d\n", line);
			return 1;
		}
		
		memcpy(emp1, emphasis, BUF_MAX);
		memcpy(emp2, emphasis, BUF_MAX);
		
		for(i = 0; i < BUF_MAX; i++)
		if(emphasis[i])
			break;
		if(i < BUF_MAX)
			empLen = extParseChars(origEmp, empText);
		else
			empLen = 0;
			
		strcpy(origInput, inputLine);
		addSlashes(inputLine);	
		memset(inputText, 0, BUF_MAX);	
		inputLen = extParseChars(inputLine, inputText);
		
		expectLen = 0;
		if(fgets(inputLine, BUF_MAX - 97, input))
		{
			line++;
			trimLine(inputLine);
			if(!strncmp("~emp", inputLine, 4))
			{
				fprintf(stderr, "ERROR:  emphasis where expected line should be at #%d\n", line);
				return 1;
			}
			
		//	if(inputLine[0] == '#')
		//	{
		//		fprintf(stderr, "ERROR:  comment where expected line should be at #%d\n", line);
		//		return 1;
		//	}
			
			if(inputLine[0])
			{
				addSlashes(inputLine);	
				expectLen = extParseChars(inputLine, expectText);
			}
		}
		
		output1Len = BUF_MAX;
		lou_translate(
			"en-ueb-g2.ctb",
			inputText,
			&inputLen,
			output1Text,
			&output1Len,
			emp1,
			NULL,
			NULL,
			NULL,
			NULL,
			0);
			
		if(out_more)
		{
			output2Len = BUF_MAX;
			lou_translate(
				"en-ueb-g2.ctb",
				inputText,
				&inputLen,
				output2Text,
				&output2Len,
				emp2,
				NULL,
				NULL,
				NULL,
				NULL,
				dotsIO | ucBrl);
		}
			
		if(!expectLen)
		{
			write(outFile, inputText, inputLen * 2);
			write(outFile, &nl, 4);
			if(empLen)
			{
				write(outFile, empText, empLen * 2);
				write(outFile, &nl, 4);
				if(etnLen)
				{
					write(outFile, etnText, etnLen * 2);
					write(outFile, &nl, 4);
				}
			}
			
			write(outFile, output1Text, output1Len * 2);
			write(outFile, &nl, 4);
			
			if(out_more)
			{
				write(outFile, output2Text, output2Len * 2);
				write(outFile, &nl, 4);
			}
			
			blank_out = 0;
		}
		else if(   expectLen != output1Len
				|| memcmp(output1Text, expectText, expectLen * 2))
		{
			fail_cnt++;
			tmpLen = extParseChars("in:  ", tmpText);
			write(failFile, tmpText, tmpLen * 2);
			write(failFile, inputText, inputLen * 2);
			write(failFile, &nl, 4);
			if(empLen)
			{
				tmpLen = extParseChars("emp: ", tmpText);
				write(failFile, tmpText, tmpLen * 2);
				write(failFile, empText, empLen * 2);
				write(failFile, &nl, 4);
				if(etnLen)
				{
					tmpLen = extParseChars("etn: ", tmpText);
					write(failFile, tmpText, tmpLen * 2);
					write(failFile, etnText, etnLen * 2);
					write(failFile, &nl, 4);
				}
			}
			
			tmpLen = extParseChars("ueb: ", tmpText);
			write(failFile, tmpText, tmpLen * 2);
			write(failFile, expectText, expectLen * 2);
			write(failFile, &nl, 4);
			
			if(out_more)
			{
				tmpLen = extParseChars("     ", tmpText);
				write(failFile, tmpText, tmpLen * 2);
				if(lou_charToDots("en-ueb-g2.ctb", expectText, tmpText, expectLen, ucBrl))
					write(failFile, tmpText, expectLen * 2);
				else
				{
					tmpLen = extParseChars("FAIL", tmpText);
					write(failFile, tmpText, tmpLen * 2);
				}
				write(failFile, &nl, 4);
			}
			
			tmpLen = extParseChars("lou: ", tmpText);
			write(failFile, tmpText, tmpLen * 2);
			write(failFile, output1Text, output1Len * 2);
			write(failFile, &nl, 4);
			
			if(out_more)
			{
				tmpLen = extParseChars("     ", tmpText);
				write(failFile, tmpText, tmpLen * 2);
				write(failFile, output2Text, output2Len * 2);
				write(failFile, &nl, 4);
			}
			
			blank_fail = 0;
		}
		else
		{
			pass_cnt++;
			printf("%s\n", origInput);
			fprintf(passFile, "%s\n", origInput);
			blank_pass = 0;
			if(out_more)
			{
				write(outFile, inputText, inputLen * 2);
				write(outFile, &nl, 4);
				if(empLen)
				{
					write(outFile, empText, empLen * 2);
					write(outFile, &nl, 4);
					if(etnLen)
					{
						write(outFile, etnText, etnLen * 2);
						write(outFile, &nl, 4);
					}
				}			
				write(outFile, output1Text, output1Len * 2);
				write(outFile, &nl, 4);
				write(outFile, output2Text, output2Len * 2);
				write(outFile, &nl, 4);
				blank_out = 0;
			}
		}
	}
	
	float percent = (float)pass_cnt / (float)(pass_cnt + fail_cnt);
	printf("%f%%\t%d\t%d\n", percent, pass_cnt, fail_cnt);
	
	fclose(passFile);
	close(outFile);
	close(failFile);

	return 0;
}
