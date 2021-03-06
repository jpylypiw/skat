# skat [![GitHub License](https://img.shields.io/github/license/Dichloromethane/skat)](https://github.com/Dichloromethane/skat/blob/master/LICENSE)

[![Repo Size](https://img.shields.io/github/repo-size/Dichloromethane/skat.svg)](https://github.com/Dichloromethane/skat)
[![Release Status](https://img.shields.io/badge/status-alpha-orange)](https://github.com/Dichloromethane/skat)
[![build](https://github.com/Dichloromethane/skat/workflows/build/badge.svg)](https://github.com/Dichloromethane/skat/actions?query=workflow%3Abuild)
[![shellcheck](https://github.com/Dichloromethane/skat/workflows/shellcheck/badge.svg)](https://github.com/Dichloromethane/skat/actions?query=workflow%3Ashellcheck)
[![markdownlint](https://github.com/Dichloromethane/skat/workflows/markdownlint/badge.svg)](https://github.com/Dichloromethane/skat/actions?query=workflow%3Amarkdownlint)

---

**Skat** is a client-server application to play a round of Skat with multiple players.

The application is programmed in **C** and therefore unbeatably **fast**. It is possible to play Skat via the **command line** or via a **graphical client**.

Currently only **Linux** is supported as operating system. It is currently tested on **Debian / Ubuntu**.

---

## Quick Start

For easy installation on Debian the following steps must be executed:

```sh
apt update
apt install -y libglfw3-dev libfreetype-dev build-essential bear
make all
```

After the installation the server can be started as follows:

```sh
./skat_server
```

The command line client can be executed with the following command:

```sh
./skat_client "playername"
```

---

## roadmap

- add `msg` or `say` command to chat with other users
- inform player who is not playing that he is only watching on round begin
- list possible commands in reizen phase
- "YOU hast gepasst at reizwert 18." - fix the language
- "This round is over. The Spielwert was 20" - language!
- context sensitive help: for example in turn phase
- add kibitzen feature - the waiting player can see the cards of the other players
- add command to print seating order

- too much information here:

```sh
Card Kr9 played. Cards currently on table: KrA(71) Kr7(65) Kr9(67)
It is xXxSkatProfi9000xXx's turn.
> --
You lost the Stich. Gid good.
It is xXxSkatProfi9000xXx's turn.
```

```sh
Scores for current round:
        YOU: 0
        iTitus: 0
        xXxSkatProfi9000xXx: 20

> --
Tallied scores after the current round:
        YOU: 3
        iTitus: 0
        xXxSkatProfi9000xXx: 20
        Marcel: 0
```

I should not have points.

- `Your hand:` when hand empty print message that you are watching

---

## requirements

The following requirements must be met to form the project.

On Debian the following packages must be installed:

- libglfw3-dev
- libfreetype-dev
- build-essential
- bear

---

## Setup graphics in WSL

You will need an XServer in Windows. We recommend [VcXsrv](https://sourceforge.net/projects/vcxsrv/).

Start it like this:

![Step 1](doc/vcxsrv_1.png)

![Step 2](doc/vcxsrv_2.png)

![Step 3](doc/vcxsrv_3.png)

Then you need this in your `.bashrc`:

```sh
export DISPLAY=$(awk '/nameserver / {print $2; exit}' /etc/resolv.conf 2>/dev/null):0.0
```

This will set the `DISPLAY` environment variable to `<IP of your windows machine>:0.0`.

Then you can start the graphical client with:

```sh
./skat_client -g <NAME>
```

---

## tools

It is possible to display the dependencies between the classes.
For this purpose the script dep_graph.sh in the tools folder can be used.

Requirements:

- package **graphviz** on Debian

Options:

- `-p [FLAG]` _print_graph_ instead of creating an image, the structure is printed in DOT format
- `-l [FLAG]` _include_libs_ when creating the graph the libraries are displayed in addition to the own classes

Arguments:

- `-s [DIRECTORY_LIST]` _source_dirs_ the specified directory is added to the list of directories to be output. Helpful if you only need the dependencies of a directory
- `-o [FILEPATH]` _output_file_ With this option it is possible to overwrite the output file name.

Example:

```sh
bash tools/dep_graph.sh -p
```

---

## License

skat is BSD 3-Clause License.
