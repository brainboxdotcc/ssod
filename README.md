![Artwork](https://github.com/brainboxdotcc/ssod/blob/main/resource/app_encyclopaedia.jpg)

The Seven Spells Of Destruction is a multiplayer open world role playing game played through Discord. It is based on the 2.0 version of this game, which was a web based game played through the web.

This game has quite a long history.

### Timeline of the game's development

* __1993__: This game was created by me at age 13 as a 1480 paragraph novel, similar in inspiration to "[Fighting Fantasy](https://en.wikipedia.org/wiki/Fighting_Fantasy)" books by Steve Jackson and Ian Livingstone.
* __1996__: The game book was typed up into [inter-word](https://en.wikipedia.org/wiki/Wordwise#InterWord) on BBC Master 128 Computer
* __2000__: The game book content was transcribed into Microsoft Word 98
* __2001__: Word 98 content converted to a web based game, completely single player. C++ backend. 
* __2003__: Multiplayer features added to the game. [ssod.org](ssod.org) doman name registered. Peak player count in 2005 was 1500 concurrent players.
* __2014__: Start of development of a 3D single player Seven Spells game in Unreal Engine 4
* __2020__: Unreal Engine 4 game abandoned due to lack of resources to adapt the content to a fully open 3D world.
* __2023__: New parser/engine created for the 2004 game content to run via Discord through a bot.

## Navigating the codebase

* [Engine Documentation](docs)
* [Historical 2003 Code](old)
* [Modern Code](src)
* [Lore Encyclopaedia](resource/lore)

## Compilation

```bash
mkdir build
cd build
cmake ..
make -j
```

## Configuring the bot

Create a config.json in the directory above the build directory.

__NOTE__: It is __EXTREMELY IMPORTANT__ to create secure IV/key values for the encryption. This is used to encrypt the state content sent to the user, and if an insecure configuration is placed into the config file here, it may allow selfbots and malicious users to tamper with game state. Keep these values secure and keep them secret as your token, for your the protection of the bot!

```json
{
	"token": "token goes here", 
	"log": "log path goes here",
    "shards": 1,
	"database": {
		"host": "localhost",
		"username": "mysql username",
		"password": "mysql password",
		"database": "mysql database",
		"port": 3306
	},
	"encryption": {
		"iv": "16 character AES256 IV",
		"key": "32 character AES256 symmetric key"
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

* [D++](https://github.com/brainboxdotcc/dpp) v10.0.29 or later
* libcrypto/libssl
* libmysqlclient 8.x
* zlib
* g++ 11.4 or later
* cmake
* fmtlib
* spdlog
* screen

## Starting the bot

```bash
cd ssod
screen -dmS ssod ./run.sh
```
