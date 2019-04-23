/*
make
./chat_server 9000
./chat_client 127.0.0.1 9000
*/

#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include "asio.hpp"
#include "chat_message.hpp"
#include <ncurses.h>
#include <string.h>
#include <string>
#include <math.h>
#include <chrono>
#include <time.h>
#include <fstream>

#include <signal.h>

using asio::ip::tcp;
using namespace std;
using namespace std::this_thread; //need for sleep_for
using namespace std::chrono; //need for nanoseconds
typedef std::deque<chat_message> chat_message_queue;

class chat_client
{
public:
  //initilize new chat_client class and call do_connect()
  chat_client(asio::io_context& io_context,
      const tcp::resolver::results_type& endpoints)
    : io_context_(io_context),
      socket_(io_context)
  {
    do_connect(endpoints);
  }
  //draws the prompt at the bottom left. ex - "Nick: "
  void prompt()
  {
      //refresh X,Y
      getScreen();

      // converting string to char[] then printing to screen
      string concat(getNick());
      int n = concat.length();
      char char_array[n + 1];
      strcpy(char_array, concat.c_str());
      clearRow(maxrow-1);
      mvprintw(maxrow-1,1," %s: ",char_array);
  }
  string checkRoomExists(string currentRoomSender)
  {
    fillVectors();
    clearChat();

    int found = 0;
    int len = roomList.size();

    for (int j = 0; j < len; j++)
    {
      if (currentRoomSender == roomList.at(j))
      {
        found = 1;
      }
    }
    if (found == 1)
    {
      currentRoom.assign(currentRoomSender);
      prompt();
      return "/";
    }
    else
    {
      int n = currentRoomSender.length();
      char char_array[n + 1];

      strcpy(char_array, currentRoomSender.c_str());
      mvprintw(row, col, "[%s] does not exist", char_array);
      prompt();
      //read user input
      char str[100];
      getstr(str);
      return str;
    }
    
  }
  //set local chat_client variable to new room name
  void changeRoom(string currentRoomSender)
  {
    currentRoom.assign(currentRoomSender);
  }
  //return local chat_client variable for current room name
  string getRoom()
  {
    return currentRoom;
  }
  //set user's account name
  void setNick(string nickSender)
  {
    nickName.assign(nickSender);
  }
  //get name from user. check for dupes
  void loginScreen(int dupe)
  {
    try
    {
      clearChat();
      getScreen();
      int error = 0;
      //initilize place to store user's account name
      char nameArray[20] = {0};

      if (getMemberCount() >= 50)
      {
        char welcome1[] = "Welcome to Super Chat";
        char welcome2[] = "*MAX USERS ONLINE*";
        char welcome3[] = "";
        mvprintw((maxrow / 2) - 3, (maxcol - strlen(welcome1)) / 2, "%s", welcome1);
        mvprintw(maxrow / 2, (maxcol - strlen(welcome2)) / 2, "%s", welcome2);
        mvprintw(maxrow-1, 1, "%s", welcome3);

        error = 1;
        prompt();
      }

  
      
      
      else if (dupe == 0)
      {
        char welcome1[]="Welcome to Super Chat";
        char welcome2[]="Enter user name: ";
        mvprintw((maxrow/2)-3,(maxcol-strlen(welcome1))/2,"%s",welcome1);
        mvprintw(maxrow/2,(maxcol-strlen(welcome2))/2,"%s",welcome2);
      }
      else
      {
        char welcome1[]="That name is already taken";
        char welcome2[]="Enter user name: ";
        mvprintw((maxrow/2)-3,(maxcol-strlen(welcome1))/2,"%s",welcome1);
        mvprintw(maxrow/2,(maxcol-strlen(welcome2))/2,"%s",welcome2);
      }
      
      getstr(nameArray);
      string nameStr(nameArray);
      int found = 0;
      int lenMembers = memberList.size();
      
      for (int j = 0; j<lenMembers;j++)
      {
        int n = memberListLength.at(j);
        string memberCompare(memberList.at(j));
        memberCompare.erase(n, memberCompare.length());
        if(nameStr == memberCompare)
        {
          found = 1;
          break;

        }
      }
      if (found == 1 || error == 1)
      {
        sleep_for(nanoseconds(1000000));
        loginScreen(1);
      }
      else
      {
        setNick(nameStr);
        setTimestamp(1);
        string msg(getTime() + ";Connected;" + getNick() + ";");
        sendEvent(msg);
        clearChat();
        showHeaderFooter();
        prompt();
      }
      
      
    }
    catch (char *excp)
    {
      cout << "Caught " << excp;
    }
  }

