//
// chat_client.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

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
        asio::buffer(write_msgs_.front().data(),
          write_msgs_.front().length()),
        [this](std::error_code ec, std::size_t /*length*/)
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
char msgs[50][100];
int chatSize = 14;
char nameArray[80];
void initMsgs()
{

    strcpy(msgs[0],"Mike: C provides constructs that map efficiently");
    strcpy(msgs[1],"Tahmid Stubbs: i agree");
    strcpy(msgs[2],"Aniela Branch: C was originally developed by Dennis Ritchie");
    strcpy(msgs[3],"Iain Britton: yeah");
    strcpy(msgs[4],"Iain Britton: A state diagram is a type of diagram used");
    strcpy(msgs[5],"Iain Britton: computer science and related fields to describe");
    strcpy(msgs[6],"Tahmid Stubbs: behavior of systems. State diagrams require");
    strcpy(msgs[7],"Harvir Reyna: the system described is composed of");
    strcpy(msgs[8],"Aniela Branch: diagrams are used to give an abstract descript");
    strcpy(msgs[9],"Harvir Reyna: this behavior is analyzed and represented");
    strcpy(msgs[10],"Rees Wolfe: it can occur in one or more possible sts");
    strcpy(msgs[11],"Mike: C provides constructs that map efficiently");
    strcpy(msgs[12],"Tahmid Stubbs: Despite its low-level capabilities, the language");
    strcpy(msgs[13],"Aniela Branch: C was originally developed by Dennis Ritchie");
    strcpy(msgs[14],"Iain Britton: yeah");

}
void clearRow(int row, int maxcol)
{
    for (int i = 0;i<maxcol;i++)
    {
        mvprintw(row, i, " ");
    }
}
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
void showContent(char content[][100])
{

    int position = 5;
    for (int i = chatSize;i>=0;i--)
    {
        mvprintw(LINES - position, 0, "                                                               ");
        //mvprintw(LINES - position, 0, "%s", msgs[i]);
        mvprintw(LINES - position, 0, "%s", content[i]);
        position = position + 1;
    }

}
void showMemList(int row, int col)
{
    mvprintw(row/3, col-13, "Member List");
    mvprintw((row/3)+1, col-13, "------------");
    mvprintw((row/3)+2, col-13, "Aniela Branch");
    mvprintw((row/3)+3, col-13, "Rees Wolfe");
    mvprintw((row/3)+4, col-13, "Samina Kouma");
    mvprintw((row/3)+5, col-13, "Harvir Reyna");
    mvprintw((row/3)+6, col-13, "Iain Britton");
    mvprintw((row/3)+7, col-13, "Tahmid Stubbs");
    //mvprintw((row/3)+8, col-13, nameArray);
    for (int i = (row/3)+1;i<row-3;i++)
    {
        mvprintw(i, col-15, "|");
    }

}
void clearChat(int maxrow, int maxcol)
{
    //int position = 5;
    for (int i = 2;i<maxrow-4;i++)
    {
        for (int j = 0;j<maxcol-20;j++)
        {
            mvprintw(i, j, " ");
        }
        //mvprintw(LINES - position, 0, "HELP  MENU");
        //position = position + 1;
    }
}
void addMsg(char *user, char *line)
{

    for (int i = 0;i<50;i++)
    {

        msgs[i][0] = '\0';
        strcpy(msgs[i], msgs[i+1]);

    }

    msgs[chatSize][0] = '\0';
    strcpy(msgs[chatSize], user);
    strcat(msgs[chatSize], ": ");
    strcat(msgs[chatSize], line);

}
void joinChatRoom()
{
    //MULTIPLE ROOMS
    /*
    It's a network programming example not a chat server.

    The example chat server operates on a socket and doesn't have any
    logic regarding virtual chat rooms. One chat is bound to a single socket
    which is blocked for other instances of the chat server.

    If you really want to operate multiple chat rooms with this chat server
    example you can bind individual instances of the chat server to other ports.
    An other way would be multiplexing the connections on the same socket using
    select command or other multiplexing commands.

    *********
    are we then creating multithreading?
    based off creating multiple sessions or server classes?

    multiple ports or multiplexing
    *********
     * */

}
void register_participant()
{
    //how to send nickname or GUID to server
    //created a "name" variable in chat_participant
    //need the client to tell the server to asssociate its
    //i assume the server stores the master list of all partipants

    //dupes
    //private msgs (edit lines of code in chat_room::deliver)
    
    //message has multiple fields
    //destin for specific chat room
}
int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cerr << "Usage: chat_client <host> <port>\n";
      return 1;
    }

    asio::io_context io_context;

    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(argv[1], argv[2]);
    chat_client c(io_context, endpoints);
    c.changeRoom("Lobby");
    std::thread t([&io_context](){ io_context.run(); });



 /*



    getmaxyx(stdscr,row,col);
    mvprintw((row/2)-3,(col-strlen(welcome1))/2,"%s",welcome1);
    mvprintw(row/2,(col-strlen(welcome2))/2,"%s",welcome2);
    getstr(nameArray);
    clearRow((row/2)-3,col);
    clearRow(row/2, col);


    int position = 2;
    //showContent(msgs);
    showHeaderFooter(row, col);
    showMemList(row, col);
*/
/*
//THIS IS THE START OF NICKS LOOP
    int loop = 1;
    char msgArray[80];
    char welcome1[]="Welcome to Super Chat";
    char welcome2[]="Enter user name: ";
    int row,col;
    initscr();
    while (loop == 1)
    {

      mvprintw(LINES - 2, 0, "%s:                                                                                                ",nameArray);
      mvprintw(LINES - 2, 0, "%s:  ", nameArray);
      getstr(msgArray);
      mvprintw(LINES - 5, 0, "                                                                                                ");

      showHeaderFooter(row, col);
      clearChat(row, col);
      showMemList(row, col);

      addMsg(nameArray, msgArray);
      showContent(msgs);

      //chat_message msg;
      //msg.body_length(std::strlen(msgArray));
      //std::memcpy(msg.body(), msgArray, msg.body_length());
      //msg.encode_header();
      //c.write(msg);

    }
    //c.close();
    //t.join();

*/
/*
    WINDOW * mainwin;
        int ch;
        if ( (mainwin = initscr()) == NULL ) {
            fprintf(stderr, "Error initializing ncurses.\n");
            exit(EXIT_FAILURE);
        }
        noecho();
            keypad(mainwin, TRUE);
            mvaddstr(5, 10, "Press a key ('q' to quit)...");
            mvprintw(7, 10, "You pressed: ");
            refresh();
            while ( (ch = getch()) != 'q' ) {
                   deleteln();
                   mvprintw(7, 10, "You pressed: 0x%x (%s)", ch, intprtkey(ch));
                   refresh();
               }
               delwin(mainwin);
               endwin();
               refresh();
            */
    //initscr();

    //THIS IS THE START OF ASIO's LOOP
        char line[chat_message::max_body_length + 1];
        while (std::cin.getline(line, chat_message::max_body_length + 1))
        {
            string str(line);

          if (str == "/room qa")
          {
            c.changeRoom("qa");
          }
          else
          {
              chat_message msg;

            //c.getRoom()
              //line
              //char[strlen(line)+strlen(c.getRoom())];

              string concat(line + c.getRoom());
              int n = concat.length();
              char char_array[n + 1];
              strcpy(char_array, concat.c_str());



              msg.body_length(strlen(char_array));

              //memcpy roomName the message is going to
              //read commands here
              //string finalMsg = line + c.getRoom();
              //nt size = sizeof(c.getRoom());
              //std::memcpy(msg.body(), c.getRoom(), strlen(c.getRoom()));
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



