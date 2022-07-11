#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>

using namespace std;

bool encode();
bool decode();
char* ConvertToArr(fstream* file, int* text_size);
vector<int> ToBin(char* text, int text_size);
int CheckBit(const int byte, const int bit);
int DefControlBits(int block_size);
vector<int> InsertControlBits(vector<int>* Bin, int block_size, int control, int text_size, int* count);
void CopyToBlock(vector<int>* Bin, vector<int>* Block, int block_size, int* count);
void SetControlBits(vector<int>* Block, int block_size, int control);
void WriteToArray(int* arr, vector<int> Block, int size, int* count);
void WriteToFile(int* arr, fstream* file, int size);
void SetControlBitsZero(vector<int>* Bin, int size);
vector<int> CopyEncryptBlock(vector<int> Bin, int block_size, int control, int* count);
void FixBits(vector<int>* Block, vector<int>* Block_cpy, int size);
void WriteToResult(vector<int> Bin, int* arr, int size, int* count);
void WriteToFileResult(int* arr, fstream* file, int size);

int control_bits[] = { 0, 1, 3, 7, 15, 31, 63 };

int main()
{
	int selection;
	cout << "Encrypt - 1" << endl << "Decrypt - 2" << endl;
	cin >> selection;
	switch (selection)
	{
	case 1:
		if (encode())
			cout << "Encrypted successfully!" << endl;
		else
			cout << "Encryption failure" << endl;
		break;
	case 2:
		if (decode())
			cout << "Decrypted successfully!" << endl;
		else
			cout << "Decryption failure" << endl;
		break;
	default:
		cout << "Wrong number" << endl;
		break;
	}
	return 0;
}

bool encode()
{
	int block_size;
	int count_block = 0, count_arr = 0;
	int result[3000] = { 0 };
	cout << "Enter the block size: ";
	cin >> block_size;
	fstream sourcefile("source_text.txt", ios::in | ios::binary);
	if (!sourcefile.is_open())
	{
		cout << "Coudn't open the source file!" << endl;
		return false;
	}
	fstream codedfile("encrypted.txt", ios::out | ios::binary);
	if (!codedfile.is_open())
	{
		cout << "Coudn't open the encrypted file!" << endl;
		return false;
	}
	int text_size = 0;
	char* text = ConvertToArr(&sourcefile, &text_size);
	vector<int> Binary = ToBin(text, text_size);
	int control = DefControlBits(block_size);
	for (int i = 0; i < ceil((double)(text_size * 8) / (double)block_size); i++)
	{
		vector<int> Block = InsertControlBits(&Binary, block_size, control, text_size, &count_block);
		SetControlBits(&Block, block_size, control);
		WriteToArray(result, Block, block_size + control, &count_arr);
	}
	WriteToFile(result, &codedfile, count_arr);
	sourcefile.close();
	codedfile.close();
	return true;
}

bool decode()
{
	int block_size;
	int count_bits = 0, count_result = 0;
	cout << "Enter block size: ";
	cin >> block_size;
	fstream inputfile("encrypted.txt", ios::in | ios::binary);
	if (!inputfile.is_open())
	{
		cout << "Couldn't open the input file" << endl;
		return false;
	}
	fstream outputfile("decrypted.txt", ios::out | ios::binary);
	if (!outputfile.is_open())
	{
		cout << "Couldn't open the output file" << endl;
		return false;
	}
	int text_size = 0;
	int result[3000];
	char* text = ConvertToArr(&inputfile, &text_size);
	int control = DefControlBits(block_size);
	vector<int> Bin_orig = ToBin(text, text_size);
	while (Bin_orig.size() % (block_size + control) != 0)
	{
		Bin_orig.pop_back();
	}
	for (int i = 0; i < (text_size * 8) / (block_size + control); i++)
	{
		vector<int> Block = CopyEncryptBlock(Bin_orig, block_size, control, &count_bits);
		vector<int> Block_cpy = Block;
		SetControlBitsZero(&Block_cpy, block_size + control);
		SetControlBits(&Block_cpy, block_size, control);
		FixBits(&Block, &Block_cpy, control);
		WriteToResult(Block, result, block_size + control, &count_result);
	}
	WriteToFileResult(result, &outputfile, count_result);
	inputfile.close();
	outputfile.close();
	return true;
}

char* ConvertToArr(fstream* file, int* text_size)
{
	char ch = 0;
	int i = 0;
	char* text = (char*)malloc(1 * sizeof(char));
	ch = file->get();
	text[i] = ch;
	for (i = 1; !file->eof(); i++)
	{
		text = (char*)realloc(text, (i + 1) * sizeof(char));
		ch = file->get();
		text[i] = ch;
	}
	text[i] = '\0';
	*text_size = i - 1;
	return text;
}

vector<int> ToBin(char* text, int text_size)
{
	int character, m = 0;
	vector<int> Bin;
	for (int i = 0; i < text_size; i++)
	{
		for (int j = 7; j >= 0; j--)
		{
			character = CheckBit(text[m], j);
			Bin.push_back(character);
		}
		m++;
	}
	return Bin;
}