  //return local chat_client variable for account name
  string getNick()
  {
    return nickName;
  }
  int getTimestamp()
  {
    return timeINT;
  }
  void setTimestamp(int onSender)
  {
    timeINT = onSender;
  }
  string getTime()
  {
    time_t my_time;
    struct tm * timeinfo;
    time (&my_time);
    timeinfo = localtime (&my_time);
    int hour = timeinfo->tm_hour;
    int min = timeinfo->tm_min;
    char str[12];
    sprintf(str, "%02d:%02d", hour, min);
    return str;
  }
  //asio chat example for sending message to server
  //calls do_write()   (private function of chat_client)
  void write(const chat_message& msg)
  {
    asio::post(io_context_,
        [this, msg]()
        {
          bool write_in_progress = !write_msgs_.empty();
          write_msgs_.push_back(msg);
          if (!write_in_progress)
          {
            do_write();
          }
        });
  }
   //asio chat example for closing chat client connection to server
  void close()
  {
    asio::post(io_context_, [this]() { socket_.close(); });
  }
  //get screen size and initilize counters
  void getScreen()
  {
      getmaxyx(stdscr,maxrow,maxcol);
  }
  //for admin use. display raw event list
  string debug()
  {
    clearChat();

    string delimiter = ";";

    //get size of current event
    int logLength = sysLog.size();

    //loop through all events
    for (int i = logLength-1; i>=0;i--)
    {
      //move into local variable
      string str(sysLog.at(i));
      
      string dateTime = str.substr(0, str.find(delimiter));
      str.erase(0, str.find(delimiter)+1);

      int n = sysLogLength.at(i) + 2;

      //convert to char[]
      char char_array[n + 1];
      strcpy(char_array, str.c_str());

      int n2 = dateTime.length();
      char char_array2[n2 + 1];
      strcpy(char_array2, dateTime.c_str());

      printMsg(char_array, char_array2, n);
    }

    prompt();
    //read user input
    char str[100];
    getstr(str);
    return str;
  }
  //clear ncruses gui at a certain row
  void clearRow(int row)
  {
      for (int i = 0;i<maxcol;i++)
      {
          mvprintw(row, i, " ");
      }
  }
  void prvtMsg(string sender, string digitMsg, string msg)
  {
    int n = msg.length();
    char char_array[n + 1];
    strcpy(char_array, msg.c_str());
    mvprintw(row,1,"%s", char_array);
    //cleanup UI after the 32bit pointer
    for (int j = n+1; j<maxcol;j++)
    {
        mvprintw(row,j," ");
    }

    //increment down to next line
    if (row >= 2)
    {
        row = row - 1;
    }
    if (code == digitMsg)
    {
      mvprintw(row,1,"[%s]",msg);
    }
  }
  //set private msg digit for client
  void setDigit(string digitMsg)
  {
      code = digitMsg;

  }
  void printMsg(char msg[], char dateTime[], int msgSize)
  {
    //if it is just a regular chat message, then use this to print to screen
    if (timeINT == 1)
    {
      mvprintw(row,1,"[%s]",dateTime);
      mvprintw(row,9,"%s", msg);
    }
    else
    {
      msgSize=msgSize-8;
        mvprintw(row,1,"%s", msg);
    }
    //cleanup UI after the 32bit pointer
    for (int j = msgSize+1; j<maxcol;j++)
    {
        mvprintw(row,j," ");
    }

    //increment down to next line
    if (row >= 2)
    {
        row = row - 1;
    }
  }
  void readSystemLog()
  {
    //refresh data in ignore/member vectors
    fillVectors();
      //get size of current event
      int logLength = sysLog.size();

      //loop through all events
      for (int i = logLength-1; i>=0;i--)
      {

      //move into local variable
      string str(sysLog.at(i));

      //chosen format
      string delimiter = ";";
      //get first token
      string dateTime = str.substr(0, str.find(delimiter));
      //erase first token
      str.erase(0, str.find(delimiter)+1);


      //get second token
      string action = str.substr(0, str.find(delimiter));
      //erase second token
      str.erase(0, str.find(delimiter)+1);



        //see what type of action it is
        if (action == "ChatMsg")
        {
            //get second token
            string roomName = str.substr(0, str.find(delimiter));
            //erase second token
            str.erase(0, str.find(delimiter)+1);

            //CHECK FOR IGNORE
            int ignoreFound = 0;
            string user = str.substr(0, str.find(":"));
            int len = ignoreList.size();
            for (int j = 0; j<len;j++)
            {
              string userInListTemp(ignoreList.at(j));
              string userInList = userInListTemp.substr(0,ignoreListLength.at(j));
              if (user == userInList)
              {
                ignoreFound = 1;
              }
            }
            //is user in the same room as the msg?
            if (getRoom() == roomName && ignoreFound == 0)
            {
                //convert to char[]
                int n = sysLogLength[i] - action.length() - roomName.length();
                char char_array[n + 1];
                strcpy(char_array, str.c_str());

                int n2 = dateTime.length();
                char char_array2[n2 + 1];
                strcpy(char_array2, dateTime.c_str());

                printMsg(char_array, char_array2, n);
            }
      }
        else if (action == "Transfer")
        {
            //get second token
            string fileName = str.substr(0, str.find(delimiter));
            //erase second token
            str.erase(0, str.find(delimiter)+1);
            //get third token
            string sender = str.substr(0, str.find(delimiter));
            //erase third token
            str.erase(0, str.find(delimiter)+1);
            //get fourth token
            string recipient = str.substr(0, str.find(delimiter));
            //erase fourth token
            str.erase(0, str.find(delimiter)+1);

            string content = str.substr(0, str.find(delimiter));

            if (sender == getNick())
            {
                string msg = "*Sent file to " + recipient;
              int n = msg.length() + 8;
              char char_array[n + 1];
              strcpy(char_array, msg.c_str());

              int n2 = dateTime.length();
              char char_array2[n2 + 1];
              strcpy(char_array2, dateTime.c_str());

              printMsg(char_array, char_array2, n);
            }
            

            if (recipient == getNick())
            {
              ofstream fs;
              fs.open(fileName);
              fs <<  content;
              fs.close();
            


              string msg = "*Downloaded file from " + sender;
              int n = msg.length() + 8;
              char char_array[n + 1];
              strcpy(char_array, msg.c_str());

              int n2 = dateTime.length();
              char char_array2[n2 + 1];
              strcpy(char_array2, dateTime.c_str());

              printMsg(char_array, char_array2, n);
            }


        }
        else if (action == "Ignore")
        {
          //do nothing because this is handled in fillVectors()
        }
        else if (action == "Connected")
        {
          //do nothing because this is handled in fillVectors()
        }
        else if (action == "Disconnected")
        {
          //do nothing because this is handled in fillVectors()
        }
        else if (action == "DeleteRoom")
        {
          //do nothing because this is handled in fillVectors()
        }
        else if (action == "CreateRoom")
        {
          //do nothing because this is handled in fillVectors()
        }

        //display private messages to user, if INT code found
        else if (action == "PrivateMessage")
        {
          //get second token
          string sender = str.substr(0, str.find(delimiter));
          //erase second token//
          str.erase(0, str.find(delimiter)+1);
          //get third token
          string digit = str.substr(0, str.find(delimiter));
          //erase third token
          str.erase(0, str.find(delimiter)+1);
          //get fourth token
          string msg = str.substr(0, str.find(delimiter));
          //erase fourth token
          if (code == digit)
          {

            string finalMsg("(" + sender + "): " + msg);
            int n = msg.length() + sender.length() + 12;
            char char_array[n + 1];
            strcpy(char_array, finalMsg.c_str());

            int n2 = dateTime.length();
            char char_array2[n2 + 1];
            strcpy(char_array2, dateTime.c_str());

            printMsg(char_array, char_array2, n);
          }
        }
      prompt();
      }
  }
  int getMemberCount()
  {
    return memberList.size();
  }
  int getRoomCount()
  {
    return roomList.size();
  }
  //populate memberlist and ignorelist vectors
  //used for displaying both lists to user
  //also provides functionality to other functions
  void fillVectors()
  {
    
    memberList.clear();
    memberListLength.clear();
    ignoreList.clear();
    ignoreListLength.clear();
    roomList.clear();
    roomListLength.clear();
    //get size of current event
    int logLength = sysLog.size();

    //loop through all events
    for (int i = 0; i < logLength; i++)
    {
      //move into local variable
      string str(sysLog.at(i));

      //chosen format
      string delimiter = ";";

      //get first token
      string dateTime = str.substr(0, str.find(delimiter));

      //erase first token
      str.erase(0, str.find(delimiter) + 1);

      //get second token
      string action = str.substr(0, str.find(delimiter));

      //erase second token
      str.erase(0, str.find(delimiter) + 1);

      //find users in ignore events and add them from the vector
      if (action == "Ignore")
      {
        string sender = str.substr(0, str.find(delimiter));
        str.erase(0, str.find(delimiter) + 1);
        string offender = str.substr(0, str.find(delimiter));
        if (find(ignoreList.begin(), ignoreList.end(), offender) != ignoreList.end())
        {
          int len = ignoreList.size();
          for (int j = 0; j < len; j++)
          {
            if (offender == ignoreList.at(j))
            {
              ignoreList.erase(ignoreList.begin() + j);
              ignoreListLength.erase(ignoreListLength.begin() + j);
            }
          }
        }
        else if (getNick() == sender)
        {
          ignoreList.push_back(offender);
          ignoreListLength.push_back(offender.length());
        }
      }
      else if (action == "DeleteRoom")
      {
        string room = str.substr(0, str.find(delimiter));
        int len = roomList.size();
        for (int j = 0; j < len; j++)
        {
          if (room == roomList.at(j))
          {
            roomList.erase(roomList.begin() + j);
            roomListLength.erase(roomListLength.begin() + j);
          }
        }
      }
      else if (action == "CreateRoom")
      {
        string room = str.substr(0, str.find(delimiter));
        int len = roomList.size();
        int found = 0;

        for (int j = 0; j < len; j++)
        {
          if (room == roomList.at(j))
          {
            found = 1;
          }
        }
        if (found == 0)
        {
          int roomLen = room.length();
          roomList.push_back(room);
          roomListLength.push_back(roomLen);
        }
      }
      //find users in connect events and add them to the vector
      else if (action == "Connected")
      {
        string person = str.substr(0, str.find(delimiter));
        int len = memberList.size();
        int found = 0;

        for (int j = 0; j < len; j++)
        {
          if (person == memberList.at(j))
          {
            found = 1;
          }
        }
        if (found == 0)
        {
          int personLen = person.length();
          memberList.push_back(person);
          memberListLength.push_back(personLen);
        }
      }
      //find users in disconnect events and remove them from the vector
      else if (action == "Disconnected")
      {
        string person = str.substr(0, str.find(delimiter));
        int len = memberList.size();
        for (int j = 0; j < len; j++)
        {
          if (person == memberList.at(j))
          {
            memberList.erase(memberList.begin() + j);
            memberListLength.erase(memberListLength.begin() + j);
          }
        }
      }
      prompt();
      }

  }
  //wipes entire chat middle section. leaving the header and footer alone.
  void clearChat()
  {
      row = maxrow-3;
      col = 1;
      for (int r = 2;r<maxrow-2;r++)
      {
          for (int c = 0;c<maxcol;c++)
          {
              mvprintw(r, c, " ");
          }
      }
  }
  //iterate through vector "memberList"
  //display vector data to user's screen
  string showMemberList()
  {
    fillVectors();
    clearChat();

      int len = memberList.size();
      mvprintw(maxrow-(4+len),0, "**%i member(s) online**", len);
      
      for (int j = 0; j<len;j++)
      {
        
        int n = memberListLength.at(j);
        char char_array[n + 1];
        string str(memberList.at(j));
        str.erase(n, str.length());
        strcpy(char_array, str.c_str());
        int printOnRow = maxrow-(4+j);
        
        mvprintw(printOnRow,0, "[%s]", char_array);
      }
    prompt();
    //read user input
    char str[100];
    getstr(str);
    return str;
  }
  //iterate through vector "ignoreList"
  //display vector data to user's screen
  string showIgnoreList()
  {
    fillVectors();
    clearChat();

      int len = ignoreList.size();
      mvprintw(maxrow-(4+len),0, "**IGNORE LIST**");
      
      for (int j = 0; j<len;j++)
      {
        int n = ignoreListLength.at(j);
        char char_array[n + 1];
        string str(ignoreList.at(j));
        str.erase(n, str.length());
        strcpy(char_array, str.c_str());
        int printOnRow = maxrow-(4+j);
        mvprintw(printOnRow,0, "[%s]", char_array);
      }
    prompt();
    //read user input
    char str[100];
    getstr(str);
    return str;
  }
  string showErrorMsg(string msg)
  {
    clearChat();
    int n = msg.length();
    char char_array[n + 1];
    strcpy(char_array, msg.c_str());

    mvprintw(maxrow-3, 1, "**%s**", char_array);
    
    prompt();
    //read user input
    char str[100];
    getstr(str);
    return str;
  }
  //display full list of commands to user
  string showHelpMenu()
  {
    //build array to display
    clearChat();
    int size = 16;
    string help[size];
 
    help[15] = "/quit - exit SuperChat";
    help[14] = "/time - turn off timestamps";
    help[13] = "/memberlist - show all members online";
    help[12] = "/roomlist - show all members online";
    help[11] = "/join <room> - join an existing chat room";
    help[10] = "/leave - switch to lobby";
    help[9] = "/create <room> - creates a new chat room";
    help[8] = "/delete <room> - creates a new chat room";
    help[7] = "/private <int> <message> - sends a private message to digit";
    help[6] = "/setprivate <int> - set digit for recieving private messages";
    help[5] = "/rename <name> - changes your account name";
    help[4] = "/ignore <user> - ignore another user";
    help[3] = "/ignorelist - show your ignored users";
    help[2] = "/debug - show raw event log (admin only)";
    help[1] = "/transfer <path> <user> - transfer file to another user";
    help[0] = "**Enter any key to close help menu**";
    for (int i = 0 ;i<size;i++)
    {
        int n = help[i].length();
        char char_array[n + 1];
        strcpy(char_array, help[i].c_str());
        mvprintw(maxrow-(3+i),0, char_array);
    }
    prompt();
    //read user input
    char str[100];
    getstr(str);
    return str;

  }
  string showRoomList()
  {
    fillVectors();
    clearChat();

    int len = roomList.size();
    mvprintw(maxrow - (4 + len), 0, "**ROOM LIST**");

    for (int j = 0; j < len; j++)
    {
      int n = roomListLength.at(j);
      char char_array[n + 1];
      string str(roomList.at(j));
      str.erase(n, str.length());
      strcpy(char_array, str.c_str());
      int printOnRow = maxrow - (4 + j);
      mvprintw(printOnRow, 0, "[%s]", char_array);
    }
    prompt();
    //read user input
    char str[100];
    getstr(str);
    return str;
  }
  //function redraws the header (clock, chat room name, etc) and the footer (the row of "=")
  void showHeaderFooter()
  {
      //refresh X,Y
      getScreen();

      //clear top info panel (clock, room name, and help info)
      for (int i = 0;i<maxcol;i++)
      {
          mvprintw(0, i, " ");
      }

      //convert to char[]
      string str(getRoom());
      int n = str.length();
      char room_array[n + 1];
      strcpy(room_array, str.c_str());

      //redraw top info panel (clock, room name, and help info)
      string str2(getTime());
      int n2 = str2.length();
      char time_array[n2 + 1];
      strcpy(time_array, str2.c_str());

      mvprintw(0, 0, "%s", time_array);
      mvprintw(0, 10, "%s", room_array);
      mvprintw(0, maxcol-20, "For help: type /help");

     //draw top line
      for (int i = 0;i<maxcol;i++)
      {
          mvprintw(1, i, "=");
      }
          //draw bottom line
      for (int i = 0;i<maxcol;i++)
      {
          mvprintw(maxrow-2, i, "=");
      }
  }
  //convert string to char[] then send it off to the server
  void sendEvent(string event)
  {
      chat_message msg;
      int n = event.length();
      char char_array[n + 1];
      strcpy(char_array, event.c_str());
      msg.body_length(strlen(char_array));
      std::memcpy(msg.body(), char_array, msg.body_length());
      msg.encode_header();
      write(msg);
  }
private:
  //asio chat example that initlizes the connection to the server
  void do_connect(const tcp::resolver::results_type& endpoints)
  {
    //on connect, set user default room to Lobby
    changeRoom("Lobby");
    //get screen size and initialize counters
    getScreen();


    asio::async_connect(socket_, endpoints,
        [this](std::error_code ec, tcp::endpoint)
        {
          if (!ec)
          {
            do_read_header();
          }
        });
  }
  //asio chat example that reads the header and then calls do_read_body()
  void do_read_header()
  {
    asio::async_read(socket_,
        asio::buffer(read_msg_.data(), chat_message::header_length),
        [this](std::error_code ec, std::size_t /*length*/)
        {
          if (!ec && read_msg_.decode_header())
          {
            do_read_body();
          }
          else
          {
            socket_.close();
          }
        });
  }
  //primary method that iterates through all events from server
  //pushes events into the vector "sysLog"
  void do_read_body()
  {
      asio::async_read(socket_,
              asio::buffer(read_msg_.body(), read_msg_.body_length()),
              [this](std::error_code ec, std::size_t /*length*/)
              {
                if (!ec)
                {
                    //store event
                    //string str(read_msg_.body());
                    sysLog.push_back(read_msg_.body());
                    sysLogLength.push_back(read_msg_.body_length());

                    //recursion
                    do_read_header();
                }
                else
                {
                  socket_.close();
                }
              });
        }
  //asio chat example function that handles sending messages to the server via vector
  void do_write()
  {
    asio::async_write(socket_,
        asio::buffer(write_msgs_.front().data(),write_msgs_.front().length()),[this](std::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {
            write_msgs_.pop_front();
            if (!write_msgs_.empty())
            {
              do_write();
            }
          }
          else
          {
            socket_.close();
          }
        });

  }
//private variables for chat_client
private:
  asio::io_context& io_context_;
  tcp::socket socket_;
  string currentRoom;
  string nickName;
  int maxrow,maxcol;
  int row,col;
  int timeINT;
  string code;
  chat_message read_msg_;
  chat_message_queue write_msgs_;
  vector<string> sysLog;
  vector<int> sysLogLength;
  vector<string> ignoreList;
  vector<int> ignoreListLength;
  vector<string> memberList;
  vector<int> memberListLength;
  vector<string> roomList;
  vector<int> roomListLength;
};
static void handle_signal(int sig)
{

}
int main(int argc, char* argv[])
{
    try
    {
      

      //built in code written in asio example
      if (argc != 3)
      {
        std::cerr << "Usage: chat_client <host> <port>\n";
        return 1;
        }
        asio::io_context io_context;
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(argv[1], argv[2]);
        chat_client c(io_context, endpoints);
        std::thread t([&io_context](){ io_context.run(); });

        //signal builder#1
        struct sigaction act;
        memset(&act, '\0', sizeof(act));
        act.sa_handler = &handle_signal;
        if (sigaction(SIGINT, &act, NULL) < 0)
        {
          perror("sigaction: ");
          return 1;
        }

        //initlize ncurses 
        initscr();
        cbreak();
        raw();

        //enable keypad input
        keypad(stdscr, TRUE);

        //show user input
        echo();
        
        //generate member list and ignore list
        c.fillVectors();
        
        //slow down the loop so that the server can catch up
        sleep_for(nanoseconds(1000000));

        int loop = 1;
        //show login screen so user can input account name
        c.loginScreen(0);
        
        
          
        
        //main loop starts here
        
        while (loop == 1)
        {
          //slow down the loop so that the server can catch up
          sleep_for(nanoseconds(1000000));

          //redraw top and bottom panels and prompt
          c.clearChat();
          c.showHeaderFooter();
          c.readSystemLog();
          c.showHeaderFooter();
          c.prompt();

          //initialize input storage
          char str[100];

          //read user input
          getstr(str);

          //convert to string
          string line(str);

          //COMMANDS
          

          //display help menu
          if (line.find("/help") == 0)
          {
              line.assign(c.showHelpMenu());
          }
            //display list of ignored users
          if (line.find("/ignorelist") == 0)
          {
              line.assign(c.showIgnoreList());
          }
            //display list of ignored users
          if (line.find("/memberlist") == 0)
          {
              line.assign(c.showMemberList());
          }
            //display list of ignored users
          if (line.find("/debug") == 0)
          {
              line.assign(c.debug());
          }
          if (line.find("/roomlist") == 0)
          {
            line.assign(c.showRoomList());
          }
          //join a different existing room
          if (line.find("/join") == 0)
          {

            string newRoom = line.substr(line.find(" ") + 1, line.length());
            line.assign(c.checkRoomExists(newRoom));
          }
          //file transfer
          if (line.find("/transfer") == 0)
          {
              //chosen format
              string delimiter = " ";
              //erase command
              line.erase(0, line.find(delimiter)+1);
              //get first token
              string filepath = line.substr(0, line.find(delimiter));
              //erase first token
              line.erase(0, line.find(delimiter)+1);
              //get second token
              string user = line.substr(0, line.find(delimiter));

              fstream fp;
              fp.open (filepath);
              string content;
              string fsline;
              if (fp.is_open())
              {
                while ( getline (fp,fsline) )
                {
                  content = content + fsline;
                }
                fp.close();
              }
              string event(c.getTime() + ";Transfer;" + filepath + ";" + c.getNick() + ";" + user + ";" + content + ";");
              c.sendEvent(event);
          }
          //rename account name
          else if (line.find("/rename") == 0)
          {
              //TODO -  verify that name does not exist
              string newName = line.substr(line.find(" ")+1, line.length());
              c.setNick(newName);
          }

          
          
                    
          //leave current room. default lobby
          else if (line.find("/leave") == 0)
          {
              c.changeRoom("Lobby");
          }
          //ignore other user
          else if (line.find("/ignore") == 0)
          {
            //chosen format
              string delimiter = " ";
              //get first token
              string token1 = line.substr(0, line.find(delimiter));
              //erase first token
              line.erase(0, line.find(delimiter)+1);

              //get second token
              string token2 = line.substr(0, line.find(delimiter));

              if (line.find(delimiter) > 1)
              {
                  string event(c.getTime() + ";Ignore;" + c.getNick() + ";" + token2 + ";");
                  c.sendEvent(event);
              }
          }
          //send private message
          else if (line.find("/setprivate") == 0)
          {
              string digit2 =line.substr(line.find(" ")+1, line.length());
              c.setDigit(digit2);
          }
          else if (line.find("/private") == 0)
          {
              string privateMessage =line.substr(line.find(" ")+1, line.length());
              string MsgToSend =privateMessage.substr(privateMessage.find("")+2,privateMessage.length());
              string digit = privateMessage.substr(0,1);
              string event(c.getTime() + ";PrivateMessage;"+ c.getNick() + ";" + digit +";" + MsgToSend + ";");
              c.sendEvent(event);

          }
          //create new chat room
          else if (line.find("/create") == 0)
          {
            if (c.getRoomCount() >= 10)
            {
              line.assign(c.showErrorMsg("Max rooms reached"));
            }
            else
            {
              string newRoom = line.substr(line.find(" ") + 1, line.length());
              string event(c.getTime() + ";CreateRoom;" + newRoom + ";" + c.getNick());
              c.sendEvent(event);
            }
          }
          //delete existing chat room
          else if (line.find("/delete") == 0)
          {
              string newRoom = line.substr(line.find(" ")+1, line.length());
              string event(c.getTime() + ";DeleteRoom;" + newRoom + ";" + c.getNick());
              c.sendEvent(event);
          }
          //qduit the app
          else if (line.find("/quit") == 0)
          {
              string event(c.getTime() + ";Disconnected;" + c.getNick() + ";");
              c.sendEvent(event); //send disconnect msg to server
              loop = 0; // this makes it quit
          }
          //toggle timestamps on chat messages
          else if (line.find("/time") == 0)
          {
            if (c.getTimestamp() == 0)
            {
                c.setTimestamp(1);
            }
            else {  c.setTimestamp(0);}
          }
          //unknown command
          else if (line.find("/") == 0)
          {

          }
          //no message
          else if (line.length() == 0)
          {
           
          }
          //no valid command was found
          //assume that this is just a regular chat message to current room
          else
          {
              string event(c.getTime() + ";ChatMsg;" + c.getRoom() + ";" + c.getNick() + ": " + line);
              c.sendEvent(event);

          }

          str[0] = '\0';
        }

        //close chat client and join threads
        c.close();
        t.join();
        }
        catch (std::exception& e)
        {
                std::cerr << "Exception: " << e.what() << "\n";
        }
        //close ncurses window
        endwin();
        return 0;
}

