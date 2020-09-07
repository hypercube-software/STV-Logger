#include<iostream>
#include "Logger.h"

namespace HypercubeSoftware {
    class MyClass {
    public: 
        void hello()
        {
            m_logger->error("Error receiving packet: %d", 100);
        }
    private:
        std::unique_ptr<Logger> m_logger = LogManager::Instance().getLogger("MyClass");

    };
}
int main(int argc, char *argv[]){
   LogManager::Instance().setLevel(LogLevel::LL_DEBUG);
   LogManager::Instance().open("./STV-Logger");
   HypercubeSoftware::MyClass c;
   c.hello();
   return 0;
}