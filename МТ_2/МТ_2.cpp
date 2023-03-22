#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <string>
#include <cctype>

using namespace std;

static const string Format = ".txt";
static const string Keywords = "Keywords";
static const string Separators = "Separators";
static const string OperationSigns = "OperationSigns";
static const string VariableFile = "VariableTable";
static const string TokensFile = "Tokens";
static const string OutFileFormat = "Out";


#pragma region ConstTable

class ConstTable
{
public:
	ConstTable(const string& fileName)
	{
		CreateTable(fileName);
	};
	~ConstTable()
	{
		_table.clear();
	};

	int IndexByWord(const string& text);
	string WordByIndex(int Index);

	void PrintTable(const string& outFileName);
private:
	vector<string> _table;

	int IsContains(int Index);
	void CreateTable(const string& fileName);
};

void ConstTable::CreateTable(const string& fileName)
{
	ifstream fin(fileName);
	if (!fin.is_open())
		cerr << "File didn't openned";

	string s;
	while (fin >> s)
		_table.push_back(s);

	fin.close();
}

int ConstTable::IndexByWord(const string& text)
{
	for (int i = 0; i < _table.size(); i++)
		if (_table[i] == text)
			return i;
	return -1;
}

string ConstTable::WordByIndex(int Index)
{
	return IsContains(Index) == -1 ? string() : _table[Index];
}

int ConstTable::IsContains(int Index)
{
	return Index >= 0 && Index < _table.size() ? Index : -1;
}

void ConstTable::PrintTable(const string& outFileName)
{
	ofstream fout(outFileName);
	int i = 0;
	for (auto& el : _table)
		fout << ++i << " " << el << endl;
	fout.close();
}

#pragma endregion

#pragma region VariableTable

ostream& operator << (ostream& out, pair<bool, int> el)
{
	return out << "{ " << el.first << ", " << el.second << " }" << endl;
}

struct Lexeme
{
	string name;
	bool type = false;
	int value = -1;

	Lexeme(const string& Name)
	{
		name = Name;
	}

	Lexeme() {}

	friend bool operator == (const Lexeme& leftOperand, const Lexeme& rightOperand)
	{
		return
			leftOperand.name == rightOperand.name &&
			leftOperand.type == rightOperand.type &&
			leftOperand.value == rightOperand.value;
	}

	friend bool operator != (const Lexeme& leftOperand, const Lexeme& rightOperand)
	{
		return !(leftOperand == rightOperand);
	}

	friend ostream& operator << (ostream& out, const Lexeme& el)
	{
		if (!el.name.empty())
		{
			return out << "{ "
				<< el.name << ", "
				<< (el.type ? string("int") : string("Unknow type")) << ", "
				<< (el.value == -1 ? -1 : el.value) << " }" << endl;
		}
		return out << "The element doesn`t exist!";
	}
};

class VariableTable
{
public:
	VariableTable()
	{
		CreateEmptyTable();
	};
	~VariableTable()
	{
		_table.clear();
	};

	int SearchByWord(const string& NameVariable);

	Lexeme SearchByIndex(int Index);

	void SetAttribute(const string& NameVariable, bool TypeVariable, int ValueVariable);
	void SetAttribute(int Index, bool Type);
	void SetAttribute(int Index, int Value);

	pair<bool, int> GetAttribute(int Index);
	string SearchByNumber(int Index);

	void PrintTable(const string& outFileName);
private:
	vector<Lexeme> _table;

	void CreateEmptyTable();
	void AddingElement(Lexeme El);

	int IsContains(const string& NameEl);
	int IsContains(const int& Index);
};

void VariableTable::CreateEmptyTable()
{
	_table.resize(0);
}

void VariableTable::AddingElement(Lexeme El)
{
	_table.push_back(El);
}

void VariableTable::SetAttribute(const string& NameVariable, bool TypeVariable, int ValueVariable)
{
	auto indexElement = SearchByWord(NameVariable);
	_table[indexElement].type = TypeVariable;
	_table[indexElement].value = ValueVariable;
}

void VariableTable::SetAttribute(int Index, bool Type)
{
	if (IsContains(Index) == -1)
		return;
	_table[Index].type = Type;
}

void VariableTable::SetAttribute(int Index, int Value)
{
	if (IsContains(Index) == -1)
		return;
	_table[Index].value = Value;
}

void VariableTable::PrintTable(const string& outFileName)
{
	ofstream fout(outFileName);
	int i = 0;
	for (auto& el : _table)
		fout << "[" << ++i << "] " << el << endl;
	fout.close();
}

