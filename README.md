![Logo](https://github.com/brainboxdotcc/ssod/blob/main/resource/app_logo.png)

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
	}
	,
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
