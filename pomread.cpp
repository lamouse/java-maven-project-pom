#include "pugixml.hpp"
#include <fstream>
#include <algorithm>
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <map>
#include <iomanip>

const std::string POM_FILE_NAME = "pom.xml";

const std::string TO_SPLIT = " --|> ";

using namespace std;
using namespace std::filesystem;

vector<path> getPomPaths(const directory_iterator& its);

vector<string> readPom(const path& p);

vector<string> split(string s, string delimiter){
    vector<string> result;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        result.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    result.push_back(s);
    return result;
}

int main(int argc, char* argv[]){
    if (argc < 2){
        cout << "invide param" << endl;
        exit(1);
    }
    
    path pomRootPath(argv[1]);
    if (!exists(pomRootPath)){
        cout << "path not found" << endl;
        exit(2);
    }

    directory_iterator pomRootList(pomRootPath);
    std::vector<path> pomPaths = getPomPaths(pomRootList);

    vector<string> deps;
    for (auto& p : pomPaths){
        auto dep = readPom(p);
        deps.insert(deps.end(), dep.begin(), dep.end());
    }

    for (auto& d : deps){
        cout << d << endl;
    }

    cout << "#-------base-------" << endl;

    vector<string> base;
    for (auto& d : deps){
        auto sp = split(d, TO_SPLIT);
        if(sp.empty()){
            continue;
        }
        base.push_back(sp[1] + TO_SPLIT + sp[0]);
    }

    sort(base.begin(), base.end());
    for (auto& b : base){
        cout << b << endl;
    }

    return 0;
}

vector<path> getPomPaths(const directory_iterator& its){
    std::vector<path> paths;
    for (auto& it : its){
        if (it.path().filename().string().rfind('.', 0) == 0)
        {
            continue;
        }
        directory_entry entry(it);
        if (entry.status().type() == file_type::directory){
            directory_iterator list(it);           //文件入口容器
            for (auto& l:list){
                if (l.path().filename().string().rfind(POM_FILE_NAME, 0) == 0){
                    paths.push_back(it);
                }
            }
        }else{
            continue;
        }
    }
    return paths;
}

vector<string> readPom(const path& p){
    vector<string> models;
    vector<string> result;
    std:string pomPath = std::filesystem::absolute(p).string() + "/" + POM_FILE_NAME;
    ifstream in(pomPath);
    string line;
    if (!in.is_open()){
        cerr << "open file " << pomPath << " error" << endl;
        exit(3);
    }
    pugi::xml_document doc;
    pugi::xml_parse_result xml = doc.load(in);
    pugi::xml_node tools = doc.child("project").child("modules");
    pugi::xml_node art = doc.child("project").child("artifactId");
    for (pugi::xml_node tool: tools.children("module"))
    {
        models.push_back(tools.child_value());
    }

    if (!models.empty())
    {
        directory_iterator pomRootList(p);
        std::vector<path> pomPaths = getPomPaths(pomRootList);

        for (auto& p : pomPaths){
            auto pomVec = readPom(p);
            result.insert(result.end(), pomVec.begin(), pomVec.end());
        }
    }

    pugi::xml_node deps = doc.child("project").child("dependencies");
    for (pugi::xml_node dep: deps.children("dependency"))
    {
        string depArtStr = dep.child("artifactId").child_value();
        if (dep.child("groupId").child_value() == string("cn.dfcx") && depArtStr.find("biz") == std::string::npos){
            string artStr = art.child_value();
            std::replace(artStr.begin(), artStr.end(), '-', '_');
            std::replace(depArtStr.begin(), depArtStr.end(), '-', '_');

            result.push_back(artStr +  TO_SPLIT  + depArtStr);
        }
    }
    in.close();

    return result;
}

