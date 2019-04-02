/*
make -lncurses
./chat_server 9000
./chat_client 127.0.0.1 9000
g++ seproject.cpp -lncurses
g++ chat_client.cpp -lncurses
./a.out
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

using asio::ip::tcp;
using namespace std;
typedef std::deque<chat_message> chat_message_queue;
char msgs[50][100];
char help[50][100];
int chatSize = 14;
char nameArray[80];
char roomName[80];

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
  void changeRoom(std::string roomName)
  {
    currentRoom = roomName;

  }
  string getRoom()
  {
    return currentRoom;

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

private:
  void do_connect(const tcp::resolver::results_type& endpoints)
  {
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
                  std::cout.write(read_msg_.body(), read_msg_.body_length());
                  std::cout << "\n";
                  //mvprintw(0,0,"%s",read_msg_.body());
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
  asio::io_context& io_context_;
  tcp::socket socket_;
  std::string currentRoom;
  chat_message read_msg_;
  chat_message_queue write_msgs_;
};


int main(int argc, char* argv[])
{
  try
  {
	//ALL UNCHANGED ASIO CODE
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


	//send new client to lobby
	c.changeRoom("Lobby");
	//initialize variables
    char welcome1[]="Welcome to Super Chat";
    char welcome2[]="Enter user name: ";
    int row,col;
    int loop = 1;
    int position = 2;
	
	//initialize NCURSES
    initscr();
	//get size of current window
    getmaxyx(stdscr,row,col);
	//print welcome1 to screen
    mvprintw((row/2)-3,(col-strlen(welcome1))/2,"%s",welcome1);

	//initialize char[]
	char line[chat_message::max_body_length + 1];
	
	//start infinite loop
	while (loop == 1)
	{
		//read current line from user
		getstr(line);
		//convert to string
		string str(line);

	//compare statements to read commands
	if (str == "/room qa")
	{
		c.changeRoom("qa");
	}
	//if no command found, then just read as a new chat message
	else
	{
		//mostly ASIO code
		chat_message msg;
		//add room name to message
		string concat(line + c.getRoom());
		int n = concat.length();
		char char_array[n + 1];
		strcpy(char_array, concat.c_str());
		msg.body_length(strlen(char_array));
		std::memcpy(msg.body(), char_array, msg.body_length());
		msg.encode_header();
		c.write(msg);
	}
	}

	c.close();
	t.join();
	}
	catch (std::exception& e)
	{
	std::cerr << "Exception: " << e.what() << "\n";
	}
	endwin();
	return 0;
    }