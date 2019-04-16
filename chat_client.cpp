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




using asio::ip::tcp;
using namespace std;

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
  //set local chat_client variable to new account name
    bool changeNick(string nickSender)
  {
      //alan - loop through memberlist vector
      //if match found - error code and reshow screen
      //if no match found - nickName.assign(nickSender);
     
      int arraySize=memberList.size();
      for(int i=0;i<arraySize;i++)
      {
	      if(nickSender == memberList.at(i))
	      {
		    return true;
	      }
      }
    nickName.assign(nickSender);
    //memberList.push_back(nickSender);
    return false;
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
          //get current screen size
      getmaxyx(stdscr,maxrow,maxcol);
          //these two variables are starting points for where to print chat messages. (typically top left corner)

  }
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

  void prvtMsgOLD(string sender, string digitMsg, string msg)
  {
      //create private int variable in chat_client called "digit"
      //if (c.digit == digitMsg)
      //if true, then print msg



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
    //create private int variable in chat_client called "digit"
    if (code == digitMsg)
    {
      mvprintw(row,1,"[%s]",msg);

                

    }
    else
    {
      //mvprintw(row,1,"message doesn't exists!");

    }

}


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
    checkIgnores();
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

            //RICARDO
            //int found = 0;
            //CHatMsg;Lobby;Nick;Msgy
            //for loop through roomList vector
            //if roomList[i] == Lobby. if found set found = 1


            //if found = 0;
            //then
            //pop messages from deleted rooms from vector sysLog

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
                //mvprintw(maxrow/2,maxcol/2,"%s",userInList);
                //mvprintw(maxrow/2,maxcol/2,"user ignored");
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
            //string event(c.getTime() + ";Transfer;" + filepath + ";" + c.getNick() + ";" + user + ";" + content + ";");
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

          /*
          string sender = str.substr(0, str.find(delimiter));
          str.erase(0, str.find(delimiter)+1);
          string offender = str.substr(0, str.find(delimiter));
          int test = str.find(delimiter);
          //str.erase(0, str.find(delimiter)+1);

          if (find(ignoreList.begin(), ignoreList.end(),offender)!=ignoreList.end())
          {

          }
          else if (getNick() == sender)
          {
            ignoreList.push_back(offender);
            //ignoreListLength.push_back(offender.length());
            ignoreListLength.push_back(test);
           


            //ignoreList.push_back(offender);
            //ignoreListLength.push_back(offender.length());

          }
          
          */
        }
        else if (action == "Connected")
        {
            //add to private vector memberlist inside chat_client
            //vector.push_back("");
            memberList.push_back(getNick());


        }
        else if (action == "Disconnected")
        {
          string person = str.substr(0, str.find(delimiter));
          str.erase(0, str.find(delimiter)+1);

          int len = memberList.size();

            
            for (int j = 0; j<len;j++)
            {
              if (person == memberList.at(j))
              {
                  memberList.erase (memberList.begin() + j);
              }
            }




        }
        else if (action == "DeleteRoom")
        {
            //need to make a room vector
            //pop for remove



        }
        else if (action == "CreateRoom")
        {
            //need to make a room vector
            //push for create


        }
        else if (action == "PrivateMessage")
        {
            //string event("PrivateMessage;"+ c.getNick() + ";" + digit +";" + MsgToSend);
            //string event("PrivateMessage;"+ c.getNick() + ";" + digit +";" + MsgToSend);


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
            //str.erase(0, str.find(delimiter)+1);

            //prvtMsg(sender, digit, msg);
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


            //int n = sysLogLength[i] - action.length() - sender.length() - digit.length();
                //char char_array[n + 1];
                //strcpy(char_array, str.c_str());
            ////mvprintw(row,1,"%s", char_array);
            //mvprintw(row,1,"test");
            //row = row -1;




        }
      //redraw the prompt at bottom left (ex- "Nick: ")
      prompt();
      }
  }
  void checkIgnores()
  {
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
      if (action == "Ignore")
        {
          string sender = str.substr(0, str.find(delimiter));
          str.erase(0, str.find(delimiter)+1);
          //int offenderSize = str.find(delimiter);
          //string offender = str.substr(0, offenderSize);
          string offender = str.substr(0, str.find(delimiter));

          
          

          if (find(ignoreList.begin(), ignoreList.end(),offender)!=ignoreList.end())
          {

          }
          else if (getNick() == sender)
          {
            //offenderSize is wrong
            ignoreList.push_back(offender);
            ignoreListLength.push_back(offender.length());
          }
          
        }






      prompt();
      }

  }
  void clearChat()
  {
      //wipes entire chat middle section. leaving the header and footer alone.
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
  string showMemberList()
  {

          clearChat();

            int len = memberList.size();
            mvprintw(maxrow-(4+len),0, "**MEMBER LIST**");
            
            for (int j = 0; j<len;j++)
            {
              
              //string newStr = ignoreList.at(j).substr(0, n);
              int n = 4;
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
  string showIgnoreList()
  {
    checkIgnores();
          clearChat();

            int len = ignoreList.size();
            mvprintw(maxrow-(4+len),0, "**IGNORE LIST**");
            
            for (int j = 0; j<len;j++)
            {
              int n = ignoreListLength.at(j);
              //string newStr = ignoreList.at(j).substr(0, n);
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
  string showHelpMenu()
  {
          clearChat();
          int size = 14;
          string help[size];
          help[13] = "**HELP MENU**";
          help[12] = "/quit - exit SuperChat";
          help[11] = "/time - turn off timestamps";
          help[10] = "/members <room> - show members in an existing chat room";
          help[9] = "/join <room> - join an existing chat room";
          help[8] = "/leave - switch to lobby";
          help[7] = "/create <room> - creates a new chat room";
          help[6] = "/delete <room> - creates a new chat room";
          help[5] = "/private <int> <message> - sends a private message";
          help[4] = "/rename <name> - changes your account name";
          help[3] = "/ignore <user> - ignore another user";
          help[2] = "/ignorelist - show your ignored users";
          help[1] = "/transfer <path> <user> - transfer file to another user";
          help[0] = "**Enter any key to close menu**";
          for (int i = 0 ;i<size;i++)
          {
              int n = help[i].length();
              char char_array[n + 1];
              strcpy(char_array, help[i].c_str());
              mvprintw(maxrow-(4+i),0, char_array);
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

  void sendEvent(string event)
  {
      //build new chat_message
      chat_message msg;

      //build string according to format standards
      //Time;Action;Room;Actor;StringVal
      //string concat(c.getTime() + ";ChatMsg;" + c.getRoom() + ";" + c.getNick() + ": " + line);

      //convert string to char[] then send it off to the server
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
  //primary method that iterates through all msgs in queue.
  //Method prints messages and calls functions to process server info for the client
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
  vector<string> memberListLength;
  
  //benjamin - private variable INT - code/digit
  //sunil - private variable string<vector> - memberlist vector
  //ricardo - private variable string<vector> - roomList vector

};

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

        //ncurses display initializations
        char welcome1[]="Welcome to Super Chat";
        char welcome2[]="Enter user name: ";
        int row,col;
        int loop = 1;
        //initlize ncurses window
        initscr();
        cbreak();
        raw();
        //enable keypad input
        keypad(stdscr, TRUE);
        //show user input
        echo();
        //get screen size
        getmaxyx(stdscr,row,col);
        //print welcome messages to screen
        mvprintw((row/2)-3,(col-strlen(welcome1))/2,"%s",welcome1);
        mvprintw(row/2,(col-strlen(welcome2))/2,"%s",welcome2);

        //initilize place to store user's account name
        char nameArray[10];
        //read account name from user input. cap at 10 characters
        getnstr(nameArray,10);

        //convert to string
        string nameStr(nameArray);

        
        






/*
bool dupe = false;
  //test case
        //c.memberList.push_back("nick"); 

        //set user's name
        //impliment dupe checking in c.changeNick
        dupe = c.changeNick(nameStr);

	while(dupe)
        {
                char copy[]="Name already taken";
                //clear welcome messages away
                c.clearRow((row/2)-3);
		c.clearRow(row/2);
                mvprintw((row/2)-3,(col-strlen(copy))/2,"%s",copy);
                mvprintw(row/2,(col-strlen(welcome2))/2,"%s",welcome2);
                getnstr(nameArray,10);
                //convert to string
                string nameStr(nameArray);
                dupe = c.changeNick(nameStr);
        }
























        //set user's name
        //impliment dupe checking in c.changeNick
        c.changeNick(nameStr);
        c.setTimestamp(1);
        string msg(c.getTime() + ";Connected;" + c.getNick());
        c.sendEvent(msg);

        //clear welcome messages away
        c.clearRow((row/2)-3);
        c.clearRow(row/2);

        //draw top info panel and bottom prompt panel
        c.showHeaderFooter();

      

        //draw initial prompt at bottom left
        c.prompt();

        //main loop starts here
        while (loop == 1)
        {
          //slow down the loop so that the server can catch up
          using namespace std::this_thread; //need for sleep_for
          using namespace std::chrono; //need for nanoseconds
          sleep_for(nanoseconds(10000));


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
            //file transfer
            if (line.find("/transfer") == 0)
            {
                //chosen format
                string delimiter = " ";

                line.erase(0, line.find(delimiter)+1);
                //get first token
                string filepath = line.substr(0, line.find(delimiter));
                //erase first token
                line.erase(0, line.find(delimiter)+1);

                //get second token
                string user = line.substr(0, line.find(delimiter));
                //erase second token
                //line.erase(0, line.find(delimiter)+1);

                //get third token
                //string user = line.substr(0, line.find(delimiter));
                //erase third token
                //line.erase(0, line.find(delimiter)+1);


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
                c.changeNick(newName);
            }

            else if (line.find("/listrooms") == 0)
            {
                //loop through c.roomList() and print all rooms
            }
            //join a different existing room
            else if (line.find("/join") == 0)
            {
                //TODO -  verify that room exists
                string newRoom = line.substr(line.find(" ")+1, line.length());
                c.changeRoom(newRoom);
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
                //in chat_client - write a get/set methods for that local private variable
                //put here - c.setDigit(5);
                string digit2 =line.substr(line.find(" ")+1, line.length());
                c.setDigit(digit2);


            }
            else if (line.find("/private") == 0)
            {
                //int CodeNum= 0;
                //string privateMessage =line.substr(line.find(" ")+1, line.length());
                //string MsgToSend =privateMessage.substr(privateMessage.find("")+2,privateMessage.length());
                //string digit = privateMessage.substr(0,1);
                //stringstream ToInt(digit);
                //ToInt >> CodeNum;

                //sprintf(str, "%02d:%02d", hour, min)



                //string event("PrivateMessage;"+ c.getNick() + ";" + digit +";" + MsgToSend);
                //c.sendEvent(event);


                //int CodeNum= 0;
                string privateMessage =line.substr(line.find(" ")+1, line.length());
                string MsgToSend =privateMessage.substr(privateMessage.find("")+2,privateMessage.length());
                string digit = privateMessage.substr(0,1);

                //set private here  c.setDigit()

                string event(c.getTime() + ";PrivateMessage;"+ c.getNick() + ";" + digit +";" + MsgToSend + ";");
                c.sendEvent(event);

            }
            //create new chat room
            else if (line.find("/create") == 0)
            {
                //TODO -  verify that room does not exist


                //if c.roomList.length() < 10
                //then continue
                //else warn the users theres max limit of 10 chat rooms
                string newRoom = line.substr(line.find(" ")+1, line.length());
                string event(c.getTime() + ";CreatedRoom;" + newRoom + ";" + c.getNick());
                c.sendEvent(event);

            }
            //delete existing chat room
            else if (line.find("/delete") == 0)
            {
                //TODO -  verify that room exists
                //should we check to see if there are any users in the chat room first
                string newRoom = line.substr(line.find(" ")+1, line.length());
                string event(c.getTime() + ";DeleteRoom;" + newRoom + ";" + c.getNick());
                c.sendEvent(event);
            }
            //display current members in current room
            else if (line.find("/members") == 0)
            {

            }
            //quit the app
            else if (line.find("/quit") == 0)
            {
                string event(c.getTime() + ";Disconnected;" + c.getNick());
                c.sendEvent(event); //send disconnect msg to server
                loop = 0; // this makes it quit

            }
            //turn off timestamp
            else if (line.find("/time") == 0)
            {
              if (c.getTimestamp() == 0)
              {
                  c.setTimestamp(1);
              }
              else {  c.setTimestamp(0);}
            }
         
            //bad command
            else if (line.find("/") == 0)
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

        //sunil - if user typed /quit then send disconnect msg to server
        //sunil - also check for ctl+c using signal handlers
        return 0;
}


