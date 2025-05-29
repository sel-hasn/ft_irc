#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Server.hpp" 

class Client{
    private:
        
        // client() //parameterize
        std::string Name;
        std::string Pass;
        std::string realName;
        std::string UserName;
        std::string BUFFER;
        bool        isRegistered;
        int         ClientSocketfd;
        bool        hasPass;
        bool        hasUserName;
        bool        hasName;
        bool        hasrealName;
        sockaddr_in ClientAddress;
    
    public:
        //getters
        std::string getName() const { return Name;  };

        std::string getPass() const { return Pass; };

        std::string getrealName() const{  return realName; };

        std::string getUserName() const{ return UserName; };

        std::string getBUFFER() const { return BUFFER; }

        std::string getHostname() {
            return Name + "!" + UserName + "@localhost";
        }

        bool getisRegistered() const{ return isRegistered; } 

        int  getClientSocketfd() const{  return ClientSocketfd; }

        bool gethasName() const{  return hasName; }

        bool gethasPass() const{ return hasPass; }

        bool gethasUserName() const { return hasUserName; }

        bool gethasrealname() const{ return hasUserName;  }

        sockaddr_in getClientAddress() const{ return ClientAddress; };

        /* setters */
        void setName(std::string name){  Name = name; }

        void setPass(std::string pass){ Pass = pass; }

        void setrealName(std::string rname){ realName = rname; }

        void setUserName(std::string uname){ UserName = uname;  }

        void setBuff(std::string buff){ BUFFER = buff; }

        void setRegister(bool stat){ isRegistered = stat; }

        void setClientsock(int fd){ ClientSocketfd = fd; }

        void sethasPass(bool stat){  hasPass = stat; }

        void sethasUname(bool stat){ hasUserName = stat; }

        void sethasName(bool stat){ hasName = stat; }

        void sethasrealName(bool stat){ hasrealName = stat; }

        void setClientAddress(sockaddr_in newaddr){ ClientAddress = newaddr; }

        Client(const Client& other){
            Name = other.Name;
            Pass = other.Pass;
            realName = other.realName;
            UserName = other.UserName;
            BUFFER = other.BUFFER;
            isRegistered = other.isRegistered;
            ClientSocketfd = other.ClientSocketfd;
            hasPass = other.hasPass;
            hasUserName = other.hasUserName;
            hasName = other.hasName;
            hasrealName = other.hasrealName;
            ClientAddress = other.ClientAddress;
        };    
        Client& operator=(const Client& other){
            if (this != &other){
                this->setBuff(other.getBUFFER());
                // this->
            }
            return *this;
        };
        Client(){
            Name = "defaultNick";
            Pass = "";
            realName = "de-real";
            UserName = "user-real";
            BUFFER = "";
            ClientSocketfd = -1;
            isRegistered = false;
            hasPass = false;
            hasUserName = false;
            hasName = false;
            hasrealName = false;
        };
        ~Client(){
            // std::cout << "client dest() called !\n";
       };
};

#endif