# Six Degrees of Kevin Bacon

Craving a little Oscar trivia? Try your hand in an Internet parlor game about Kevin Bacon’s
acting career. He’s never been nominated for an Oscar, but he’s certainly achieved
immortality—based on the premise that he is the hub of the entertainment universe. Mike
Ginelli, Craig Fass and Brian Turtle invented the game while students at Albright College in
1993, and their Bacon bit spread rapidly after convincing then TV talk-show host Jon Stewart
to demonstrate the game to all those who tuned in. From these humble beginnings, a Web
site was built, a book was published and a nationwide cult-fad was born.

When you think about Hollywood heavyweights, you don’t immediately think of Kevin
Bacon. But his career spans almost 20 years through films such as Flatliners, The Air Up There,
Footloose, The River Wild, JFK and Animal House. So brush up on your Bacon lore. To play an
Internet version, visit [http://oracleofbacon.org/](http://oracleofbacon.org/).

## Overview

There are two major components to this project:
Here is the implementation for an imdb class, which allows user to
quickly look up all of the films an actor or actress has appeared in and all of the
people starring in any given film. The imdb class is layered over two STL
maps — one mapping people to movies and another mapping movies to
people.

Also in project there is implemented a breadth-first search algorithm that consults my
super-clever imdb class to find the shortest path connecting any two
actor/actresses. If the search goes on for so long that you can tell it’ll be of length
7 or more, then you can be reasonably confident (and pretend that you know for
sure that) there’s no path connecting them. The program uses the STL and you can see a 
legitimate scenario where a complex program benefits from two types
paradigms: high-level C++ and lowlevel
C (with its exposed memory and its procedural orientation.)
