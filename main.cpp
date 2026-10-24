/*
* Copyright (C) 2026 @SbitW
* This program is free software: you can redistribute it and/or modify 
* it under the terms of the GNU General Public License as published by 
* the Free Software Foundation, either version 3 of the License, or 
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful, 
* but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License 
* along with this program. If not, see <https://gnu.org>.
*/
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <filesystem>
#include "external/json.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

struct TodoUnit {
	std::string title;
	std::string desc;
	bool done;
	NLOHMANN_DEFINE_TYPE_INTRUSIVE(TodoUnit, title, desc, done)
};
fs::path initDataFile(){
	std::string configDirEnv;
#if defined(_WIN32)
	const char* appData = std::getenv("APPDATA");
	if(!appData) throw std::runtime_error("環境変数APPDATAが見つかりませんでした");
	configDirEnv = appData;
#else
	const char* home = std::getenv("HOME");
	if(!home) throw std::runtime_error("環境変数HOMEが見つかりませんでした");
	configDirEnv = std::string(home) + "/.config";
#endif
	fs::path appDir = fs::path(configDirEnv) / "todo-cpp-cui";
	fs::path filePath = appDir / "todos.json";
	fs::create_directories(appDir);
	if(!fs::exists(filePath)){
		std::ofstream outFile(filePath);
		outFile << "[]\n";
	}
	return filePath;
}

int main(int argc, char* argv[]){
	if(argc < 2){
		std::cout << "コマンドを指定してください。\n"
			  << "コマンド一覧:\n\tadd\n\tlist\n\tdone\n\terase\n";
		return 1;
	}
	fs::path dataPath;
	try{
		dataPath = initDataFile();
	}catch (const std::exception& e){
		std::cerr << "データファイルの初期化に失敗しました:" << e.what() << "\n";
		return 1;
	}
	std::string command = argv[1];
	auto readTodos = [](const fs::path& path){
		std::ifstream inFile(path);
		json j;
		inFile >> j;
		return j.get<std::vector<TodoUnit>>();
	};
	auto writeTodos = [](const fs::path& path, const std::vector<TodoUnit>& todos){
		std::ofstream outFile(path);
		json j = todos;
		outFile << j.dump(2) << "\n";
	};
	if(command == "add"){
		if(argc < 4){
			std::cout << "使用法:cptodo add [タイトル] [内容]\n";
			return 1;
		}
		std::string title = argv[2];
		std::string desc = argv[3];
		std::vector<TodoUnit> todos = readTodos(dataPath);
		for(const auto& item : todos){
			if(item.title == title && !item.done){
				std::cout << "未完了のタスク " << title << " はすでに存在します。\n";
				return 1;
			}
		}
		todos.push_back({title, desc, false});
		writeTodos(dataPath, todos);
		std::cout << "操作は正常に完了しました。\n";
	}else if(command == "list"){
		std::vector<TodoUnit> todos = readTodos(dataPath);
		bool showAll = false;
		if(argc > 2 && std::string(argv[2]) == "all"){
			showAll = true;
		}
		for(const auto& item : todos){
			if(!showAll && item.done){
				continue;
			}
			std::string status = item.done ? "[DONE]" : "[TODO]";
			std::cout << status << " " << item.title << " (" << item.desc << ")\n";
		}
	}else if(command == "done"){
		if(argc < 3){
			std::cout << "使用法:cptodo done [タイトル]\n";
			return 1;
		}
		std::string title = argv[2];
		std::vector<TodoUnit> todos = readTodos(dataPath);
		bool found = false;
		for(auto& item : todos){
			if(item.title == title && !item.done){
				item.done = true;
				found = true;
				break;
			}
		}
		if(!found){
			std::cout << "タスク " << title << " は見つかりませんでした。\n";
			return 1;
		}
		writeTodos(dataPath, todos);
		std::cout << "操作は正常に完了しました。\n";
	}else if(command == "erase"){
		if(argc < 3){
			std::cout << "使用法:cptodo erase [タイトル]\n";
			return 1;
		}
		std::string title = argv[2];
		std::vector<TodoUnit> todos = readTodos(dataPath);
		std::vector<TodoUnit> keptTodos;
		bool found = false;
		for(const auto& item : todos){
			if(item.title == title){
				found = true;
				continue;
			}
			keptTodos.push_back(item);
		}
		if(!found){
			std::cout << "タスク " << title << " は見つかりませんでした。\n";
			return 1;
		}
		writeTodos(dataPath,keptTodos);
		std::cout << "操作は正常に完了しました。\n";
	}else{
		std::cout << "不明なコマンド:" << command << "\n";
		return 1;
	}
	return 0;
}
