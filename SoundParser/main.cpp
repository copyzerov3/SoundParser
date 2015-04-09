#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include <Windows.h>
using namespace std;


struct time
{
	float startTime;
	float endTime;
};

vector<time> times;
vector<string> files;
char * memblock = nullptr;
streampos size;
unsigned int bitrate = 0;
float timeOfFile = 0.0f;

bool Load(string filename)
{
	ifstream file(filename, ios::in | ios::binary | ios::ate);
	if (file.is_open())
	{
		size = file.tellg();
		memblock = new char[size];
		file.seekg(0, ios::beg);
		file.read(memblock, size);
		
		file.close();

		cout << "The entire file content is in memory" << endl;
		cout <<"The amount of lines read are : "<<size << endl;
	}
	else
	{
		cout << "Could not open file" << endl;
		return false;
	}
	return true;
}

void Parse()
{
	bool foundBeginning = false;
	bool foundfmt = 0;
	unsigned int startOfData = 0;
	unsigned int minimumData = bitrate / 4;
	unsigned int startOfNothing = 0;
	unsigned int nothingCounter = 0;
	int temp = 0;
	for (unsigned int i = 0; i < size; i++)
	{
		if (!foundBeginning)
		{
			if (memblock[i] == 'd')
			{
				if (memblock[i + 1] == 'a' && memblock[i + 2] == 't' && memblock[i + 3] == 'a')
				{
					foundBeginning = true;
					//cout << "Found the beginning" << endl;

					i += 3;
					startOfData = i;
				}
			}
			else if (memblock[i] == 'f')
			{
				if (memblock[i + 1] == 'm' && memblock[i + 2] == 't' && memblock[i + 3] == ' ')
				{
					//cout << "Found fmt at i : " << i << endl;
					i += 14;
					int blockOne = int(memblock[i + 1]);
					int blockTwo = int(memblock[i + 2]);
					int blockThree = int(memblock[i + 3]);
					int blockFour = int(memblock[i + 4]);
					if (blockOne < 0)
					{
						blockOne *= -1;
					}
					if (blockTwo < 0)
					{
						blockTwo *= -1;
					}
					if (blockThree < 0)
					{
						blockThree *= -1;
					}
					if (blockFour < 0)
					{
						blockFour *= -1;
					}

					UINT32 bitrateLittle = 0;
					bitrateLittle = (bitrateLittle << 8) + byte(blockFour);
					bitrateLittle = (bitrateLittle << 8) + byte(blockThree);
					if (blockTwo == 0)
					{
						bitrateLittle = (bitrateLittle << 4) + byte(blockTwo);
					}
					else
						bitrateLittle = (bitrateLittle << 8) + byte(blockTwo);
					if (blockOne == 0)
					{
						bitrateLittle = (bitrateLittle << 4) + byte(blockOne);
					}
					else
					{
						bitrateLittle = (bitrateLittle << 8) + byte(blockOne);
					}
					if (blockOne == 0 && blockTwo == 0)
					{
						bitrateLittle = (bitrateLittle << 4) + byte(blockOne);
					}

					bitrate = (bitrateLittle / 2) / 1000;
					minimumData = bitrate / 4;
				}
			}
		}
		else
		{
			if (memblock[i] == ' ' || memblock[i] == '\0')
			{
				if (nothingCounter == 0)
				{
					startOfNothing = i;

				}
				nothingCounter++;
			}
			else if (memblock[i] == 'r')
			{

				if (memblock[i + 1] == 'e' && memblock[i + 2] == 'g' && memblock[i + 3] == 'n' && (memblock[i + 4] == '/' || memblock[i + 4] == '\\'))
				{
					//cout << "Found the end" << endl;
					timeOfFile = (float)((float)((float)((float)(float)(i - startOfData) * 8) / bitrate));
					return;
				}
			}
			else
			{

				if (nothingCounter >48)
				{
					temp = int(memblock[i]);
					if (temp < 0)
					{
						temp *= -1;
					}
					if (temp > 20)
					{
						if (memblock[i + 1] == 0 || memblock[i + 2] == 0 || memblock[i + 3] || memblock[i + 4] == 0)
						{
							nothingCounter++;
						}
						else if (nothingCounter >= minimumData)
						{
							time newTime;
							newTime.startTime = (float)((float)((float)((float)(float)(startOfNothing - startOfData) * 8) / bitrate));
							newTime.endTime = (float)((float)((float)((float)(float)(startOfNothing - startOfData + nothingCounter) * 8) / bitrate));
							if (newTime.endTime>250)
								times.push_back(newTime);
							//cout << "\nNew Time Added" << endl;
							cout << "The start in the file of nothing : " << startOfNothing << endl;
							cout << "How many nothings were after it : " << nothingCounter << endl;
							cout << "Start of new Time in milliseconds : " << newTime.startTime << endl;
							cout << "New Time End Time in milliseconds : " << newTime.endTime << endl;
						//	system("PAUSE");
							nothingCounter = 0;
							startOfNothing = 0;
						}
					}
				}
				else
				{
					nothingCounter = 0;
					startOfNothing = 0;
				}
			}
		}
	}
}
void output(string filename)
{
	ofstream myfile;
	string file = filename.substr(0, filename.size() - 4);
	myfile.open("output/"+file + "Timings.xml");
	myfile << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
	myfile << "<CONTENT>\n";
	myfile << "<LABEL TITLE = \"numOfTimes\">"<<times.size()<<"</LABEL>\n";
	myfile << "<LABEL TITLE = \"fileTime\">" << timeOfFile << "</LABEL>\n";
	myfile << "<LABEL TITLE = \"sequential\">1</LABEL>\n";
	for (unsigned int i = 0; i < times.size(); i++)
	{
		myfile << "<LABEL TITLE = \"start" << (i+1) << "\">" << times[i].startTime << "</LABEL>"<<endl;
		myfile << "<LABEL TITLE = \"end" << (i +1)<< "\">" << times[i].endTime << "</LABEL>" << endl;
	}
	
	myfile << "</CONTENT>\n";
	myfile.close();
	cout << "Made file" << endl << endl;;
	times.clear();
	delete[] memblock;
}
void Find()
{
	WIN32_FIND_DATA search_data;

	memset(&search_data, 0, sizeof(WIN32_FIND_DATA));

	HANDLE handle = FindFirstFile("*.wav", &search_data);

	while (handle != INVALID_HANDLE_VALUE)
	{
		printf("Found file: %s\r\n", search_data.cFileName);
		files.push_back(search_data.cFileName);
		if (FindNextFile(handle, &search_data) == FALSE)
			break;
	}

}
int main()
{
	Find();
	unsigned int processed = 0;
	while (processed != files.size())
	{
		cout << "Current File = " << files[processed] << endl;
		if (!Load(files[processed]))
		{
			processed++;
			continue;
		}
			
		Parse();
		output(files[processed]);
		
		processed++;
	}
	cout << "Processed all files" << endl;
	system("PAUSE");
	return 0;
}