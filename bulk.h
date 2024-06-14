#pragma once
#include <string>
#include <iostream>
#include <chrono>
#include <vector>
#include <memory>
#include <fstream>
#include <thread>
#include <sstream>

class bulk
{
public:
    bulk(std::string firstCommand, std::chrono::time_point<std::chrono::system_clock> timeStart = std::chrono::system_clock::now());
    ~bulk() = default;

    void addComand(std::string newCommand);
    const std::chrono::time_point<std::chrono::system_clock> getStartTime() const;
    std::string getCommandsStr() const;
    std::size_t getCommandsSize() const;

private:
    const std::chrono::time_point<std::chrono::system_clock> m_startTime;
    std::vector<std::string> m_commands;
};

class printer
{
public:
    printer() = default;
    virtual void printBulk(std::shared_ptr<bulk> curBulk) = 0;
};

class consolePrinter: printer
{
    public:
    consolePrinter();
    void printBulk(std::shared_ptr<bulk> curBulk) override;
};

class filePrinter: printer
{
public:
    filePrinter();
    void printBulk(std::shared_ptr<bulk> curBulk) override;
};

class commandController
{
public:
    commandController(std::size_t N);
    void addCommand(std::string newCommand);
    bool isFinish();
private:
    void printBulk();

    std::size_t m_N;
    bool m_finish = false;
    std::shared_ptr<bulk> m_currentBulk;
    std::vector<std::unique_ptr<printer>> m_printers;
    int startBlockAmount;
    int endBlockAmmount;
};