int VariableTable::IsContains(const string& NameEl)
{
	if (_table.size() == 0)
		return -1;

	auto x = find(_table.begin(), _table.end() - 1, NameEl);
	if (*x != _table.back() || _table.back().name == NameEl)
		return distance(_table.begin(), x);
	return -1;
}

int VariableTable::IsContains(const int& Index)
{
	return Index >= 0 && Index < _table.size() ? Index : -1;
}

int VariableTable::SearchByWord(const string& NameVariable)
{
	int indexElement = IsContains(NameVariable);
	if (indexElement == -1)
	{
		AddingElement(Lexeme(NameVariable));
		return static_cast<unsigned long long>(indexElement) + _table.size();
	}
	return indexElement;
}


Lexeme VariableTable::SearchByIndex(int Index)
{
	int indexElement = IsContains(Index);
	if (indexElement != -1)
	{
		return _table[indexElement];
	}
	return Lexeme({});
}

pair<bool, int> VariableTable::GetAttribute(int Index)
{
	auto Element = SearchByIndex(Index);
	return make_pair(Element.type, Element.value);
}

string VariableTable::SearchByNumber(int Index)
{
	return SearchByIndex(Index).name;
}

#pragma endregion

#pragma region Scanner

ostream& operator << (ostream& out, pair<int, int> el)
{
	return out << "(" << el.first << "," << el.second << ")";
}

class Scanner
{
public:
	Scanner(const string& ProgramName)
	{
		FileReader(ProgramName);
		CreateTokensFile();
	};

	~Scanner()
	{
		_progaramText.clear();
	};

private:

	enum SymbolState
	{
		LETTER,
		DIGIT,
		SLASH,
		SYMBOL
	};

	enum NumberTable
	{
		SeparatorTable = 0,
		KeywordTable = 1,
		OperationSignTable = 2,
		VariableT = 3
	};

	enum CommentStatus
	{
		OK,
		BAD,
		WATCHMORE,
		ALLBAD
	};

	ConstTable SeparatorsTable = (Separators + Format);
	ConstTable KeywordsTable = (Keywords + Format);
	ConstTable OperationSignsTable = (OperationSigns + Format);

	VariableTable VariableTable;

	vector<string> _progaramText;
	SymbolState _state;
	size_t _sizeWord = 0;

	void FileReader(const string& file);
	void CreateTokensFile();

	void SetState(char s);

	pair<int, int> LetterParseToToken(const string& str);
	pair<int, int> DigitParseToToken(const string& str);
	pair<int, int> SymbolParseToToken(const string& str);
	CommentStatus SlashParseToToken(const string& str);

	void ErrorHandling(int LineIndex, int ColumIndex);
};

void Scanner::FileReader(const string& file)
{
	ifstream fin(file);
	if (!fin.is_open())
		cerr << "File didn't openned";

	string str;

	while (!fin.eof())
	{
		getline(fin, str);
		_progaramText.push_back(str);
	}
}

void Scanner::CreateTokensFile()
{
	ofstream fout(TokensFile + Format);

	pair<int, int> token;
	CommentStatus st = OK;

	for (size_t i = 0; i < _progaramText.size(); ++i)
	{
		string currentStr = _progaramText[i];
		currentStr.push_back(' ');

		for (size_t j = 0; j < currentStr.size(); j += _sizeWord)
		{
			if (currentStr[j] == ' ')
			{
				_sizeWord = 1;
				continue;
			}

			_sizeWord = 0;

			if (st == WATCHMORE)
				if (currentStr.substr(j).find_first_of('*') == string::npos)
				{
					_sizeWord = currentStr.size();
					continue;
				}

			SetState(currentStr[j]);

			switch (_state)
			{
			case LETTER:
				token = LetterParseToToken(currentStr.substr(j));
	
				if (st != WATCHMORE)
					fout << token;
				break;

			case DIGIT:

				token = DigitParseToToken(currentStr.substr(j));

				if (token.first == -1)
				{
					ErrorHandling(i, j); // Встретили цифру
					continue;
				}

				if (st != WATCHMORE)
					fout << token;

				break;
			case SLASH:

				_sizeWord = currentStr.size();
				st = SlashParseToToken(currentStr.substr(j));

				if (st == Scanner::CommentStatus::WATCHMORE)
				{
					_sizeWord = currentStr.size();
					continue;
				}

				if (st == Scanner::CommentStatus::BAD)
				{
					st = WATCHMORE;
					ErrorHandling(i, j); // Ошибка комментария на строке
					continue;
				}

				if (st == Scanner::CommentStatus::ALLBAD)
				{
					ErrorHandling(i, j); // Не закрыт комментарий до конца файла
					exit(1);
				}

				break;
			case SYMBOL:
				token = SymbolParseToToken(currentStr.substr(j));

				if (token.first == -1)
				{
					ErrorHandling(i, j); // Ошибка символа следующего за символом операций или за символом разделителей
					continue;
				}

				if (st != WATCHMORE)
					fout << token;

				break;
			default:
				break;
			}
		}
		fout << endl;
	}

	if (st == Scanner::CommentStatus::WATCHMORE)
		ErrorHandling(1, 1); // Поменять цифры + Это означает что чубрик не закрыл комментарий до конца проги

	fout.close();
}