int CheckBit(const int byte, const int bit)
{
	if (byte & (1 << bit))
		return 1;
	return 0;
}

int DefControlBits(int block_size)
{
	int Log = ceil(log10((double)block_size) / log10((double)2));
	for (int i = 0; i <= 6; i++)
	{
		if (block_size - 1 == control_bits[i])
			return(Log + 1);
	}
	for (int i = 0; i <= 6; i++)
	{
		if (block_size + Log == control_bits[i] + 1)
			return (Log + 1);
	}
	return Log;
}

vector<int> InsertControlBits(vector<int>* Bin, int block_size, int control, int text_size, int* count)
{
	vector<int> Block;
	CopyToBlock(Bin, &Block, block_size, count);
	for (int i = 0; i < block_size + control; i++)
	{
		if (i == control_bits[0] || i == control_bits[1] || i == control_bits[2] || i == control_bits[3] || i == control_bits[4] || i == control_bits[5] || i == control_bits[6])
			Block.insert(Block.begin() + i, 0);
	}
	return Block;
}

void CopyToBlock(vector<int>* Bin, vector<int>* Block, int block_size, int* count)
{
	int count_here = *count;
	for (int i = 0; i < block_size; i++)
	{
		if (count_here + i < Bin->size())
		{
			Block->push_back(Bin->at(count_here + i));
			(*count)++;
		}
		else
		{
			Block->push_back(0);
			(*count)++;
		}
	}
}

void SetControlBits(vector<int>* Block, int block_size, int control)
{
	int sum = 0, last, k;
	for (int i = 0; i < control; i++)
	{
		sum = 0;
		last = control_bits[i];
		while (last < block_size + control)
		{
			for (k = 0; k < control_bits[i] + 1 && last + k < block_size + control; k++)
			{
				sum += Block->at(last + k);
			}
			last = last + k;
			last += control_bits[i] + 1;
		}
		if (sum % 2 == 0)
			Block->at(control_bits[i]) = 0;
		else
			Block->at(control_bits[i]) = 1;
	}
}

void WriteToArray(int* arr, vector<int> Block, int size, int* count)
{
	for (int i = 0; i < size; i++)
	{
		arr[*count] = Block.at(i);
		(*count)++;
	}
}

int SetBitOne(const int byte, const int bit)
{
	return (byte | (1 << bit));
}

int SetBitZero(const int byte, const int bit)
{
	return (byte & ~(1 << bit));
}

void WriteToFile(int* arr, fstream* file, int size)
{
	char ch = 0;
	while (size % 8 != 0)
	{
		size++;
	}
	for (int i = 0; i < size;)
	{
		for (int k = 7; k >= 0; k--)
		{
			if (arr[i])
				ch = SetBitOne(ch, k);
			else
				ch = SetBitZero(ch, k);
			i++;
		}
		file->put(ch);
	}
}

void SetControlBitsZero(vector<int>* Bin, int size)
{
	for (int i = 0; i < size; i++)
	{
		if (i == control_bits[0] || i == control_bits[1] || i == control_bits[2] || i == control_bits[3] || i == control_bits[4] || i == control_bits[5] || i == control_bits[6])
		{
			Bin->at(i) = 0;
		}
	}
}

vector<int> CopyEncryptBlock(vector<int> Bin, int block_size, int control, int* count)
{
	vector<int> Block;
	int count_here = *count;
	for (int i = 0; i < block_size + control; i++)
	{
		Block.push_back(Bin.at(count_here + i));
		(*count)++;
	}
	return Block;
}

void FixBits(vector<int>* Block, vector<int>* Block_cpy, int size)
{
	int sum = 0;
	for (int i = 0; i < size; i++)
	{
		if (Block->at(control_bits[i]) != Block_cpy->at(control_bits[i]))
		{
			sum += control_bits[i] + 1;
		}
	}
	if (sum)
	{
		if (Block->at(sum - 1) == 1)
			Block->at(sum - 1) = 0;
		else
			Block->at(sum - 1) = 1;
	}
}

void WriteToResult(vector<int> Bin, int* arr, int size, int* count)
{
	for (int i = 0; i < size; i++)
	{
		if (!(i == control_bits[0] || i == control_bits[1] || i == control_bits[2] || i == control_bits[3] || i == control_bits[4] || i == control_bits[5] || i == control_bits[6]))
		{
			arr[*count] = Bin.at(i);
			(*count)++;
		}
	}
}

void WriteToFileResult(int* arr, fstream* file, int size)
{
	char ch = 0;
	while (size % 8 != 0)
	{
		size--;
	}
	for (int i = 0; i < size;)
	{
		for (int k = 7; k >= 0; k--)
		{
			if (arr[i])
				ch = SetBitOne(ch, k);
			else
				ch = SetBitZero(ch, k);
			i++;
		}
		file->put(ch);
	}
}