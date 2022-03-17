#include <string>
#include <sstream>
#include <vector>
#include "SkipList.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include <algorithm>
#include <fstream>

#define P(x) \
    {        \
        x, x \
    }

namespace App
{
    //控制窗口关闭
    bool showWindow = true;
    //跳表组
    std::vector<SkipList<int, int>> lists;
    //表格id
    std::vector<std::string> tableID;
    //命令
    char cmd[256] = "";
    char lastCmd[256] = "";
    //输出文件
    std::string outFile = "data/myoutput/output_0.txt";
    //比较次数
    int numCompare=0;

    //
    void copeFile(std::string &filename)
    {
        std::ifstream ifs(filename);
        outFile[21]=filename[17];
        std::ofstream ofs(outFile);
        if (!ifs.is_open() || !ofs.is_open())
        {
            std::cout << ifs.is_open() << std::endl;
            strcpy(lastCmd, "Failed to open file");
            return;
        }
        int M, N, key, op;
        std::optional<_Pair<int, int>> obj;
        ifs >> M >> N;
        //创建跳表
        lists.push_back(SkipList<int, int>());
        SkipList<int, int> &list = lists[lists.size() - 1];
        //输入数据
        for (int i = 0; i < N; i++)
        {
            ifs >> key;
            list.insert(P(key));
        }
        //操作
        for (int i = 0; i < M; i++)
        {
            int sumXOR = 0;
            ifs >> op;
            switch (op)
            {
            //查找
            case 1:
                std::cout << "search" << std::endl;
                ifs >> key;
                if (list.find(key)!=list.end())
                    ofs << "YES" << std::endl;
                else
                    ofs << "NO" << std::endl;
                break;
            //插入
            case 2:
                std::cout << "insert" << std::endl;
                ifs >> key;
                list.insert(P(key));
                for (auto it = list.begin(); it != list.end(); it++)
                    sumXOR ^= it->key;
                ofs << sumXOR << std::endl;
                break;
            //删除
            case 3:
                std::cout<<"delete"<<std::endl;
                ifs >> key;
                list.erase(key);
                for (auto it = list.begin(); it != list.end(); it++)
                    sumXOR ^= it->key;
                ofs << sumXOR << std::endl;
                break;
            //删除最小
            case 4:
                std::cout<<"pop_front"<<std::endl;
                obj = list.pop_front();
                if (obj.has_value())
                    ofs << obj.value().key << std::endl;
                break;
            //删除最大
            case 5:
                std::cout<<"pop_back"<<std::endl;
                obj = list.pop_back();
                if (obj.has_value())
                    ofs << obj.value().key << std::endl;
                break;
            default:
                break;
            }
        }
        ifs.close();
        ofs.close();

        std::stringstream ss;
        strcpy(lastCmd,ss.str().c_str());
    }

    static std::string createID(int id)
    {
        std::string ret;
        std::stringstream ss;
        ss << id;
        ss >> ret;
        return ret;
    }

    static void extendTableID()
    {
        for (int i = tableID.size(); i < lists.size(); i++)
            tableID.push_back(createID(i));
    }