void Scanner::SetState(char s)
{
	if (isalpha(s))
	{
		_state = Scanner::SymbolState::LETTER;
		return;
	}

	if (isdigit(s))
	{
		_state = Scanner::SymbolState::DIGIT;
		return;
	}

	if (s == '/' || s == '*')
	{
		_state = Scanner::SymbolState::SLASH;
		return;
	}

	_state = Scanner::SymbolState::SYMBOL;
}

pair<int, int> Scanner::LetterParseToToken(const string& str)
{
	const static string letters = " |{|}|(|)|;|+|-|=|<|<=|>|>=";
	_sizeWord = str.find_first_of(letters);
	string word = str.substr(0, _sizeWord);

	int IndexTableElement = KeywordsTable.IndexByWord(word);

	if (IndexTableElement != -1)
		return make_pair(NumberTable::KeywordTable, IndexTableElement);

	IndexTableElement = VariableTable.SearchByWord(word);
	return make_pair(NumberTable::VariableT, IndexTableElement);
}

pair<int, int> Scanner::DigitParseToToken(const string& str)
{
	const static string letters = " |{|}|(|)|;|+|-|=|<|<=|>|>=";
	_sizeWord = str.find_first_of(letters);
	string word = str.substr(0, _sizeWord);

	char* res;
	if (strtol(word.c_str(), &res, 10) == 0) // Не работает "0" !!!
		return make_pair(-1, -1);

	int IndexTableElement = VariableTable.SearchByWord(word);

	return make_pair(NumberTable::VariableT, IndexTableElement);
}

Scanner::CommentStatus Scanner::SlashParseToToken(const string& str)
{
	if (str[1] == '/') // "//___"
	{
		if (str[0] == '*')
		{
			_sizeWord = 2;
			return Scanner::CommentStatus::OK;
		}
		return Scanner::CommentStatus::OK;
	}

	if (str[1] != '*') // "/_"
		return Scanner::CommentStatus::BAD;

	string tmpStr = str.substr(1);
	_sizeWord = tmpStr.rfind('*');

	if (_sizeWord == tmpStr.find('*'))	// "/*_"
		return Scanner::CommentStatus::WATCHMORE;

	if (tmpStr[_sizeWord + 1] == '/') // "/*___*/"
	{
		_sizeWord += 3;
		return Scanner::CommentStatus::OK;
	}
	
	return Scanner::CommentStatus::ALLBAD; // "/**"
}

pair<int, int> Scanner::SymbolParseToToken(const string& str)
{
	_sizeWord = str.find(' ');
	string word = str.substr(0, _sizeWord);

	if (str[0] == '<' || str[0] == '>')
	{
		if (str[1] == '=')
			return make_pair(NumberTable::OperationSignTable, OperationSignsTable.IndexByWord(word));

		if (SeparatorsTable.IndexByWord(string(1, str[1])) == -1 || KeywordsTable.IndexByWord(string(1, str[1])) == -1)
			return make_pair(NumberTable::OperationSignTable, OperationSignsTable.IndexByWord(word));

		return make_pair(-1, -1);
	}

	int iscontains = SeparatorsTable.IndexByWord(string(1, str[0]));

	if (iscontains != -1)
	{
		_sizeWord = 1;
		return make_pair(NumberTable::SeparatorTable, SeparatorsTable.IndexByWord(string(1, str[0])));
	}

	iscontains = OperationSignsTable.IndexByWord(string(1, str[0]));

	if (iscontains != -1)
	{
		_sizeWord = 1;
		return make_pair(NumberTable::OperationSignTable, OperationSignsTable.IndexByWord(string(1, str[0])));
	}

	return make_pair(-1, -1);
}

void Scanner::ErrorHandling(int LineIndex, int ColumIndex)
{

}

#pragma endregion

int main()
{
	Scanner t("fff.txt");
}