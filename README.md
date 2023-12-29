 ![Logo](https://github.com/brainboxdotcc/ssod/blob/main/resource/app_logo.png)

The Seven Spells Of Destruction is a multiplayer open world role playing game played through Discord. It is based on the 2.0 version of this game, which was a web based game played through the web.

This game has quite a long history.

### Timeline of the game's development

* 1993: This game was created by me at age 13 as a 1480 paragraph novel, similar in inspiration to "Fighting Fantasy" books by Steve Jackson and Ian Livingstone.
* 1996: The game book was typed up into inter-word on BBC Master 128 Computer
* 2000: The game book content was transcribed into Microsoft Word 98
* 2001: Word 98 content converted to a web based game, completely single player. C++ backend. 
* 2004: Multiplayer features added to the game. Peak player count in 2005 was 1500 concurrent players.
* 2008: Multiplayer game shut down
* 2014: Start of development of a 3D single player Seven Spells game in Unreal Engine 4
* 2021: Unreal Engine 4 game abandoned due to lack of resources to adapt the content to a fully open 3D world.
* 2023: New parser/engine created for the 2004 game content to run via Discord through a bot.



## Compilation

```bash
mkdir build
cd build
cmake ..
make -j
```

## Configuring the bot

Create a config.json in the directory above the build directory:

```json
{
	"token": "token goes here", 
	"log": "log path goes here",
	"database": {
		"host": "localhost",
		"username": "mysql username",
		"password": "mysql password",
		"database": "mysql database",
		"port": 3306
	},
	"encryption": {
		"iv": "16 character AES256 IV",
		"key": "32 character AES256 symmetric key",
	},
	"botlists": {
		"top.gg": {
			"token": "top.gg bot list token"
		},
		"other compatible bot list": {
			"token": "their token..."
		}
	}
}

```

## Software Dependencies

* [D++](https://github.com/brainboxdotcc/dpp) v10.0.28 or later
* libcrypto/libssl
* libmysqlclient 8.x
* g++ 11.4 or later
* cmake
* fmtlib
* spdlog
* screen

## Starting the bot

```bash
cd beholder
screen -dmS ssod ./run.sh
```
