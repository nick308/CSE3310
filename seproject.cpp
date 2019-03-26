/*
./chat_server.cpp 9000
./chat_client 127.0.0.1 9000
g++ seproject.cpp -lncurses
./a.out
*/
#include <ncurses.h>
#include <string.h>
#include <iostream>





char msgs[50][100];
char help[50][100];
int chatSize = 14;
char nameArray[80];
char roomName[80];


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

void initMsgs()
{
    if (strcmp(roomName,"Lobby") == 0 || strcmp(roomName,"lobby") == 0)
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
    else
    {
        for (int i = 0; i < 15; i++)
        {
            strcpy(msgs[i],"");
        }
    }
}
void initHelpMenu()
{
    strcpy(help[9],"**HELP MENU**");
    strcpy(help[10],"/join <string> - join an existing chat room");
    strcpy(help[11],"/leave - switch to lobby");
    strcpy(help[12],"/create <string> - creates a new chat room");
    strcpy(help[13],"/private <int> <string> - sends a private message");
    strcpy(help[14],"/rename <string> - changes your account name");

}
void hideMemList(int maxrow, int maxcol)
{
    for (int i = maxrow/3;i<maxrow-3;i++)
    {
        for (int j = maxcol-15;j<maxcol;j++)
        {
            mvprintw(i, j, " ");
        }
    }
}
void showHeader(int row, int col)
{
    for (int i = 0;i<col;i++)
    {
        mvprintw(0, i, " ");
    }


    mvprintw(0, 0, "9:48PM");
    mvprintw(0, 10, roomName);
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
    mvprintw((row/3)+8, col-13, nameArray);
    for (int i = (row/3)+1;i<row-3;i++)
    {
        mvprintw(i, col-15, "|");
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
void clearRow(int row, int maxcol)
{
    for (int i = 0;i<maxcol;i++)
    {
        mvprintw(row, i, " ");
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
void procCmd(char *fullcmd, int row, int col)
{
    int cmdINT = 0;
    char *token = strtok(fullcmd, " ");
    if (strcmp(token,"/rename") == 0)
    {
        strcpy(nameArray,strtok(NULL, " "));

    }
    else if (strcmp(token,"/join") == 0)
    {
        strcpy(roomName,strtok(NULL, " "));
    }
    else if (strcmp(token,"/create") == 0)
    {
        strcpy(roomName,strtok(NULL, " "));
    }



}
int main()
{
    strcpy(roomName, "Lobby");
    char msgArray[80];
    initHelpMenu();
    initMsgs();
    char welcome1[]="Welcome to Super Chat";
    char welcome2[]="Enter user name: ";
    int row,col;
    initscr();
    getmaxyx(stdscr,row,col);
    mvprintw((row/2)-3,(col-strlen(welcome1))/2,"%s",welcome1);
    mvprintw(row/2,(col-strlen(welcome2))/2,"%s",welcome2);



    //std::getline(std::cin, name);
    getstr(nameArray);

    //name = std::str(nameArray);
    // std::string name;
    //std::string name = nameArray;
    clearRow((row/2)-3,col);
    clearRow(row/2, col);

   int loop = 1;
   int position = 2;
   showContent(msgs);
   showHeader(row, col);
   showMemList(row, col);

   while (loop == 1)
   {

     mvprintw(LINES - 2, 0, "%s:                                                                                                ",nameArray);
     mvprintw(LINES - 2, 0, "%s:  ", nameArray);
     getstr(msgArray);
     mvprintw(LINES - 5, 0, "                                                                                                ");


     clearChat(row, col);
     showMemList(row, col);

     if (strcmp(msgArray, "/help") == 0)
     {
         //clearChat(row, col);
         hideMemList(row, col);
         showContent(help);
     }
     else if (strcmp(msgArray, "/cancel") == 0)
     {
         //clearChat(row, col);
         //showMemList(row, col);
         showContent(msgs);
     }
     else if (msgArray[0] == '/')
     {
        procCmd(msgArray, row, col);
        //clearChat(row, col);
        //initMsgs();
        showHeader(row, col);

        hideMemList(row, col);
        showMemList(row, col);
        showContent(msgs);

     }
     else
     {

         //;
         //showMemList(row, col);
         addMsg(nameArray, msgArray);
         //clearChat(row, col);
         showContent(msgs);


     }

   }



   endwin();
   return 0;
}

