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
          // converting string to char[] then printing to screen
      string concat(getNick());
      int n = concat.length();
      char char_array[n + 1];
      strcpy(char_array, concat.c_str());
      clearRow(maxrow-1);
      mvprintw(maxrow-1,1," %s: ",char_array);
  }
  //set local chat_client variable to new room name
  void changeRoom(string roomName)
  {
    currentRoom = roomName;
  }
  //return local chat_client variable for current room name
  string getRoom()
  {
    return currentRoom;
  }
  //set local chat_client variable to new account name
  void changeNick(string nickSender)
  {
    nickName.assign(nickSender);
  }
  //return local chat_client variable for account name
  string getNick()
  {
    return nickName;
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
      row = 2;
      col = 1;
  }
  //clear ncruses gui at a certain row
  void clearRow(int row)
  {
      for (int i = 0;i<maxcol;i++)
      {
          mvprintw(row, i, " ");
      }
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
                    //convert single msg to char[]
                    string concat(read_msg_.body());
                    int n = concat.length();
                    char char_array[n + 1];
                    strcpy(char_array, concat.c_str());

                    //TODO - parse concat (string) or char_array (char[])
                    //use standards that we defined

                    //if it is just a regular chat message, then use this to print to screen
                    mvprintw(row,1,"%s",char_array);
                    //increment down to next line
                    row = row + 1;
                    //redraw the prompt at bottom left (ex- "Nick: ")
                    prompt();
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
  chat_message read_msg_;
  chat_message_queue write_msgs_;
};
//function redraws the header (clock, chat room name, etc) and the footer (the row of "=")
void showHeaderFooter(int maxrow, int maxcol)
{
        //clear top info panel (clock, room name, and help info)
    for (int i = 0;i<maxcol;i++)
    {
        mvprintw(0, i, " ");
    }
    //redraw top info panel (clock, room name, and help info)
    mvprintw(0, 0, "9:48PM");
    mvprintw(0, 10, "Lobby");
    mvprintw(0, maxcol-20, "For help: type /help");

   //draw top line
    for (int i = 0;i<maxcol;i++)
    {
        mvprintw(1, i, "=");
    }
        //draw bottom line
    for (int i = 0;i<maxcol;i++)
    {
        mvprintw(maxrow-3, i, "=");
    }
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

        //built-in code from asio example
        auto endpoints = resolver.resolve(argv[1], argv[2]);
        chat_client c(io_context, endpoints);
        //Sunil - call your custom function to send a msg to server that a new person connected

        //set user's name
        //impliment dupe checking in c.changeNick
        c.changeNick(nameStr);

        //clear welcome messages away
        c.clearRow((row/2)-3);
        c.clearRow(row/2);

        //draw top info panel and bottom prompt panel
        showHeaderFooter(row, col);

        //built-in code from asio example
        std::thread t([&io_context](){ io_context.run(); });

        //draw initial prompt at bottom left
        c.prompt();

        //main loop starts here
        while (loop == 1)
        {
            char str[80];
            //redraw prompt at bottom left
            c.prompt();
            //read user input
            getstr(str);
            //convert to string
            string line(str);

            //COMMANDS
            //change room
            if (line.find("/room ") == 0)
            {
                string roomName = line.substr(0, line.find(" "));
                c.changeRoom(roomName);
            }
            //display help menu
            else if (line.find("/help ") == 0)
            {

            }
            //ignore other user
            else if (line.find("/ignore ") == 0)
            {

            }
            //send private message
            else if (line.find("/private ") == 0)
            {

            }
            //create new chat room
            else if (line.find("/create ") == 0)
            {

            }
            //delete existing chat room
            else if (line.find("/delete ") == 0)
            {

            }
            //no valid command was found
            //assume that this is just a regular chat message to current room
            else
            {
                //convert char[] to string
                int len = sizeof(str);
                string lenStr = to_string(len);

                //build new chat_message
                chat_message msg;

                //build string according to format standards
                //Action;Location;Actor;StringVal
                //in this case it is ChatMsg;RoomName;Nickname;Message
                string concat("ChatMsg;" + c.getRoom() + ";" + c.getNick() + ": " + str);

                //convert string to char[] then send it off to the server
                int n = concat.length();
                char char_array[n + 1];
                strcpy(char_array, concat.c_str());
                msg.body_length(strlen(char_array));
                std::memcpy(msg.body(), char_array, msg.body_length());
                msg.encode_header();
                c.write(msg);
            }
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
        ///sunil - also check for ctl+c using signal handlers
        return 0;
}



