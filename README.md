# Super Mario Bros Clone

A clone of the original Super Mario Bros. 

This was not easy to make, but I made it. Anyways, I'm happy with the final product.

<pre>
Tech used:
- C++       (programming language)
- SFML      (graphics library)
- CMake     (dev tool)
- GCC       (compiler)
- VS Code   (IDE)
- GIMP      (for assets)
- Git       (for versioning)
- GitHub    (hosing repo)

Architectures used:
- ECS

Work Put in:
- 8 weeks of work
- 168 commits

C++ langue features used:
- Smart pointers
- RAII
- Templates
- Classes
- Enums

C++ libraries used:
- stdio     (for io)
- cassert   (for development/testing)
- fstream   (for file reading)
- map       (for ADT)
- memory    (for smart pointers)
- cmath     (for math)
- sstream   (for string building)
- Many more...
</pre>

# Game Play

https://github.com/user-attachments/assets/302967c9-5348-4e5b-9e8f-6409cc57fec6

Game Features
- Collision Detection (Enemy-Player, Player-Tile, etc.)
- Animations
- Real Physics (gravity, skidding, jumping, etc.)
- Enemies (AI)

# Credit

I got a lot of help making this game from the game development lectures posted by Professor Churchill on YouTube and from a guide on the physics of Super Mario Bros posted by Jdaster64. I wouldn't have been able to make this game without the information they have generously made publicly available.

- [The lectures on game development.](https://youtube.com/playlist?list=PL_xRyXins848nDj2v-TJYahzvs-XW9sVV&feature=shared)
- [The guide on the physics of SMB.](https://web.archive.org/web/20130807122227/http://i276.photobucket.com/albums/kk21/jdaster64/smb_playerphysics.png)

# Commands

The following commands are intended for Ubuntu users, and assume that all perquisites are installed. I make no guaranty that the game will build for other OSs. Additionally, some basic understanding of building and running projects is assumed, so some details are left out.

The perquisites:
- SFML
- CMake
- GCC

To build the project and run it:
```
$ make run
```

To just build it:
```
$ make all
```

To remove all binaries:
```
$ make clean
```
