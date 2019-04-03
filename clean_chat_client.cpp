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

char nameArray[15];




class chat_client
{
public:
  chat_client(asio::io_context& io_context,
      const tcp::resolver::results_type& endpoints)
    : io_context_(io_context),
      socket_(io_context)
  {
    do_connect(endpoints);
  }
  void changeRoom(string roomName)
  {
    currentRoom = roomName;
  }
  string getRoom()
  {
    return currentRoom;
  }
  void changeNick(string nickSender)
  {
    nickName = nickSender;
  }
  string getNick()
  {
    return nickName;
  }
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

  void close()
  {
    asio::post(io_context_, [this]() { socket_.close(); });
  }
  //this function will get the size of the current window and put it into maxrow and maxcol. also, sets row,col for iteration
  void getScreen()
  {
      getmaxyx(stdscr,maxrow,maxcol);
      row = 2;
      col = 1;
      //row = ceil(maxrow/2);
      //col = ceil(maxcol/2);
  }
  void clearRow(int row)
  {
      for (int i = 0;i<maxcol;i++)
      {
          mvprintw(row, i, " ");
      }
  }
private:
  void do_connect(const tcp::resolver::results_type& endpoints)
  {
	  //set default room on connect and call getScreen
    changeRoom("Lobby");
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

  void do_read_body()
  {

      asio::async_read(socket_,
              asio::buffer(read_msg_.body(), read_msg_.body_length()),
              [this](std::error_code ec, std::size_t /*length*/)
              {
                if (!ec)
                {
				  //convert read_msg_body to a string called "str"
                  string str(read_msg_.body());
				  //clear current "row" before printing it to screen
                  clearRow(row);
				  //print one message to screen at "row"
                  mvprintw(row,1,"%s",read_msg_.body());
				  //clear space after the message
                  for (int i = read_msg_.body_length()+1; i <= maxcol; i++)
                  {
                          mvprintw(row,i," ");

                  }
				  //increment to next line
                  row = row + 1;
				  //start over again
                  do_read_header();
                }
                else
                {
                  socket_.close();
                }
              });
        }

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

private:
  //built-in variables
  asio::io_context& io_context_;
  tcp::socket socket_;
  chat_message read_msg_;
  chat_message_queue write_msgs_;
  //custom variables written by Nick
  string currentRoom;
  string nickName;
  int maxrow,maxcol;
  int row,col;
};
//function redraws the header (clock, chat room name, etc) and the footer (the row of "=")
void showHeaderFooter(int row, int col)
{
    for (int i = 0;i<col;i++)
    {
        mvprintw(0, i, " ");
    }


    mvprintw(0, 0, "9:48PM");
    mvprintw(0, 10, "Lobby");
    mvprintw(0, col-20, "For help: type /help");




    for (int i = 0;i<col;i++)
    {
        mvprintw(1, i, "=");
    }
    for (int i = 0;i<col;i++)
    {
        mvprintw(row-3, i, "=");
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

	//custom code 
	//ncurses display initializations
    char welcome1[]="Welcome to Super Chat";
    char welcome2[]="Enter user name: ";
    int row,col;
    int loop = 1;
    initscr();
    nocbreak();
    raw();
    keypad(stdscr, TRUE);
    echo();
    getmaxyx(stdscr,row,col);
	//print welcome screen
    mvprintw((row/2)-3,(col-strlen(welcome1))/2,"%s",welcome1);
    mvprintw(row/2,(col-strlen(welcome2))/2,"%s",welcome2);
	//get account namee
    getnstr(nameArray,15);
	//convert char[] to string
    string nameStr(nameArray);
	
	
	
	//built-in code from asio example
    auto endpoints = resolver.resolve(argv[1], argv[2]);
    chat_client c(io_context, endpoints);
	
	//custom functions in chat_client that are explained above
    c.changeNick(nameStr);
    c.clearRow((row/2)-3);
    c.clearRow(row/2);
	
	//local function. explained above
    showHeaderFooter(row, col);
	
	//built-in code from asio example
    std::thread t([&io_context](){ io_context.run(); });

	
	
		//main loop
        while (loop == 1)
        {
			//clear row and show input line
            char str[80];
            c.clearRow(row-1);
            mvprintw(row - 1, 1, "%s:", c.getNick());
			//read user entered string
            getstr(str);
			//convert char[] to string
            string line(str);
			
			//start detecting commands
			//only one command written so far (change room)
          if (line.find("/room ") == 0)
          {
            string roomName = line.substr(0, line.find(" "));
            c.changeRoom(roomName);
          }
		  //catch-all for anything that isnt a command
		  //will be treated like a chat message
          else
          {
			  //initialize
              chat_message msg;
			  //concat room name, account name, and message
              string concat(c.getRoom() + ";" + c.getNick() + ": " + line);
			  //get final total length
              int n = concat.length();
			  //initlize
              char char_array[n + 1];
			  //copy final value into char[]
              strcpy(char_array, concat.c_str());
			  //set chat_message length
              msg.body_length(strlen(char_array));
			  //copy char[] into chat_message
              std::memcpy(msg.body(), char_array, msg.body_length());
			  //chat_message encode
              msg.encode_header();
			  //chat_client sends message to server?
              c.write(msg);
          }
        }
		//close client
        c.close();
		//join threads?
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