    void parseCmd(const char *command)
    {
        strcpy(lastCmd, App::cmd);
        std::stringstream ss;
        ss << command;
        std::string cmd;
        std::string filename;
        int key, value;
        int listID;
        int listID1, listID2;
        int num;
        ss >> cmd;
        if (cmd == "insert")
        {
            ss >> listID;

            int lastCompare=lists[listID].compareCount();

            while (ss >> key >> value)
                lists[listID].insert({key, value});

            numCompare=lists[listID].compareCount()-lastCompare;
        }
        else if (cmd == "erase")
        {
            ss >> listID;

            int lastCompare=lists[listID].compareCount();

            while (ss >> key)
                lists[listID].erase(key);

            numCompare=lists[listID].compareCount()-lastCompare;
        }
        else if (cmd == "settle")
        {
            for (auto &list : lists)
                list.settle();
        }
        else if (cmd == "search")
        {      
            ss >> listID;
            if(!(ss>>key))
            {
                strcpy(lastCmd,"invalid search");
                return;
            }

            int lastCompare=lists[listID].compareCount();

            auto it=lists[listID].find(key);
            ss.clear();
            ss.str("");
            if (it->key == key)
            {
                ss << "Found value: ";
                ss << it->value;
                strcpy(lastCmd, ss.str().c_str());
            }
            else
                strcpy(lastCmd, "Not found");

            numCompare=lists[listID].compareCount()-lastCompare;
        }
        else if (cmd == "exit")
        {
            showWindow = false;
        }
        else if (cmd == "create")
        {
            lists.push_back(SkipList<int, int>());
        }
        else if (cmd == "destroy")
        {
            ss >> listID;
            lists.erase(lists.begin() + listID);
        }
        else if (cmd == "swap")
        {
            ss >> listID1 >> listID2;
            std::swap(lists[listID1], lists[listID2]);
        }
        else if (cmd == "random")
        {
            ss >> num;
            lists.push_back(SkipList<int, int>());
            for (int i = 0; i < num; i++)
                lists[lists.size() - 1].insert(P(rand() % 10000));
        }
        else if (cmd == "file")
        {
            ss >> filename;
            copeFile(filename);
        }
        else if(cmd=="size")
        {
            ss>>listID;
            ss.clear();
            ss.str("");
            ss<<lists[listID].size();
            strcpy(lastCmd,ss.str().c_str());
        }
        else if(cmd=="level")
        {
            ss>>listID;
            ss.clear();
            ss.str("");
            ss<<lists[listID].level();
            strcpy(lastCmd,ss.str().c_str());
        }
        else if(cmd=="compare")
        {
            ss.clear();
            ss.str("");
            ss<<numCompare;
            strcpy(lastCmd,ss.str().c_str());
        }
        else if(cmd=="average")
        {
            ss>>listID;
            auto& curList=lists[listID];
            int lastCompare=curList.compareCount();
            
            //查找跳表中的所有元素
            for(const auto& p:curList)
                auto key=curList.find(p.key);
            
            //给出平均比较次数
            numCompare=curList.compareCount()-lastCompare;
            ss.clear();
            ss.str("");
            ss<<(float)numCompare/curList.size();
            strcpy(lastCmd,ss.str().c_str());
        }
        else
        {
            strcpy(lastCmd, "Unsolvable command");
            std::cout << "Incorrect command" << std::endl;
        }
        strcpy(App::cmd, "");
    }

    void windowInit()
    {
    }

    void windowShow()
    {
        ImGui::Begin("SkipList");

        // table
        const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
        static ImGuiTableFlags flags = ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
        static int freeze_cols = 1;
        static int freeze_rows = 1;
        ImVec2 outer_size = ImVec2(0.0f, TEXT_BASE_HEIGHT * 8);

        //检查是否有新表
        if (lists.size() > tableID.size())
            extendTableID();

        int currentTable = 0;
        for (const auto &list : lists)
        {
            //过大，无法显示
            if (list.size() > 40)
            {
                ImGui::BeginTable(tableID[currentTable].c_str(), 2, flags, outer_size);
                ImGui::TableSetupColumn("List size out of range", ImGuiTableColumnFlags_NoHide);
                ImGui::TableHeadersRow();
                ImGui::EndTable();
                currentTable++;
                continue;
            }
            //显示预处理
            SkipList<int, int>::iterator p = list.begin(0);
            int *temp = new int[list.size()];
            int idx = 0;
            while (p != list.end())
            {
                temp[idx++] = p->key;
                p++;
            }
            idx = 0;
            //表格打印
            if (ImGui::BeginTable(tableID[currentTable].c_str(), list.size() + 1, flags, outer_size))
            {
                ImGui::TableSetupScrollFreeze(freeze_cols, freeze_rows);
                ImGui::TableSetupColumn("List", ImGuiTableColumnFlags_NoHide);
                ImGui::TableHeadersRow();
                for (int curLevel = list.level() - 1; curLevel >= 0; curLevel--)
                {
                    SkipList<int, int>::iterator it = list.begin(curLevel);
                    ImGui::TableNextRow();
                    for (int column = 0; column < list.size() + 1; column++)
                    {
                        if (!ImGui::TableSetColumnIndex(column) && column > 0)
                            continue;
                        if (column == 0)
                            ImGui::Text("Level %d", curLevel);
                        else
                        {
                            if (it->key == temp[idx])
                            {
                                ImGui::Text("%d: %d", it->key, it->value);
                                it++;
                            }
                            idx++;
                        }
                    }
                    idx = 0;
                }
            }
            delete[] temp;
            currentTable++;
            ImGui::EndTable();
        }
        ImGui::End();

        ImGui::Begin("Command Line");
        ImGui::Text("Supported operation: \n"
                    "/create                                                      /destroy [listID]                                         /random [listSize]\n"
                    "/swap [listID1] [listID2]                                    /insert [listID] [key] [value] [key] [value] ...          /settle\n"
                    "/search [listID] [key]                                       /erase [listID] [key0] [key1] ...                         /file [filepath]\n"                                  
                    "/compare                                                     /average [listID]\n"
                    "/size [listID]                                               /level [listID]                                           /exit");

        ImGui::InputText("command here", cmd, 256);
        ImGui::Text("%s", lastCmd);

        if (ImGui::IsKeyPressed(257))
            parseCmd(cmd);
        if (ImGui::IsKeyPressed(256))
            showWindow = false;

        ImGui::End();
    }
}