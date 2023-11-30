# Manual

##  Compiling and running the project
    

Open the terminal and navigate to the directory where the Makefile is located. Type the command `make`  to compile the files. 

To start the server in the same path, execute `./Serveur/server_app`. To start the client, execute `./Client/client_app [IP] [username]`. It is forbidden to use a username that is already used by a connected user.

  

##  List of available commands for clients:
    

1.  `:ls` or `:list`
    
Description: Lists all connected clients.

2.  `:lsg` or `:listGames`
    

Description: Lists all games.

3.  `:lssg` or `:listSavedGames`
    

Description: Lists all saved games. If you have a saved game, you can check the game status by entering the game number, or you can enter : exit to return to the default mode.

4.  `:rank`
    
Description: Displays the ranking of all connected clients.

5.  `:v` or `:vie`
    

Description: Invites a user to play an Oware game. Firstly, you need to enter an available username to find your opponent. You need to wait for your opponent to enter yes or no. If your opponent enters ‘yes’ then the game starts. During your turn, the server will send you available cases to play, and you can choose a case to play.

  

In game, you have also commands:

1.  `:s`
    

Description: Surrender this game.

2.  `:p`
    

Description: Open privacy mode.

3.  `:e`
    

Description: Close privacy mode.

4.  `:dm [message]`
    

Description: Send a message to your opponent. Replace [message] with the actual message you want to send.

  

Disconnecting during a game will directly allow your opponent to win.

After the game is over, you can choose whether to save the game. If you enter yes, the game will be saved in your saved game list.

  

6.  `:o` or `:observe`
    

Description: Allows you to spectate a game. You can observe the game by entering the game number. If the game is in private mode, you need to be a friend of one of the players on both sides of the game.

7.  `:bio`
    

Description: Lets you write your personal bio.

8.  `:db [username]`
    

Description: Display the bio of a specific user. Replace [username] with the desired user's username.

9.  `:dm`
    

Description: Sends a message to a specific user. Firstly, you need to enter an available username to find the user, and then you can enter the message.

10.  `:f` or `:friend`
    

Description: Adds a user as a friend. Firstly, you need to enter an available username to find the user, and then you can enter the message. You need to wait for the user to enter yes or no. If the user enters ‘yes’ you are friends.

11.  `:rmf` or `:removeFriend`
    

Description: Removes a user from your friend list. You can enter the name to remove the friend from your friend list.

12.  `:lf` or `:listFriends`
    

Description: Lists all your added friends.

13.  `:exit`, `CTRL-C`, or `CTRL-D`
    

Description: Disconnects from the server. All your personal information will be deleted. Your friends will remove you from their friends list.

  

3.  List of available commands for server:
    

1.  `:ls` or `:list`
    

Description: Lists all connected clients.

2.  `:lsg` or `:listGames`
    

Description: Lists all games.

3.  `:rank`
    

Description: Displays the ranking of all connected clients.
