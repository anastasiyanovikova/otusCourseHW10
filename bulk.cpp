#include "bulk.h"
#include <iostream>
#include "asyncPrinter.h"

bulk::bulk(std::string firstCommand, std::chrono::time_point<std::chrono::system_clock> timeStart):
m_startTime(timeStart)
{
    m_commands.push_back(firstCommand);
}

void bulk::addComand(std::string newCommand)
{
    m_commands.push_back(newCommand);
}
const std::chrono::time_point<std::chrono::system_clock> bulk::getStartTime() const
{
    return m_startTime;
}
std::string bulk::getCommandsStr() const
{
    std::string res = "bulk: ";
    for(std::size_t i = 0; i < m_commands.size() - 1; i++)
        res = res + m_commands.at(i) + ", ";
    res = res + m_commands.at(m_commands.size() - 1);
    return res;
}

std::size_t bulk::getCommandsSize() const
{
  return m_commands.size();
}

commandController::commandController(std::size_t N): m_N(N), startBlockAmount(0), endBlockAmmount(0)
{
    m_printers.push_back(std::make_unique<assyncPrinter<consolePrinter>>());
    m_printers.push_back(std::make_unique<assyncPrinter<filePrinter>>(2));
}

void commandController::addCommand(std::string newCommand)
{
    if(m_finish)
        return;

    if(newCommand == "EOF")
    {
      if (startBlockAmount == 0)
      {
        printBulk();
        m_finish = true;
      }
      return;
    }
    else if(newCommand.find("{") != std::string::npos)
    {
      if (startBlockAmount == 0 && m_currentBulk && m_currentBulk->getCommandsSize() > 0)
      {
        printBulk();
      }
      startBlockAmount++;
    }
    else if(newCommand.find("}") != std::string::npos)
    {
      if (startBlockAmount > 0)
      {
        endBlockAmmount++;
        if (endBlockAmmount == startBlockAmount)
        {
          printBulk();
          startBlockAmount = 0;
          endBlockAmmount = 0;
        }
      }
    }
    else
    {
      if(m_currentBulk)
        m_currentBulk->addComand(newCommand);
      else{
        m_currentBulk = std::make_unique<bulk>(newCommand);
      }
      if (startBlockAmount == 0 && m_currentBulk && m_currentBulk->getCommandsSize() == m_N)
      {
        printBulk();
      }
    }
}
bool commandController::isFinish()
{
    return m_finish;
}
void commandController::printBulk()
{
    if(m_currentBulk)
    {
        for(auto &&printer: m_printers)
            printer->printBulk(m_currentBulk);
        m_currentBulk = {};
    }
}

consolePrinter::consolePrinter()
{
}

void consolePrinter::printBulk(std::shared_ptr<bulk> curBulk)
{
    std::cout<<curBulk->getCommandsStr()<<std::endl;
}

filePrinter::filePrinter()
{
}

void filePrinter::printBulk(std::shared_ptr<bulk> curBulk)
{
    std::ostringstream ss;
    ss << std::this_thread::get_id();
    std::string fileStr = "bulk" + std::to_string(curBulk->getStartTime().time_since_epoch().count()) + "-" + ss.str() + ".log";
    auto file = std::ofstream(fileStr);
    if(file.is_open())
        file << curBulk->getCommandsStr();
    file.close();
}