/*
make -lncurses
./chat_server 9000
./chat_client 127.0.0.1 9000
g++ seproject.cpp -lncurses
g++ chat_client.cpp -lncurses
./a.out
ctl+e
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
  void initHelpMenu()
  {
      char help[50][100];
      strcpy(help[9],"**HELP MENU**");
      strcpy(help[10],"/join <string> - join an existing chat room");
      strcpy(help[11],"/leave - switch to lobby");
      strcpy(help[12],"/create <string> - creates a new chat room");
      strcpy(help[13],"/private <int> <string> - sends a private message");
      strcpy(help[14],"/rename <string> - changes your account name");

  }
  void help()
  {
      clearChat();
  }
  void prompt()
  {
      string concat(getNick());
      int n = concat.length();
      char char_array[n + 1];
      strcpy(char_array, concat.c_str());
      //int len = strlen(char_array);
      //memcpy roomName the message is going to
      //read commands here
      //string finalMsg = line + c.getRoom();
      //nt size = sizeof(c.getRoom());
      //std::memcpy(msg.body(), c.getRoom(), strlen(c.getRoom()));
      //std::memcpy(msg.body(), char_array, len);


      clearRow(maxrow-1);
      mvprintw(maxrow-1,1," %s: ",char_array);
    //mvprintw(maxrow-1,1,"Nick: ");


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
    nickName.assign(nickSender);
      //std::memcpy(nickName, nickSender, nickName.length());
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
  void clearChat()
  {
      int y, x;            // to store where you are
      getyx(stdscr, y, x); // save current pos
      for (int i = 3;i<=maxrow-3;i++)
      {

          move(i, 0);          // move to begining of line
          clrtoeol();          // clear line

      }
      move(y, x);          // move back to where you were
      do_read_header();
  }
private:
  void do_connect(const tcp::resolver::results_type& endpoints)
  {
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

                    string concat(read_msg_.body());
                    int n = concat.length();
                    char char_array[n + 1];
                    strcpy(char_array, concat.c_str());
                  //string str(read_msg_.body());

                      mvprintw(row,1,"%s",char_array);
/*
                      for (int i = n+1; i <= maxcol; i++)
                      {
                          mvprintw(row,i,"-");

                      }

*/

                    row = row + 1;
                    prompt();
















                      /*
                    int num = str.find(";");
                    int len = str.length();n);
                    string roomName = str.substr(0, num);
                    string message = str.substr(num, len);

                    mvprintw(row,1,"%s",roomName);
                    if (getRoom() == roomName)
                    {
                        clearRow(row);
                        mvprintw(row,1,"%s",read_msg_.body());
                        for (int i = read_msg_.body_length()+1; i <= maxcol; i++)
                        {
                            mvprintw(row,i," ");
                            row = row + 1;
                        }

                    }
                    */




                  //move(maxrow, 1);
                  //maxcol = maxcol + 1;


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
  string currentRoom;
  string nickName;
  int maxrow,maxcol;
  int row,col;
  chat_message read_msg_;
  chat_message_queue write_msgs_;
};



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
std::string getstring()
{
    std::string input;

    // let the terminal do the line editing
    //nocbreak();
    //echo();

    // this reads from buffer after <ENTER>, not "raw"
    // so any backspacing etc. has already been taken care of
    int ch = getch();

    while ( ch != '\n' )
    {
        input.push_back( ch );
        ch = getch();
    }

    // restore your cbreak / echo settings here

    return input;
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
    c.close();
    t.join();
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

        //do you recommend using WINDOW or not.
        //some functions take a window argument

        //how to control position of ncurses cursor

        //what triggers the server updating the client with new messages
        //class initialize calls do_connect() and that calls do_read_header()

        //getstr, getchar, getch
        //man page not helpful
        //raw, nobreak, echo, nodelay, nocbreak, cbreak
        // printw(), wprintw(), mvprintw() and mvwprintw()



    char welcome1[]="Welcome to Super Chat";
    char welcome2[]="Enter user name: ";
    int row,col;
    int loop = 1;
    //int position = 2;
    //WINDOW *my_win;
    initscr();
    //stdscr = curses.initscr();
    //stdscr.clear();
    cbreak();
    raw();
    keypad(stdscr, TRUE);
    echo();
    //nodelay(stdscr, TRUE);
    getmaxyx(stdscr,row,col);
//mvaddstr((row/2)-3,(col-strlen(welcome1))/2,welcome1);
    mvprintw((row/2)-3,(col-strlen(welcome1))/2,"%s",welcome1);
    mvprintw(row/2,(col-strlen(welcome2))/2,"%s",welcome2);
    getnstr(nameArray,10);
    string nameStr(nameArray);
    /*

    showHeaderFooter(row, col);
    clearRow((row/2)-3,col);
    clearRow(row/2, col);
    showContent(msgs);
    showMemList(row, col);

*/





    auto endpoints = resolver.resolve(argv[1], argv[2]);
    chat_client c(io_context, endpoints);
    //c.changeRoom("Lobby");
    //c.getScreen();
    c.changeNick(nameStr);
    c.clearRow((row/2)-3);
    c.clearRow(row/2);
    showHeaderFooter(row, col);
    std::thread t([&io_context](){ io_context.run(); });




        c.prompt();
        while (loop == 1)
        {
            char str[80];
            //char* str;
            //string line;

            //c.clearRow(row-1);
            //string name(c.getNick());
            //mvprintw(row - 1, 1, "%s:", name);


            c.prompt();

            //string line = getstring();a
            getstr(str);
            string line(str);
            //mvprintw(row - 5, 1, "%s", str);





          if (line.find("/room ") == 0)
          {
            string roomName = line.substr(0, line.find(" "));
            c.changeRoom(roomName);
          }
          else if (line.find("/help ") == 0)
          {
            c.help();
          }
          else
          {
              int len = sizeof(str);
              string lenStr = to_string(len);
              chat_message msg;
              string concat(c.getRoom() + ";" + lenStr + ";" + c.getNick() + ": " + str);
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



