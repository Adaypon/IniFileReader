#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <exception>

class IniFile {
    std::map<
        std::string,
        std::map<std::string, std::string>
    > _data;

	const std::vector<std::string> _validBoolTrueNames = {"true", "True", "TRUE", "on", "On", "ON", "yes", "Yes", "YES", "y", "Y", "1"};
public:
	IniFile(std::string path);
	
	void save(std::string path) const;
	
    /**
     * Считывает из секции section
     * значение ключа key
     * Если не удалось считать - возвращает default value
     * 
     * Определить для std::string, int, float, bool
     */
	template<typename T>
	T read(std::string section, std::string key, T defaultValue = T{}) const;
	
    /**
     * В ключ key из секции section 
     * записывает значение value
     *
     * Определить для std::string, int, float, bool
     */
	template<typename T>
	void write(std::string section, std::string key, T value);

    /**
     * Проверяет, существует ли секция section
     */
    bool sectionExists(std::string section) const;

    /**
     * Проверяет, существует ли ключ key в секции section
     */
    bool keyExists(std::string section, std::string key) const;

    /**
     * Возвращает существующие секции
     */
    std::vector<std::string> sections() const;

    /**
     * Возвращает существующие ключи в секции section
     */
    std::vector<std::string> keys(std::string section) const;
};

// Считывание bool:
// true -  true, True, TRUE, on, On, ON, yes, Yes, YES, y, Y, 1
// false - остальные


IniFile::IniFile(std::string path) {
	std::string str;
	std::vector<std::string> fileCache;

	std::ifstream fin;
	fin.open(path);
	if (fin.is_open()) {
		while (!fin.eof()) {
			if (getline(fin, str) && !(str.length() == 1 && isspace(str[0]))) {
				fileCache.push_back(str);
			}
		}
	}
	else {
		std::cerr << "Can't open file " << path << std::endl;
		throw std::invalid_argument("Unable to load the file");
	}
	fin.close();

	for (size_t i = 0; i < fileCache.size(); ++i) {
		std::cout << fileCache[i] << std::endl;
	}
	
	std::string strSection;
	std::string strKey;
	std::string strValue;

	for (size_t i = 0; i < fileCache.size(); ++i) {
		size_t first = 0;
		size_t last = 0;

		// insert section 
		if (fileCache[i][0] == '[') {
			first = 1;
			last = fileCache[i].find_last_of(']');
			strSection = fileCache[i].substr(first, last - first);
			std::cout << strSection << std::endl;
			_data.insert(std::pair<std::string, std::map<std::string, std::string>>(strSection, {}));
		}
		// insert params
		else {
			last = fileCache[i].find('=');
			strKey = fileCache[i].substr(first, last);
			std::cout << strKey << " ";
			std::swap(first, last);
			++first;
			last = fileCache[i].size() - 1;
			strValue = fileCache[i].substr(first, last);
			if (!strValue.empty() && strValue[strValue.size() - 1] == '\r') {
    			strValue.erase(strValue.size() - 1);
			}
			std::cout << strValue << std::endl;
			_data.at(strSection).insert(std::pair<std::string, std::string>(strKey, strValue));
		}
	}
	
	std::cout << std::endl;
	for (const auto &elem : _data) {
   		std::cout << elem.first << std::endl;
		for (const auto &elem2 : elem.second) {
			std::cout << elem2.first << " " << elem2.second << std::endl;
		}
	}
	
}

void IniFile::save(std::string path) const {
	std::ofstream fout;
	fout.open(path);
	if (fout.is_open()) {
		for (const auto& elem: _data) {
			fout << "[" << elem.first << "]" << std::endl;
			for (const auto& elem2: _data.at(elem.first)) {
				fout << elem2.first << "=" << elem2.second << std::endl;
			}
			fout << std::endl;
		}
	}
	else {
		std::cerr << "Can't open file" << std::endl;
	}
	fout.close();
}


bool IniFile::sectionExists(std::string section) const {
	auto it = _data.find(section);
	return it != _data.end();
}

bool IniFile::keyExists(std::string section, std::string key) const {
	if (!sectionExists(section)) {
		return false;
	}

	auto it = _data.at(section).find(key);
	return it != _data.at(section).end();
}

std::vector<std::string> IniFile::sections() const {
	std::vector<std::string> sections;
	for (auto& elem : _data) {
		sections.push_back(elem.first);
	}
	return sections;
}

std::vector<std::string> IniFile::keys(std::string section) const {
	std::vector<std::string> keys;
	if (sectionExists(section)) {
		for (auto& elem : _data.at(section)) {
			keys.push_back(elem.first);
		}
	}
	return keys;
}

template<>
std::string IniFile::read(std::string section, std::string key, std::string defaultValue) const {
	if (sectionExists(section) && keyExists(section, key)) {
		return _data.at(section).at(key);
	}
	return defaultValue;
}

template<>
int IniFile::read(std::string section, std::string key, int defaultValue) const {
	if (sectionExists(section) && keyExists(section, key)) {
		// TODO check if value can be returned as int
		return std::stoi(_data.at(section).at(key));
	}
	return defaultValue;
}

template<>
float IniFile::read(std::string section, std::string key, float defaultValue) const {
	if (sectionExists(section) && keyExists(section, key)) {
		// TODO check if value can be returned as float
		return std::stof(_data.at(section).at(key));
	}
	
	return defaultValue;
}


template<>
bool IniFile::read(std::string section, std::string key, bool defaultValue) const {
	if (sectionExists(section) && keyExists(section, key)) {
		auto it = std::find(_validBoolTrueNames.begin(), _validBoolTrueNames.end(), _data.at(section).at(key));
		return it != _validBoolTrueNames.end();
	}
	return defaultValue;
}

template<>
void IniFile::write(std::string section, std::string key, std::string value) {
	if (sectionExists(section)) {
		if (keyExists(section, key)) {
			_data.at(section).at(key) = value;
		}
		else {
			_data.at(section).insert(std::pair<std::string, std::string>(key, value));
		}
	}
	else {
		_data.insert(std::pair<std::string, std::map<std::string, std::string>>(section, {}));
		_data.at(section).insert(std::pair<std::string, std::string>(key, value));
	}
}

template<>
void IniFile::write(std::string section, std::string key, int value) {
	if (sectionExists(section)) {
		if (keyExists(section, key)) {
			_data.at(section).at(key) = std::to_string(value);
		}
		else {
			_data.at(section).insert(std::pair<std::string, std::string>(key, std::to_string(value)));
		}
	}
	else {
		_data.insert(std::pair<std::string, std::map<std::string, std::string>>(section, {}));
		_data.at(section).insert(std::pair<std::string, std::string>(key, std::to_string(value)));
	}
}

template<>
void IniFile::write(std::string section, std::string key, float value) {
	if (sectionExists(section)) {
		if (keyExists(section, key)) {
			_data.at(section).at(key) = std::to_string(value);
		}
		else {
			_data.at(section).insert(std::pair<std::string, std::string>(key, std::to_string(value)));
		}
	}
	else {
		_data.insert(std::pair<std::string, std::map<std::string, std::string>>(section, {}));
		_data.at(section).insert(std::pair<std::string, std::string>(key, std::to_string(value)));
	}
}

template<>
void IniFile::write(std::string section, std::string key, bool value) {
	std::string strValue = value ? "true" : "false";
	if (sectionExists(section)) {
		if (keyExists(section, key)) {
			_data.at(section).at(key) = strValue;
		}
		else {
			_data.at(section).insert(std::pair<std::string, std::string>(key, strValue));
		}
	}
	else {
		_data.insert(std::pair<std::string, std::map<std::string, std::string>>(section, {}));
		_data.at(section).insert(std::pair<std::string, std::string>(key, strValue));
	}
}